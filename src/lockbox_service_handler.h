#pragma once

#include "LockboxService.h"
#include "counter.h"
#include "db_manager_server.h"

namespace lockbox {

class LockboxServiceHandler : virtual public LockboxServiceIf {
 public:
  // Does not take ownershi of |manager|.
  LockboxServiceHandler(DBManagerServer* manager);

  UserID RegisterUser(const UserAuth& user);

  DeviceID RegisterDevice(const UserAuth& user);

  TopDirID RegisterTopDir(const UserAuth& user);

  bool AcquireLockRelPath(const PathLock& lock);
  void ReleaseLockRelPath(const PathLock& lock);

  int64_t UploadPackage(const LocalPackage& pkg);

  void DownloadPackage(LocalPackage& _return, const DownloadRequest& req);

  void PollForUpdates(Updates& _return,
                      const UserAuth& auth,
                      const DeviceID device);

  void Send(const UserAuth& sender,
            const std::string& receiver_email,
            const VersionInfo& vinfo);

  void GetLatestVersion(VersionInfo& _return,
                        const UserAuth& requestor,
                        const std::string& receiver_email);
 private:
  DBManagerServer* manager_;
};

} // namespace lockbox
