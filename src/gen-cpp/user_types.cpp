/**
 * Autogenerated by Thrift Compiler (1.0.0-dev)
 *
 * DO NOT EDIT UNLESS YOU ARE SURE THAT YOU KNOW WHAT YOU ARE DOING
 *  @generated
 */
#include "user_types.h"

#include <algorithm>



const char* UserProfile::ascii_fingerprint = "3368C2F81F2FEF71F11EDACDB2A3ECEF";
const uint8_t UserProfile::binary_fingerprint[16] = {0x33,0x68,0xC2,0xF8,0x1F,0x2F,0xEF,0x71,0xF1,0x1E,0xDA,0xCD,0xB2,0xA3,0xEC,0xEF};

uint32_t UserProfile::read(::apache::thrift::protocol::TProtocol* iprot) {

  uint32_t xfer = 0;
  std::string fname;
  ::apache::thrift::protocol::TType ftype;
  int16_t fid;

  xfer += iprot->readStructBegin(fname);

  using ::apache::thrift::protocol::TProtocolException;


  while (true)
  {
    xfer += iprot->readFieldBegin(fname, ftype, fid);
    if (ftype == ::apache::thrift::protocol::T_STOP) {
      break;
    }
    switch (fid)
    {
      case 1:
        if (ftype == ::apache::thrift::protocol::T_I32) {
          xfer += iprot->readI32(this->uid);
          this->__isset.uid = true;
        } else {
          xfer += iprot->skip(ftype);
        }
        break;
      case 2:
        if (ftype == ::apache::thrift::protocol::T_STRING) {
          xfer += iprot->readString(this->name);
          this->__isset.name = true;
        } else {
          xfer += iprot->skip(ftype);
        }
        break;
      case 3:
        if (ftype == ::apache::thrift::protocol::T_STRING) {
          xfer += iprot->readString(this->blurb);
          this->__isset.blurb = true;
        } else {
          xfer += iprot->skip(ftype);
        }
        break;
      default:
        xfer += iprot->skip(ftype);
        break;
    }
    xfer += iprot->readFieldEnd();
  }

  xfer += iprot->readStructEnd();

  return xfer;
}

uint32_t UserProfile::write(::apache::thrift::protocol::TProtocol* oprot) const {
  uint32_t xfer = 0;
  xfer += oprot->writeStructBegin("UserProfile");

  xfer += oprot->writeFieldBegin("uid", ::apache::thrift::protocol::T_I32, 1);
  xfer += oprot->writeI32(this->uid);
  xfer += oprot->writeFieldEnd();

  xfer += oprot->writeFieldBegin("name", ::apache::thrift::protocol::T_STRING, 2);
  xfer += oprot->writeString(this->name);
  xfer += oprot->writeFieldEnd();

  xfer += oprot->writeFieldBegin("blurb", ::apache::thrift::protocol::T_STRING, 3);
  xfer += oprot->writeString(this->blurb);
  xfer += oprot->writeFieldEnd();

  xfer += oprot->writeFieldStop();
  xfer += oprot->writeStructEnd();
  return xfer;
}

void swap(UserProfile &a, UserProfile &b) {
  using ::std::swap;
  swap(a.uid, b.uid);
  swap(a.name, b.name);
  swap(a.blurb, b.blurb);
  swap(a.__isset, b.__isset);
}

