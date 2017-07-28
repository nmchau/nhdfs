
#include "filewriter.h"
#include "filesystem.h"
#include "workers.h"
#include "macros.h"

#include <iostream>

namespace nhdfs
{

Napi::FunctionReference FileWriter::constructor;

void FileWriter::Init(Napi::Env env, Napi::Object exports)
{
    Napi::HandleScope scope(env);
    Napi::Function t =
        DefineClass(
            env,
            "FileWriter",
            {InstanceMethod("Open", &FileWriter::Open),
             InstanceMethod("Write", &FileWriter::Write),
             InstanceMethod("Flush", &FileWriter::Flush),
             InstanceMethod("HFlush", &FileWriter::HFlush),
             InstanceMethod("Sync", &FileWriter::Sync),
             InstanceMethod("Close", &FileWriter::Close)});
    constructor = Napi::Persistent(t); //TODO : ??
    constructor.SuppressDestruct();
    (exports).Set(Napi::String::New(env, "FileWriter"), t);
}

FileWriter::FileWriter(const Napi::CallbackInfo &info) : Napi::ObjectWrap<FileWriter>()
{
    Napi::Env env = info.Env();
    REQUIRE_ARGUMENTS(2);
    REQUIRE_ARGUMENT_STRING(0, path)
    REQUIRE_ARGUMENT_FS(1, f)
    this->path = path;
    this->fs = f->fs;
}

FileWriter::~FileWriter()
{
    //std::cout << "destroy writer " << this->path << std::endl;
    this->close();
    //int res = this->close();
    // if (res < 0) //TODO:
    // {
    //     Napi::Error::New(env, hdfsGetLastError()).ThrowAsJavaScriptException();
    // }
}

Napi::Value FileWriter::Open(const Napi::CallbackInfo &info) 
{
    Napi::Env env = info.Env();
    REQUIRE_ARGUMENTS(1)
    REQUIRE_ARGUMENT_FUNCTION(0, cb)
    std::function<int()> f = [this] {
        file = hdfsOpenFile(fs, path.c_str(), O_WRONLY, 0 /*not used*/, 0, 0);
        return (file) ? 0 : -1;
    };
    SimpleResWorker::Start(f, cb);
    return info.Env().Null();
}

Napi::Value FileWriter::Write(const Napi::CallbackInfo &info)
{
    Napi::Env env = info.Env();
    REQUIRE_ARGUMENTS(3)
    REQUIRE_ARGUMENT_BUFFER(0, buffer)
    REQUIRE_ARGUMENT_INT(1, l)
    REQUIRE_ARGUMENT_FUNCTION(2, cb)
    char *b = buffer.Data();
    std::function<int()> f = [this, b, l] {
        int res = hdfsWrite(fs, file, b, l);
        return res;
    };
    SimpleResWorker::Start(f, cb);
    return info.Env().Null();
}

Napi::Value FileWriter::Flush(const Napi::CallbackInfo &info)
{
    Napi::Env env = info.Env();
    REQUIRE_ARGUMENTS(1);
    REQUIRE_ARGUMENT_FUNCTION(0, cb)
    std::function<int()> f = [this] {
        return hdfsFlush(fs, file);
        ;
    };
    SimpleResWorker::Start(f, cb);
    return info.Env().Null();
}

Napi::Value FileWriter::HFlush(const Napi::CallbackInfo &info)
{
    Napi::Env env = info.Env();
    REQUIRE_ARGUMENTS(1);
    REQUIRE_ARGUMENT_FUNCTION(0, cb)
    std::function<int()> f = [this] {
        return hdfsHFlush(fs, file);
        ;
    };
    SimpleResWorker::Start(f, cb);
    return info.Env().Null();
}

Napi::Value FileWriter::Sync(const Napi::CallbackInfo &info)
{
    Napi::Env env = info.Env();
    REQUIRE_ARGUMENTS(1);
    REQUIRE_ARGUMENT_FUNCTION(0, cb)
    std::function<int()> f = [this] {
        return hdfsSync(fs, file);
        ;
    };
    SimpleResWorker::Start(f, cb);
    return info.Env().Null();
}

Napi::Value FileWriter::Close(const Napi::CallbackInfo &info)
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

int FileWriter::close()
{
    if (this->file)
    {
        return hdfsCloseFile(this->fs, this->file);
    }
    return 0;
}
}