
#include "filereader.h"
#include "filesystem.h"
#include "workers.h"
#include "macros.h"

#include <iostream>

namespace nhdfs
{

Napi::FunctionReference FileReader::constructor;

void FileReader::Init(Napi::Env env, Napi::Object exports)
{
    Napi::HandleScope scope(env);
    Napi::Function t =
        DefineClass(
            env,
            "FileReader",
            {
                InstanceMethod("Open", &FileReader::Open),
                InstanceMethod("Read", &FileReader::Read),
                InstanceMethod("Close", &FileReader::Close)
            }
        );
    constructor = Napi::Persistent(t); //TODO : ??
    constructor.SuppressDestruct();
    (exports).Set(Napi::String::New(env, "FileReader"), t);
}

FileReader::FileReader(const Napi::CallbackInfo &info) : Napi::ObjectWrap<FileReader>() 
{
    Napi::Env env = info.Env();
    REQUIRE_ARGUMENTS(2);
    REQUIRE_ARGUMENT_STRING(0, path)
    REQUIRE_ARGUMENT_FS(1, f)
    this->path = path;
    this->fs = f->fs;
}

FileReader::~FileReader()
{
    //std::cout << "destroy reader " << this->path << std::endl;
    this->close();
    //int res = this->close();
    // if (res < 0) //TODO: 
    // {
    //     Napi::Error::New(env, hdfsGetLastError()).ThrowAsJavaScriptException();
    // }
}


Napi::Value FileReader::Open(const Napi::CallbackInfo &info)
{
    Napi::Env env = info.Env();
    REQUIRE_ARGUMENTS(1)
    REQUIRE_ARGUMENT_FUNCTION(0, cb)
    std::function<int()> f = [this] {
        file = hdfsOpenFile(fs, path.c_str(), O_RDONLY, 0 /*not used*/, 0, 0);
        return (file) ? 0 : -1;
    };
    SimpleResWorker::Start(f, cb);
    return info.Env().Null();
}

Napi::Value FileReader::Read(const Napi::CallbackInfo &info)
{
    std::cout << "read " << std::endl;
    Napi::Env env = info.Env();
    REQUIRE_ARGUMENTS(3);
    REQUIRE_ARGUMENT_BUFFER(0, buffer)
    REQUIRE_ARGUMENT_INT(1, l)
    REQUIRE_ARGUMENT_FUNCTION(2, cb)
    char * b = buffer.Data();
    std::function<int()> f = [this, b, l] {
        int r = hdfsRead(fs, file, b, l);
        return r;
    };
    SimpleResWorker::Start(f, cb);
    return info.Env().Null();
}

Napi::Value FileReader::Close(const Napi::CallbackInfo &info)
{
    Napi::Env env = info.Env();
    REQUIRE_ARGUMENTS(1);
    REQUIRE_ARGUMENT_FUNCTION(0, cb)
    std::function<int()> f = [this] {
        return close();
    };
    SimpleResWorker::Start(f, cb);
    return info.Env().Null();
}

int FileReader::close()
{
    if ( this->file ) {
        return hdfsCloseFile(this->fs, this->file);
    }
    return 0;
}

} //namespace