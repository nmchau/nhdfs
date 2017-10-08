#include "nhdfs.h"

using namespace nhdfs;

namespace
{

Napi::Object RegisterModule(Napi::Env env, Napi::Object exports)
{
  Napi::HandleScope scope(env);

  ClusterInfo::Init(env, exports);
  FileSystem::Init(env, exports);
  FileReader::Init(env, exports);
  FileWriter::Init(env, exports);
  return exports;
}

}

NODE_API_MODULE(nhdfs, RegisterModule)