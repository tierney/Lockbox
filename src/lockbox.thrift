namespace cpp lockbox

# Basic authentication.
struct UserAuth {
  1: required string email,
  2: required string password,
}

typedef i64 DeviceID
typedef i64 UserID
typedef i64 TopDirID

# Packages uploaded for storage and sharing on Lockbox.
enum PackageType {
  SNAPSHOT,
  DELTA,
}

struct LocalPackage {
  1: required string base_abs_path,
  2: required string base_rel_path,
  3: optional PackageType type,
  4: optional string contents,
}

struct HybridCrypto {
  1: required string data,
  2: optional map<string, string> user_enc_session,
}

struct RemotePackage {
  1: required string base_abs_path,
  2: required HybridCrypto path,
  3: required HybridCrypto data,
}

struct DownloadRequest {
  1: required UserAuth auth,
  2: required string top_dir,
  3: required string pkg_name,
}

struct PathLock {
  1: required UserAuth user,
  2: required i64 top_dir_id,
  3: required i32 seconds_requested,
  4: optional string rel_path;
}

# When a user requests an update list for files to fetch, this is the list that
# the user can use to fetch requests for new downloads.
struct Updates {
  1: required string device,
  2: required string user,
  3: optional list<HybridCrypto> updates,
}

# Hash Chain Digests. Follows the methodology outlined by Jinyuan Li.
#
struct HashChainDigestHistory {
  1: required list<string> history,
}

struct HashChainDigestEntry {
  1: required i64 view,
  2: required i64 seq_no,
  3: required string digest,
  4: required string signature,
}

# TODO(tierney): These should be tied to the TDN and relative paths too...
struct VersionInfo {
  1: optional map<i32, HashChainDigestEntry> vector,
  2: optional HashChainDigestHistory history
}

# Database names.
enum LockboxDatabase {
  UNKNOWN = 0
  EMAIL_USER,
  USER_DEVICE,
  DEVICE_SYNC,
  EMAIL_KEY,
  USER_TOP_DIR,

  TOP_DIR_PLACEHOLDER,

  TOP_DIR_META,
  TOP_DIR_RELPATH_LOCK,
  TOP_DIR_SNAPSHOTS,
  TOP_DIR_DATA,
  TOP_DIR_FPTRS,
}

# Unifying Service
#
service LockboxService {
  # Returns the UID.
  UserID RegisterUser(1:UserAuth user),

  # Returns the Device ID.
  DeviceID RegisterDevice(1:UserAuth user),

  # Returns the top directory's GUID.
  TopDirID RegisterTopDir(1:UserAuth user),

  # Grab lock on rel path for the TDN.
  bool AcquireLockRelPath(1:PathLock lock),

  void ReleaseLockRelPath(1:PathLock lock),

  i64 UploadPackage(1:LocalPackage pkg),

  LocalPackage DownloadPackage(1:DownloadRequest req),

  Updates PollForUpdates(1:UserAuth auth, 2:DeviceID device),

  # Hash chain service API.
  void Send(1:UserAuth sender, 2:string receiver_email, 3:VersionInfo vinfo),

  VersionInfo GetLatestVersion(1:UserAuth requestor, 2:string receiver_email),
}
