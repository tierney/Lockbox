#include "file_event_queue_handler.h"

#include <vector>

#include "base/md5.h"
#include "base/sha1.h"
#include "base/strings/string_number_conversions.h"
#include "base/memory/scoped_ptr.h"
#include "base/file_util.h"
#include "base/files/file_path.h"
#include "base/strings/string_split.h"
#include "file_watcher/file_watcher.h"
#include "util.h"
#include "encryptor.h"
#include "thrift_util.h"
#include "simple_delta.h"

using std::vector;

namespace lockbox {

namespace {

// TODO(tierney): If memory becomes an issue, here is one place that can be
// improved. Incrementally read file and run contents through MD5 digest
// updater.
string FileToMD5(const string& path) {
  string contents;
  CHECK(file_util::ReadFileToString(base::FilePath(path), &contents));
  return base::MD5String(contents);
}

} // namespace

FileEventQueueHandler::FileEventQueueHandler(const string& top_dir_id,
                                             DBManagerClient* dbm,
                                             Client* client,
                                             Encryptor* encryptor,
                                             UserAuth* user_auth)
    : dbm_(dbm),
      client_(client),
      encryptor_(encryptor),
      user_auth_(user_auth),
      thread_(new boost::thread(
          boost::bind(&FileEventQueueHandler::Run, this))),
      top_dir_id_(top_dir_id) {
  CHECK(dbm);
  CHECK(client);
  CHECK(encryptor);
  CHECK(user_auth);
  LOG(INFO) << "Starting FileEventQueueHandler for " << top_dir_id;

  // Enumerate the files and check the latest times. We can hold off on hashes
  // until we have more information.
  DBManagerClient::Options options;
  options.type = ClientDB::TOP_DIR_LOCATION;
  CHECK(dbm_->Get(options, top_dir_id_, &top_dir_path_));
  PrepareMaps();
}

FileEventQueueHandler::~FileEventQueueHandler() {
}

// Enumerate the files and check the latest times. We can hold off on hashes
// until we have more information.
void FileEventQueueHandler::PrepareMaps() {
  file_util::FileEnumerator enumerator(
      base::FilePath(top_dir_path_),
      true /* recursive */,
      file_util::FileEnumerator::FILES |
      file_util::FileEnumerator::DIRECTORIES);

  while (true) {
    base::FilePath fp(enumerator.Next());
    if (fp.value().empty()) {
      break;
    }

    file_util::FileEnumerator::FindInfo info;
    enumerator.GetFindInfo(&info);
    time_t mtime =
        file_util::FileEnumerator::GetLastModifiedTime(info).ToTimeT();
    path_mtime_[fp.value()] = mtime;

    // TODO(tierney): This could be an epically slow operation.
    // Open up all files and hash them, mapping the path to the hash.
    path_hashes_[fp.value()] = FileToMD5(fp.value());
  }
}

namespace {

void ParseTimestampPath(const string& ts_path_key, string* timestamp, string* path) {
  CHECK(timestamp);
  CHECK(path);

  vector<string> ts_path;
  // TODO(tierney): See the file_watcher_thread for updates to this code's
  // handling of the key formatting.
  LOG(INFO) << "ts_path_key: " << ts_path_key;
  base::SplitString(ts_path_key, ':', &ts_path);
  if (ts_path.size() != 2) {
    CHECK(false) << "bad ts_path_key " << ts_path_key;
  }

  timestamp->assign(ts_path[0]);
  path->assign(ts_path[1]);
}

} // namespace

void FileEventQueueHandler::Run() {
  // Start up phase. Should get information from the RELPATHS to know which
  // files have been tracked already. Need to run through file to see if
  // anything has changed since last examined.

  while (true) {
    // Should grab entries from both the server and the local client changes. If
    // there are changes from the cloud, we should prioritize those.

    string key, value;
    // Prioritize the remote actions first.
    if (dbm_->First(
            DBManager::Options(ClientDB::UPDATE_QUEUE_SERVER, top_dir_id_),
            &key, &value)) {
      HandleRemoteAction(key, value);
      LOG(INFO) << "Done with remote update.";
      dbm_->Delete(
          DBManager::Options(ClientDB::UPDATE_QUEUE_SERVER, top_dir_id_),
          key);
      LOG(INFO) << "Deleted entry on cloud after update";
      continue;
    }

    if (dbm_->First(
            DBManager::Options(ClientDB::UPDATE_QUEUE_CLIENT, top_dir_id_),
            &key, &value)) {
      HandleLocalAction(key, value);
      LOG(INFO) << "Done with local update";
      dbm_->Delete(
          DBManager::Options(ClientDB::UPDATE_QUEUE_CLIENT, top_dir_id_),
          key);
    }
  }
}

namespace {

void SplitKey(const string& key, string* timestamp, string* top_dir,
              string* guid, string* hash) {
  CHECK(timestamp);
  CHECK(top_dir);
  CHECK(guid);
  CHECK(hash);
  timestamp->clear();
  top_dir->clear();
  guid->clear();
  hash->clear();

  vector<string> key_entries;
  base::SplitString(key, '_', &key_entries);
  CHECK(key_entries.size() == 4);
  timestamp->assign(key_entries[0]);
  top_dir->assign(key_entries[1]);
  guid->assign(key_entries[2]);
  hash->assign(key_entries[3]);
}

} // namespace
void FileEventQueueHandler::HandleRemoteAction(const string& key,
                                               const string& value) {
  LOG(INFO) << "HandleRemoteAction() called.";
  // Value stored in the |key|.
  // 1367949789_1_25baec40-5a1f-108b-4ca31a19-3ab710a6_yUCJ/6CWfX2Uin3kzy6cZC7L9Wc=,
  // ts        tdn   guid                                  hash
  string timestamp, top_dir, guid, hash;
  SplitKey(key, &timestamp, &top_dir, &guid, &hash);

  string top_dir_path;
  dbm_->Get(DBManager::Options(ClientDB::TOP_DIR_LOCATION, ""),
            top_dir, &top_dir_path);

  const string rel_path(dbm_->RelpathGuidToPath(guid, top_dir));

  // Local lock.
  if (!dbm_->AcquireLockPath(guid, top_dir)) {
    CHECK(false) << "Local change beat us.";
  }

  DownloadRequest request;
  request.auth.email = user_auth_->email;
  request.auth.password = user_auth_->password;
  request.top_dir = top_dir;
  request.pkg_name = hash;

  // Grab the package from the cloud.
  LOG(INFO) << "Getting remote package";
  RemotePackage package;
  client_->Exec<void, RemotePackage&, const DownloadRequest&>(
      &LockboxServiceClient::DownloadPackage, package, request);

  // Learn the action type from the type of the package (DELTA | SNAPSHOT).
  LOG(INFO) << "Package type " << _PackageType_VALUES_TO_NAMES.at(package.type);
  LOG(INFO) << "   " << hash;

  // Case: Delta.
  if (package.type == PackageType::DELTA) {
    // Check that the prev is what we have. If we have the latest, then quit.

    // Decrypt the delta.

    // Apply the delta.

    // Write the updated file.
  }

  // Case: Snapshot.
  if (package.type == PackageType::SNAPSHOT) {
    string current_file;
    string path = top_dir_path + rel_path;
    LOG(INFO) << "Path " << path;
    file_util::ReadFileToString(base::FilePath(path), &current_file);

    // See if the current file and the decrypted file are the same.
    string payload;
    encryptor_->Decrypt(package.payload.data,
                        package.payload.user_enc_session,
                        &payload);
    LOG(INFO) << "Downloaded " << payload;
    LOG(INFO) << "Current Fh " << current_file;
  }

  // Release local lock.
  dbm_->ReleaseLockPath(guid, top_dir);
}

void FileEventQueueHandler::HandleLocalAction(const string& ts_path,
                                              const string& event_type) {
  // For a local action.
  string ts, path;
  ParseTimestampPath(ts_path, &ts, &path);

  // What to do in each of the |event_types| (added, deleted, modified)?
  // We'll start by handling the added case.
  int fw_action = 0;
  base::StringToInt(event_type, &fw_action);
  LOG(INFO) << "fw_action: " << fw_action;
  bool success = false;
  switch (fw_action) {
    case FW::Actions::Add:
      success = HandleAddAction(path);
      if (!success) {
        LOG(WARNING) << "Someone else add won... ";
        CHECK(false);
      }
      // If not successful, consider rewriting the queue entry with the
      // current timestamp so that we can make sure to handle any updates to
      // the file, even if they come from the cloud.
            break;
    case FW::Actions::Delete:
      LOG(WARNING) << "Delete not implemented.";
      break;
    case FW::Actions::Modified:
      success = HandleModAction(path);
      CHECK(success) << "Someone else mod won...";
      break;
    default:
      CHECK(false) << "Unrecognize event " << fw_action;
  }
}

bool FileEventQueueHandler::HandleModAction(const string& path) {
  // TODO(tierney): Check that the hash of the file before and after are the
  // same (unmodified). Otherwise we need to start over.

  // Need to lock the path.
  DBManagerClient::Options options;
  options.type = ClientDB::LOCATION_RELPATH_ID;
  options.name = top_dir_id_;
  string path_guid;
  dbm_->Get(options, RemoveBaseFromInput(top_dir_path_, path), &path_guid);

  // Lock the file in the cloud.
  PathLockRequest path_lock;
  path_lock.user.email = user_auth_->email;
  path_lock.user.password = user_auth_->password;
  path_lock.top_dir = top_dir_id_;
  path_lock.rel_path = path_guid;

  // Acquire the lock on the cloud.
  LOG(INFO) << "Locking on the cloud.";
  PathLockResponse response;
  client_->Exec<void, PathLockResponse&, const PathLockRequest&>(
      &LockboxServiceClient::AcquireLockRelPath,
      response,
      path_lock);
  if (!response.acquired) {
    LOG(INFO) << "Someone else already locked the file " << path;
    CHECK(false) << "This should not happen (using GUIDs in the cloud).";
    // TODO(tierney): Consider rewriting the queue entry with the current
    // timestamp so that we can make sure to handle any updates to the
    // file, even if they come from the cloud.
    return false;
  }

  const string relative_path(RemoveBaseFromInput(top_dir_path_, path));
  string serial_pkg;

  /*
  // Pull out the original file.
  options.type = ClientDB::DATA;
  options.name = top_dir_id_;

  // Need to find the hash of the previous and check if we have that.
  string prev_hash;
  dbm_->Get(DBManager::Options(ClientDB::RELPATHS_HEAD_HASH, top_dir_id_),
            relative_path, &prev_hash);

  string serial_pkg;
  dbm_->Get(options, prev_hash, &serial_pkg);

  RemotePackage prev_version_pkg;
  ThriftFromString(serial_pkg, &prev_version_pkg);

  // Unwrap the data.
  string output;
  encryptor_->Decrypt(prev_version_pkg.payload.data,
                      prev_version_pkg.payload.user_enc_session,
                      &output);
  */
  string output;
  dbm_->Get(DBManager::Options(ClientDB::RELPATHS_HEAD_FILE, top_dir_id_),
            relative_path, &output);
  string prev_hash;
  dbm_->Get(DBManager::Options(ClientDB::RELPATHS_HEAD_FILE_HASH, top_dir_id_),
            relative_path, &prev_hash);
  LOG(INFO) << "Prev Hash " << prev_hash;

  // Read the file from the disk.
  string current;
  file_util::ReadFileToString(base::FilePath(path), &current);

  string current_sha1(base::SHA1HashString(current));
  string current_sha1_hex(base::HexEncode(current_sha1.c_str(),
                                          current_sha1.length()));
  LOG(INFO) << "Curr Hash " << current_sha1_hex;
  if (current_sha1_hex == prev_hash) {
    LOG(INFO) << "File actually unchanged according to SHA1";

    // Release the lock.
    // TODO(tierney): Locking path remotely should be a scoped operation.
    LOG(INFO) << "Releasing lock";
    client_->Exec<void, const PathLockRequest&>(
        &LockboxServiceClient::ReleaseLockRelPath,
        path_lock);
    return true;
  }

  // Compute the difference.
  string delta(Delta::Generate(output, current));

  // encrypt, bundle the package, upload as a DELTA
  LOG(INFO) << "Encrypting.";
  RemotePackage package;
  package.top_dir = top_dir_id_;
  package.rel_path_id = path_guid;
  package.type = PackageType::DELTA;
  encryptor_->EncryptString(
      top_dir_path_, path, delta, response.users, &package);



  // Upload the package. Cloud needs to update the appropriate user's
  // update queues.
  LOG(INFO) << "Uploading.";
  int64 ret = client_->Exec<int64, const RemotePackage&>(
      &LockboxServiceClient::UploadPackage, package);
  LOG(INFO) << "  Uploaded " << ret;

  // Store the data in the local client db. SHOULD ACTUALLY STORE THE WHOLE
  // VERSION. THIS WAY WE CAN AVOID RECONSTRUCTION COSTS.
  options.type = ClientDB::DATA;
  options.name = top_dir_id_;
  const string sha1(base::SHA1HashString(package.payload.data));
  const string hash(base::HexEncode(sha1.c_str(), sha1.size()));

  serial_pkg.clear();
  ThriftToString(package, &serial_pkg);
  dbm_->Put(options, hash, serial_pkg);

  // Put into relpath the latest hash.
  dbm_->Put(DBManager::Options(ClientDB::RELPATHS_HEAD_HASH, top_dir_id_),
            relative_path, hash);

  // Then set the fptr.
  dbm_->Put(DBManager::Options(ClientDB::FPTRS, top_dir_id_), hash, prev_hash);


  // Release the lock.
  LOG(INFO) << "Releasing lock";
  client_->Exec<void, const PathLockRequest&>(
      &LockboxServiceClient::ReleaseLockRelPath,
      path_lock);

  return true;
}

// bool FileEventQueueHandler::LockPath() {
//   return false;
// }

bool FileEventQueueHandler::HandleAddAction(const string& path) {
  // Register path.
  string path_guid;
  RegisterRelativePathRequest rel_path_req;
  rel_path_req.user.email = user_auth_->email;
  rel_path_req.user.password = user_auth_->password;
  rel_path_req.top_dir = top_dir_id_;
  client_->Exec<void, string&, const RegisterRelativePathRequest&>(
      &LockboxServiceClient::RegisterRelativePath,
      path_guid,
      rel_path_req);

  LOG(INFO) << "ADDed file GUID from server " << path_guid;

  const string relative_path(RemoveBaseFromInput(top_dir_path_, path));

  // Store the association between the GUID and the location.
  DBManagerClient::Options options;
  options.type = ClientDB::RELPATH_ID_LOCATION;
  options.name = top_dir_id_;
  dbm_->Put(options, path_guid, relative_path);
  options.type = ClientDB::LOCATION_RELPATH_ID;
  dbm_->Put(options, relative_path, path_guid);

  // Lock the file in the cloud.
  PathLockRequest path_lock;
  path_lock.user.email = user_auth_->email;
  path_lock.user.password = user_auth_->password;
  path_lock.top_dir = top_dir_id_;
  path_lock.rel_path = path_guid;

  // Acquire the lock on the cloud.
  LOG(INFO) << "Locking on the cloud.";
  PathLockResponse response;
  client_->Exec<void, PathLockResponse&, const PathLockRequest&>(
      &LockboxServiceClient::AcquireLockRelPath,
      response,
      path_lock);
  if (!response.acquired) {
    LOG(INFO) << "Someone else already locked the file " << path;
    CHECK(false) << "This should not happen (using GUIDs in the cloud).";
    // TODO(tierney): Consider rewriting the queue entry with the current
    // timestamp so that we can make sure to handle any updates to the
    // file, even if they come from the cloud.
    return false;
  }

  string current;
  file_util::ReadFileToString(base::FilePath(path), &current);

  // Read the file, do the encryption.
  LOG(INFO) << "Encrypting.";
  RemotePackage package;
  package.top_dir = top_dir_id_;
  package.rel_path_id = path_guid;
  package.type = PackageType::SNAPSHOT;
  encryptor_->EncryptString(
      top_dir_path_, path, current, response.users, &package);

  // string out_path;
  // encryptor_->Decrypt(package.payload.data,
  //                     package.payload.user_enc_session,
  //                     &out_path);
  // LOG(INFO) << "Is it what we expect? " << out_path;

  // Upload the package. Cloud needs to update the appropriate user's
  // update queues.
  LOG(INFO) << "Uploading.";
  int64 ret = client_->Exec<int64, const RemotePackage&>(
      &LockboxServiceClient::UploadPackage,
      package);
  LOG(INFO) << "  Uploaded " << ret;

  // Store the data in the local client db.
  options.type = ClientDB::DATA;
  options.name = top_dir_id_;
  const string sha1(base::SHA1HashString(package.payload.data));
  const string hash(base::HexEncode(sha1.c_str(), sha1.size()));

  // Place the contents of the previous file.
  const string current_file_sha1(base::SHA1HashString(current));
  const string current_file_sha1_hex(base::HexEncode(current_file_sha1.c_str(),
                                                     current_file_sha1.size()));
  dbm_->Put(DBManager::Options(ClientDB::RELPATHS_HEAD_FILE, top_dir_id_),
            relative_path, current);
  dbm_->Put(DBManager::Options(ClientDB::RELPATHS_HEAD_FILE_HASH, top_dir_id_),
            relative_path, current_file_sha1_hex);

  string serial_pkg;
  ThriftToString(package, &serial_pkg);
  dbm_->Put(options, hash, serial_pkg);

  // Put into relpath the latest hash.
  dbm_->Put(DBManager::Options(ClientDB::RELPATHS_HEAD_HASH, top_dir_id_),
            relative_path, hash);

  // Then set the fptr.
  dbm_->Put(DBManager::Options(ClientDB::FPTRS, top_dir_id_), hash, "");

  // Release the lock.
  LOG(INFO) << "Releasing lock";
  client_->Exec<void, const PathLockRequest&>(
      &LockboxServiceClient::ReleaseLockRelPath,
      path_lock);


  return true;
}

} // namespace lockbox
