#include "nhdfs.h"

using namespace nhdfs;

namespace
{

void RegisterModule(Napi::Env env, Napi::Object exports, Napi::Object module)
{
  Napi::HandleScope scope(env);

  FileSystem::Init(env, exports);
  FileReader::Init(env, exports);
}

}

NODE_API_MODULE(nhdfs, RegisterModule)