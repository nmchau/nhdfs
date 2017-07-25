#ifndef NH_ASYNC_H
#define NH_ASYNC_H

#include <napi.h>
#include "filesystem.h"

namespace nhdfs {

class FileSystem;

/* abstract */ 
class AsyncWorker : public Napi::AsyncWorker {
public:
  AsyncWorker (
      FileSystem* fs,
      napi_env env,
      napi_value callback
  ) : Napi::AsyncWorker(Napi::Function(env, callback)), fileSystem(fs) { }

protected:
//   void SetStatus(leveldb::Status status) {
//     this->status = status;
//     if (!status.ok())
//       SetError(status.ToString());
//   }
  FileSystem* fileSystem;
private:
  //leveldb::Status status;
};

} // namespace leveldown

#endif //NH_ASYNC_H