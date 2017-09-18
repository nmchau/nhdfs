#ifndef NHDFS_FILESYSTEM_H_
#define NHDFS_FILESYSTEM_H_

#include <map>
#include <vector>
#include <string>
#include <napi.h>

#include "nhdfs.h"

#include <hdfs/hdfs.h>
#include <memory>

namespace nhdfs
{

#define DEFAULT "default"
#define USER "user"
#define TICKETPATH "kerbTicketCachePath"
#define TOKEN "authToken"

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
  // Napi::Value Copy(const Napi::CallbackInfo &info);
  Napi::Value GetWorkingDirectory(const Napi::CallbackInfo &info);
  Napi::Value SetWorkingDirectory(const Napi::CallbackInfo &info);

  Napi::Value CreateDirectory(const Napi::CallbackInfo &info);
  Napi::Value Delete(const Napi::CallbackInfo &info);

  Napi::Value SetReplication(const Napi::CallbackInfo &info);

  Napi::Value List(const Napi::CallbackInfo &info);
  Napi::Value GetPathInfo(const Napi::CallbackInfo &info);

  /**
  * GetDefaultBlockSize - Get the default blocksize.
  * @return The default blocksize or error.
  */
  Napi::Value GetDefaultBlockSize(const Napi::CallbackInfo &info);

  /**
  * hdfsGetCapacity - Return the raw capacity of the filesystem.
  */
  Napi::Value GetCapacity(const Napi::CallbackInfo &info);

  /**
  * hdfsGetUsed - Return the total raw size of all files in the filesystem.
  */
  Napi::Value GetUsed(const Napi::CallbackInfo &info);

  /**
 * Change the user and/or group of a file or directory.
 * @param path          the path to the file or directory
 * @param owner         User string.  Set to NULL for 'no change'
 * @param group         Group string.  Set to NULL for 'no change'
 * @return              0 on success else -1
 */
  Napi::Value Chown(const Napi::CallbackInfo &info);

  /**
 * hdfsChmod
 * @param path the path to the file or directory
 * @param mode the bitmask to set it to
 * @return 0 on success else -1
 */
  Napi::Value Chmod(const Napi::CallbackInfo &info);

  /**
 * hdfsUtime
 * @param path the path to the file or directory
 * @param mtime new modification time or -1 for no change
 * @param atime new access time or -1 for no change
 * @return 0 on success else -1
 */
 Napi::Value Utime(const Napi::CallbackInfo &info);

 /**
 * Truncate - Truncate the file in the indicated path to the indicated size.
 * @param fs The configured filesystem handle.
 * @param path the path to the file.
 * @param pos the position the file will be truncated to.
 * @return true if and client does not need to wait for block recovery,
 * false if client needs to wait for block recovery.
 */
 Napi::Value Truncate(const Napi::CallbackInfo &info);


  FileSystem(const Napi::CallbackInfo &info);
  ~FileSystem();

  std::string NameNode() { return this->nameNode; }
  tPort Port() { return this->port; }

  friend class FileReader;
  friend class FileWriter;

private:
  std::string nameNode;
  tPort port;
  Napi::Object params;
  hdfsFS fs = nullptr;
};

} //nhdfs

#endif //NHDFS_FILESYSTEM_H_