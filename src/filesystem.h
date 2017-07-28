#ifndef NHDFS_FILESYSTEM_H_
#define NHDFS_FILESYSTEM_H_

#include <map>
#include <vector>
#include <string>
#include <napi.h>

#include "nhdfs.h"
#include "async.h"

#include <hdfs/hdfs.h>
#include <memory>

namespace nhdfs
{

class FileSystem;

class FileSystem : public Napi::ObjectWrap<FileSystem>
{
public:
  static Napi::FunctionReference constructor;
  static void Init(Napi::Env env, Napi::Object exports);

  static inline bool HasInstance(Napi::Value val)
  {
    Napi::Env env = val.Env();
    Napi::HandleScope scope(env);
    if (!val.IsObject())
      return false;
    Napi::Object obj = val.As<Napi::Object>();
    return obj.InstanceOf(constructor.Value());
  }

  Napi::Value Exists(const Napi::CallbackInfo &info);

  Napi::Value Rename(const Napi::CallbackInfo &info);
  Napi::Value GetWorkingDirectory(const Napi::CallbackInfo &info);
  Napi::Value SetWorkingDirectory(const Napi::CallbackInfo &info);

  Napi::Value CreateDirectory(const Napi::CallbackInfo &info);
  Napi::Value Delete(const Napi::CallbackInfo &info);

  Napi::Value SetReplication(const Napi::CallbackInfo &info);

  Napi::Value List(const Napi::CallbackInfo &info);
  Napi::Value GetPathInfo(const Napi::CallbackInfo &info);

  FileSystem(const Napi::CallbackInfo &info);
  ~FileSystem();

  std::string NameNode() { return this->nameNode; }
  tPort Port() { return this->port; }

  friend class FileReader;
  friend class FileWriter;

private:
  std::string nameNode;
  tPort port;
  hdfsFS fs = nullptr;
};

} //nhdfs

#endif //NHDFS_FILESYSTEM_H_