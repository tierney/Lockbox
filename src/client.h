// Templated client that does what we want.
//
// lockbox::UserID user_id =
//     client.Exec<lockbox::UserID, const lockbox::UserAuth&>(
//         &lockbox::LockboxServiceClient::RegisterUser,
//         auth);
// lockbox::DeviceID device_id =
//     client.Exec<lockbox::DeviceID, const lockbox::UserAuth&>(
//         &lockbox::LockboxServiceClient::RegisterDevice,
//         auth);
// lockbox::TopDirID top_dir_id =
//     client.Exec<lockbox::TopDirID, const lockbox::UserAuth&>(
//         &lockbox::LockboxServiceClient::RegisterTopDir,
//         auth);
#pragma once

#include "LockboxService.h"
#include <thrift/transport/TSocket.h>
#include <thrift/transport/TBufferTransports.h>
#include <thrift/protocol/TBinaryProtocol.h>

namespace lockbox {

class Client {
 public:
  struct ConnInfo {
    ConnInfo(std::string host, int port) : host(host), port(port) {}

    std::string host;
    int port;
  };

  Client(const ConnInfo& conn_info)
    : socket_(new apache::thrift::transport::TSocket(conn_info.host,
                                                       conn_info.port)),
      transport_(new apache::thrift::transport::TFramedTransport(socket_)),
      protocol_(new apache::thrift::protocol::TBinaryProtocol(transport_)),
      client_(protocol_),
      host_(conn_info.host),
      port_(conn_info.port) {
  }

  virtual ~Client() {}

  // Driver for the LockboxServiceClient code.
  template <typename R, typename... Args>
  R Exec(R(LockboxServiceClient::*func)(Args...), Args... args) {
    return execer<R(Args...)>::exec(transport_, client_, func,
                                    std::forward<Args>(args)...);
  }


 private:
  // Specialized driver for the Thrift API. In particular, the template wrappers
  // are necessary to account for some void return types given that the Thrift
  // code offers functions with void and non-void return types.
  template <typename F>
  struct execer {};

  template<typename R, typename... Args>
  struct execer<R(Args...)> {
    static R exec(
        boost::shared_ptr<apache::thrift::transport::TTransport> transport_,
        LockboxServiceClient client_,
        R(LockboxServiceClient::*func)(Args...), Args... args) {
      transport_->open();
      R ret = (client_.*func)(std::forward<Args>(args)...);
      transport_->close();
      return ret;
    }
  };

  template<typename... Args>
  struct execer<void(Args...)> {
    static void exec(
        boost::shared_ptr<apache::thrift::transport::TTransport> transport_,
        LockboxServiceClient client_,
        void(LockboxServiceClient::*func)(Args...), Args... args) {
      transport_->open();
      (client_.*func)(std::forward<Args>(args)...);
      transport_->close();
    }
  };

  boost::shared_ptr<apache::thrift::transport::TSocket> socket_;
  boost::shared_ptr<apache::thrift::transport::TTransport> transport_;
  boost::shared_ptr<apache::thrift::protocol::TProtocol> protocol_;

  LockboxServiceClient client_;

  std::string host_;
  int port_;
};

} // namespace lockbox