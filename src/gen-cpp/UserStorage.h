/**
 * Autogenerated by Thrift Compiler (1.0.0-dev)
 *
 * DO NOT EDIT UNLESS YOU ARE SURE THAT YOU KNOW WHAT YOU ARE DOING
 *  @generated
 */
#ifndef UserStorage_H
#define UserStorage_H

#include <thrift/TDispatchProcessor.h>
#include "user_types.h"



class UserStorageIf {
 public:
  virtual ~UserStorageIf() {}
  virtual void store(const UserProfile& user) = 0;
  virtual void retrieve(UserProfile& _return, const int32_t uid) = 0;
};

class UserStorageIfFactory {
 public:
  typedef UserStorageIf Handler;

  virtual ~UserStorageIfFactory() {}

  virtual UserStorageIf* getHandler(const ::apache::thrift::TConnectionInfo& connInfo) = 0;
  virtual void releaseHandler(UserStorageIf* /* handler */) = 0;
};

class UserStorageIfSingletonFactory : virtual public UserStorageIfFactory {
 public:
  UserStorageIfSingletonFactory(const boost::shared_ptr<UserStorageIf>& iface) : iface_(iface) {}
  virtual ~UserStorageIfSingletonFactory() {}

  virtual UserStorageIf* getHandler(const ::apache::thrift::TConnectionInfo&) {
    return iface_.get();
  }
  virtual void releaseHandler(UserStorageIf* /* handler */) {}

 protected:
  boost::shared_ptr<UserStorageIf> iface_;
};

class UserStorageNull : virtual public UserStorageIf {
 public:
  virtual ~UserStorageNull() {}
  void store(const UserProfile& /* user */) {
    return;
  }
  void retrieve(UserProfile& /* _return */, const int32_t /* uid */) {
    return;
  }
};

typedef struct _UserStorage_store_args__isset {
  _UserStorage_store_args__isset() : user(false) {}
  bool user;
} _UserStorage_store_args__isset;

class UserStorage_store_args {
 public:

  UserStorage_store_args() {
  }

  virtual ~UserStorage_store_args() throw() {}

  UserProfile user;

  _UserStorage_store_args__isset __isset;

  void __set_user(const UserProfile& val) {
    user = val;
  }

  bool operator == (const UserStorage_store_args & rhs) const
  {
    if (!(user == rhs.user))
      return false;
    return true;
  }
  bool operator != (const UserStorage_store_args &rhs) const {
    return !(*this == rhs);
  }

  bool operator < (const UserStorage_store_args & ) const;

  uint32_t read(::apache::thrift::protocol::TProtocol* iprot);
  uint32_t write(::apache::thrift::protocol::TProtocol* oprot) const;

};


class UserStorage_store_pargs {
 public:


  virtual ~UserStorage_store_pargs() throw() {}

  const UserProfile* user;

  uint32_t write(::apache::thrift::protocol::TProtocol* oprot) const;

};


class UserStorage_store_result {
 public:

  UserStorage_store_result() {
  }

  virtual ~UserStorage_store_result() throw() {}


  bool operator == (const UserStorage_store_result & /* rhs */) const
  {
    return true;
  }
  bool operator != (const UserStorage_store_result &rhs) const {
    return !(*this == rhs);
  }

  bool operator < (const UserStorage_store_result & ) const;

  uint32_t read(::apache::thrift::protocol::TProtocol* iprot);
  uint32_t write(::apache::thrift::protocol::TProtocol* oprot) const;

};


class UserStorage_store_presult {
 public:


  virtual ~UserStorage_store_presult() throw() {}


  uint32_t read(::apache::thrift::protocol::TProtocol* iprot);

};

typedef struct _UserStorage_retrieve_args__isset {
  _UserStorage_retrieve_args__isset() : uid(false) {}
  bool uid;
} _UserStorage_retrieve_args__isset;

class UserStorage_retrieve_args {
 public:

  UserStorage_retrieve_args() : uid(0) {
  }

  virtual ~UserStorage_retrieve_args() throw() {}

  int32_t uid;

  _UserStorage_retrieve_args__isset __isset;

  void __set_uid(const int32_t val) {
    uid = val;
  }

  bool operator == (const UserStorage_retrieve_args & rhs) const
  {
    if (!(uid == rhs.uid))
      return false;
    return true;
  }
  bool operator != (const UserStorage_retrieve_args &rhs) const {
    return !(*this == rhs);
  }

  bool operator < (const UserStorage_retrieve_args & ) const;

  uint32_t read(::apache::thrift::protocol::TProtocol* iprot);
  uint32_t write(::apache::thrift::protocol::TProtocol* oprot) const;

};


class UserStorage_retrieve_pargs {
 public:


  virtual ~UserStorage_retrieve_pargs() throw() {}

  const int32_t* uid;

  uint32_t write(::apache::thrift::protocol::TProtocol* oprot) const;

};

typedef struct _UserStorage_retrieve_result__isset {
  _UserStorage_retrieve_result__isset() : success(false) {}
  bool success;
} _UserStorage_retrieve_result__isset;

class UserStorage_retrieve_result {
 public:

  UserStorage_retrieve_result() {
  }

  virtual ~UserStorage_retrieve_result() throw() {}

  UserProfile success;

  _UserStorage_retrieve_result__isset __isset;

  void __set_success(const UserProfile& val) {
    success = val;
  }

  bool operator == (const UserStorage_retrieve_result & rhs) const
  {
    if (!(success == rhs.success))
      return false;
    return true;
  }
  bool operator != (const UserStorage_retrieve_result &rhs) const {
    return !(*this == rhs);
  }

  bool operator < (const UserStorage_retrieve_result & ) const;

  uint32_t read(::apache::thrift::protocol::TProtocol* iprot);
  uint32_t write(::apache::thrift::protocol::TProtocol* oprot) const;

};

typedef struct _UserStorage_retrieve_presult__isset {
  _UserStorage_retrieve_presult__isset() : success(false) {}
  bool success;
} _UserStorage_retrieve_presult__isset;

class UserStorage_retrieve_presult {
 public:


  virtual ~UserStorage_retrieve_presult() throw() {}

  UserProfile* success;

  _UserStorage_retrieve_presult__isset __isset;

  uint32_t read(::apache::thrift::protocol::TProtocol* iprot);

};

class UserStorageClient : virtual public UserStorageIf {
 public:
  UserStorageClient(boost::shared_ptr< ::apache::thrift::protocol::TProtocol> prot) :
    piprot_(prot),
    poprot_(prot) {
    iprot_ = prot.get();
    oprot_ = prot.get();
  }
  UserStorageClient(boost::shared_ptr< ::apache::thrift::protocol::TProtocol> iprot, boost::shared_ptr< ::apache::thrift::protocol::TProtocol> oprot) :
    piprot_(iprot),
    poprot_(oprot) {
    iprot_ = iprot.get();
    oprot_ = oprot.get();
  }
  boost::shared_ptr< ::apache::thrift::protocol::TProtocol> getInputProtocol() {
    return piprot_;
  }
  boost::shared_ptr< ::apache::thrift::protocol::TProtocol> getOutputProtocol() {
    return poprot_;
  }
  void store(const UserProfile& user);
  void send_store(const UserProfile& user);
  void recv_store();
  void retrieve(UserProfile& _return, const int32_t uid);
  void send_retrieve(const int32_t uid);
  void recv_retrieve(UserProfile& _return);
 protected:
  boost::shared_ptr< ::apache::thrift::protocol::TProtocol> piprot_;
  boost::shared_ptr< ::apache::thrift::protocol::TProtocol> poprot_;
  ::apache::thrift::protocol::TProtocol* iprot_;
  ::apache::thrift::protocol::TProtocol* oprot_;
};

class UserStorageProcessor : public ::apache::thrift::TDispatchProcessor {
 protected:
  boost::shared_ptr<UserStorageIf> iface_;
  virtual bool dispatchCall(::apache::thrift::protocol::TProtocol* iprot, ::apache::thrift::protocol::TProtocol* oprot, const std::string& fname, int32_t seqid, void* callContext);
 private:
  typedef  void (UserStorageProcessor::*ProcessFunction)(int32_t, ::apache::thrift::protocol::TProtocol*, ::apache::thrift::protocol::TProtocol*, void*);
  typedef std::map<std::string, ProcessFunction> ProcessMap;
  ProcessMap processMap_;
  void process_store(int32_t seqid, ::apache::thrift::protocol::TProtocol* iprot, ::apache::thrift::protocol::TProtocol* oprot, void* callContext);
  void process_retrieve(int32_t seqid, ::apache::thrift::protocol::TProtocol* iprot, ::apache::thrift::protocol::TProtocol* oprot, void* callContext);
 public:
  UserStorageProcessor(boost::shared_ptr<UserStorageIf> iface) :
    iface_(iface) {
    processMap_["store"] = &UserStorageProcessor::process_store;
    processMap_["retrieve"] = &UserStorageProcessor::process_retrieve;
  }

  virtual ~UserStorageProcessor() {}
};

class UserStorageProcessorFactory : public ::apache::thrift::TProcessorFactory {
 public:
  UserStorageProcessorFactory(const ::boost::shared_ptr< UserStorageIfFactory >& handlerFactory) :
      handlerFactory_(handlerFactory) {}

  ::boost::shared_ptr< ::apache::thrift::TProcessor > getProcessor(const ::apache::thrift::TConnectionInfo& connInfo);

 protected:
  ::boost::shared_ptr< UserStorageIfFactory > handlerFactory_;
};

class UserStorageMultiface : virtual public UserStorageIf {
 public:
  UserStorageMultiface(std::vector<boost::shared_ptr<UserStorageIf> >& ifaces) : ifaces_(ifaces) {
  }
  virtual ~UserStorageMultiface() {}
 protected:
  std::vector<boost::shared_ptr<UserStorageIf> > ifaces_;
  UserStorageMultiface() {}
  void add(boost::shared_ptr<UserStorageIf> iface) {
    ifaces_.push_back(iface);
  }
 public:
  void store(const UserProfile& user) {
    size_t sz = ifaces_.size();
    size_t i = 0;
    for (; i < (sz - 1); ++i) {
      ifaces_[i]->store(user);
    }
    ifaces_[i]->store(user);
  }

  void retrieve(UserProfile& _return, const int32_t uid) {
    size_t sz = ifaces_.size();
    size_t i = 0;
    for (; i < (sz - 1); ++i) {
      ifaces_[i]->retrieve(_return, uid);
    }
    ifaces_[i]->retrieve(_return, uid);
    return;
  }

};



#endif