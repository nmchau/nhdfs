
#include "filereader.h"
#include "filesystem.h"
#include "macros.h"

#include <iostream>

namespace nhdfs
{

class ReadWorker : public Napi::AsyncWorker
{
  public:
    ReadWorker(std::function<int()> f, Napi::Function cb) : AsyncWorker(cb), func(f), msg("") {}
    void Execute() override
    {
        this->res = func();
        if (this->res < 0)
        {
            this->msg = hdfsGetLastError();
            std::cout << "err: " << this->msg << std::endl;
        }
    }
    void OnOK() override
    {
        auto err = (this->res < 0) ? Napi::String::New(this->Env(), msg) : this->Env().Null();
        auto v = Napi::Number::New(this->Env(), this->res);
        this->Callback().MakeCallback(this->Receiver().Value(), std::initializer_list<napi_value>{err, v});
    }

  private:
    std::function<int()> func;
    int res = 0;
    std::string msg;
};

Napi::FunctionReference FileReader::constructor;

void FileReader::Init(Napi::Env env, Napi::Object exports)
{
    Napi::HandleScope scope(env);
    Napi::Function t =
        DefineClass(
            env,
            "FileReader",
            {
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
    std::cout << "construct " << std::endl;
    Napi::Env env = info.Env();
    REQUIRE_ARGUMENTS(2);
    REQUIRE_ARGUMENT_STRING(0, path)
    this->path = path;
    std::cout << "construct " << this->path << std::endl;
    if (!FileSystem::HasInstance(info[1]))
    {
        Napi::TypeError::New(env, "FileSystem object expected").ThrowAsJavaScriptException();
    }
    FileSystem * f = Napi::ObjectWrap<FileSystem>::Unwrap(info[1].As<Napi::Object>());
    this->fs = f->fs;
    this->file = hdfsOpenFile(fs, this->path.c_str(), O_RDONLY, 0 /*not used*/, 0, 0);
    if (!this->file)
    {
        std::cout << "cerr: " << hdfsGetLastError() << std::endl;
        Napi::Error::New(env, hdfsGetLastError()).ThrowAsJavaScriptException();
    }
}

FileReader::~FileReader()
{
    std::cout << "destroy reader " << this->path << std::endl;
    this->close();
    //int res = this->close();
    // if (res < 0) //TODO: 
    // {
    //     Napi::Error::New(env, hdfsGetLastError()).ThrowAsJavaScriptException();
    // }
}

Napi::Value FileReader::Read(const Napi::CallbackInfo &info)
{
    std::cout << "read " << std::endl;
    Napi::Env env = info.Env();
    REQUIRE_ARGUMENTS(3);
    Napi::Buffer<char> buffer = info[0].As<Napi::Buffer<char>>();
    char * b = buffer.Data();
    //b[0] = '1';
    //b[2] = '1';
    //std::cout << "read buflen" << buffer.Length() << std::endl;
    tSize l = info[1].As<Napi::Number>();
    Napi::Function cb = info[2].As<Napi::Function>();
    //std::cout << "read before async " << std::endl;
    std::function<int()> f = [this, b, l] {
        //std::cout << "read start async " << l <<std::endl;
        int r = hdfsRead(fs, file, b, l);//buffer.Data(), l);
        //std::cout << "read" << r <<std::endl;
        //std::cout << b << l <<std::endl;
        return r;
    };
    ReadWorker *worker = new ReadWorker(f, cb);
    worker->Queue();
    return info.Env().Null();
}

Napi::Value FileReader::Close(const Napi::CallbackInfo &info)
{
    std::cout << "c close " << std::endl;
    Napi::Env env = info.Env();
    REQUIRE_ARGUMENTS(1);
    Napi::Function cb = info[0].As<Napi::Function>();
    std::function<int()> f = [this] {
        return close();
    };
    ReadWorker *worker = new ReadWorker(f, cb);
    worker->Queue();
    return info.Env().Null();
}

int FileReader::close()
{
    if ( this->file ) {
        std::cout << "f close " << std::endl;
        return hdfsCloseFile(this->fs, this->file);
    }
    return 0;
}

} //namespace