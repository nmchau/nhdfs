#include <napi.h>
#include <utility>
#include <initializer_list>

#include <chrono>
#include <thread>
#include <functional>
#include <iostream>

#include "filesystem.h"
#include "macros.h"
#include "workers.h"

namespace nhdfs
{

hdfsFS connect(std::string node, tPort port, Napi::Env env)
{
    struct hdfsBuilder *builder = hdfsNewBuilder();
    hdfsBuilderSetNameNode(builder, node.c_str());
    hdfsBuilderSetNameNodePort(builder, port);
    hdfsFS fs = hdfsBuilderConnect(builder);
    hdfsFreeBuilder(builder);
    if (!fs)
    {
        throw Napi::Error::New(env, hdfsGetLastError());
    }
    return fs;
}

Napi::FunctionReference FileSystem::constructor;

void FileSystem::Init(Napi::Env env, Napi::Object exports)
{
    Napi::HandleScope scope(env);
    Napi::Function t =
        DefineClass(
            env,
            "FileSystem",
            {InstanceMethod("Exists", &FileSystem::Exists),
             InstanceMethod("Rename", &FileSystem::Rename),
             InstanceMethod("GetWorkingDirectory", &FileSystem::GetWorkingDirectory),
             InstanceMethod("SetWorkingDirectory", &FileSystem::SetWorkingDirectory),
             InstanceMethod("CreateDirectory", &FileSystem::CreateDirectory),
             InstanceMethod("Delete", &FileSystem::Delete),
             InstanceMethod("List", &FileSystem::List),
             InstanceMethod("GetPathInfo", &FileSystem::GetPathInfo)});
    constructor = Napi::Persistent(t);
    constructor.SuppressDestruct();
    (exports).Set(Napi::String::New(env, "FileSystem"), t);
}

FileSystem::FileSystem(const Napi::CallbackInfo &info) : Napi::ObjectWrap<FileSystem>()
{
    REQUIRE_ARGUMENTS(2);
    REQUIRE_ARGUMENT_STRING(0, nn);
    REQUIRE_ARGUMENT_UINT(1, p);
    this->nameNode = nn;
    this->port = p;
    this->fs = connect(nn, p, info.Env());
}

FileSystem::~FileSystem()
{
    hdfsDisconnect(this->fs);
}

Napi::Value FileSystem::Exists(const Napi::CallbackInfo &info)
{
    REQUIRE_ARGUMENTS(2)
    REQUIRE_ARGUMENT_STRING(0, path)
    REQUIRE_ARGUMENT_FUNCTION(1, cb)
    std::function<int()> f = [this, path] {
        return hdfsExists(fs, path.c_str());
    };
    SimpleResWorker::Start(f, cb);
    return info.Env().Null();
}

Napi::Value FileSystem::Rename(const Napi::CallbackInfo &info)
{
    REQUIRE_ARGUMENTS(3)
    REQUIRE_ARGUMENT_STRING(0, oldPath)
    REQUIRE_ARGUMENT_STRING(1, newPath)
    REQUIRE_ARGUMENT_FUNCTION(2, cb)
    std::function<int()> f = [this, oldPath, newPath] {
        return hdfsRename(fs, oldPath.c_str(), newPath.c_str());
    };
    SimpleResWorker::Start(f, cb);
    return info.Env().Null();
}

Napi::Value FileSystem::GetWorkingDirectory(const Napi::CallbackInfo &info)
{

    REQUIRE_ARGUMENTS(3);
    REQUIRE_ARGUMENT_BUFFER(0, buffer)
    REQUIRE_ARGUMENT_INT(1, l)
    REQUIRE_ARGUMENT_FUNCTION(2, cb)
    char *b = buffer.Data();
    std::function<int()> f = [this, b, l] {
        char * res = hdfsGetWorkingDirectory(fs, b, l);
        return ( ! res ) ? -1 : 0;
    };
    SimpleResWorker::Start(f, cb);
    return info.Env().Null();
}

Napi::Value FileSystem::SetWorkingDirectory(const Napi::CallbackInfo &info)
{
    REQUIRE_ARGUMENTS(2)
    REQUIRE_ARGUMENT_STRING(0, path)
    REQUIRE_ARGUMENT_FUNCTION(1, cb)
    std::function<int()> f = [this, &path] {
        return hdfsSetWorkingDirectory(fs, path.c_str());
    };
    SimpleResWorker::Start(f, cb);
    return info.Env().Null();
}

Napi::Value FileSystem::CreateDirectory(const Napi::CallbackInfo &info)
{
    REQUIRE_ARGUMENTS(2);
    REQUIRE_ARGUMENT_STRING(0, path)
    REQUIRE_ARGUMENT_FUNCTION(1, cb)

    std::function<int()> f = [this, &path] {
        return hdfsCreateDirectory(fs, path.c_str());
    };
    SimpleResWorker::Start(f, cb);
    return info.Env().Null();
}

Napi::Value FileSystem::Delete(const Napi::CallbackInfo &info)
{
    REQUIRE_ARGUMENTS(3)
    REQUIRE_ARGUMENT_STRING(0, path)
    REQUIRE_ARGUMENT_INT(1, recursive)
    REQUIRE_ARGUMENT_FUNCTION(2, cb)
    std::function<int()> f = [this, &path, recursive] {
        return hdfsDelete(fs, path.c_str(), recursive);
    };
    SimpleResWorker::Start(f, cb);
    return info.Env().Null();
}

Napi::Value FileSystem::SetReplication(const Napi::CallbackInfo &info)
{
    REQUIRE_ARGUMENTS(3)
    REQUIRE_ARGUMENT_STRING(0, path)
    REQUIRE_ARGUMENT_INT(1, repNum)
    REQUIRE_ARGUMENT_FUNCTION(2, cb)
    std::function<int()> f = [this, &path, repNum] {
        return hdfsSetReplication(fs, path.c_str(), repNum);
    };
    SimpleResWorker::Start(f, cb);
    return info.Env().Null();
}

Napi::Object FileInfoToObject(hdfsFileInfo *fileStat, Napi::Env env)
{
    Napi::Object res = Napi::Object::New(env);
    char kind = (char)fileStat->mKind;

    res.Set(NAPISTRING(env, "type"), NAPISTRING(env, (kind == 'F' ? "file" : kind == 'D' ? "directory" : "other")));
    res.Set(NAPISTRING(env, "path"), NAPISTRING(env, fileStat->mName));
    res.Set(NAPISTRING(env, "size"), Napi::Number::New(env, fileStat->mSize));
    res.Set(NAPISTRING(env, "replication"), Napi::Number::New(env, fileStat->mReplication));
    res.Set(NAPISTRING(env, "block_size"), Napi::Number::New(env, fileStat->mBlockSize));
    res.Set(NAPISTRING(env, "owner"), NAPISTRING(env, fileStat->mOwner));
    res.Set(NAPISTRING(env, "group"), NAPISTRING(env, fileStat->mGroup));
    res.Set(NAPISTRING(env, "permissions"), Napi::Number::New(env, fileStat->mPermissions));
    res.Set(NAPISTRING(env, "last_mod"), Napi::Number::New(env, fileStat->mLastMod));
    res.Set(NAPISTRING(env, "last_access"), Napi::Number::New(env, fileStat->mLastAccess));
    return res;
}

class ListWorker : public Napi::AsyncWorker
{
  public:
    ListWorker(const std::string &p, Napi::Function cb, hdfsFS fs) : AsyncWorker(cb), fs(fs), path(p), msg("") {}
    virtual ~ListWorker()
    {
        if (this->info)
            hdfsFreeFileInfo(this->info, this->num);
    }
    void Execute() override
    {
        this->info = hdfsListDirectory(this->fs, this->path.c_str(), &this->num);
        if (!this->info)
        {
            this->msg = hdfsGetLastError();
        }
    }
    void OnOK() override
    {
        auto nv = this->Env().Null(); //either error or result list
        if (this->info)
        {
            Napi::Array result(Napi::Array::New(this->Env(), num));
            for (int i = 0; i < this->num; i++)
            {
                Napi::Object o = FileInfoToObject(&this->info[i], this->Env());
                (result).Set(i, o);
            }
            this->Callback().MakeCallback(this->Receiver().Value(), std::initializer_list<napi_value>{nv, result});
        }
        else
        {
            auto err = Napi::String::New(this->Env(), this->msg);
            this->Callback().MakeCallback(this->Receiver().Value(), std::initializer_list<napi_value>{err, nv});
        }
    }

  private:
    hdfsFS fs;
    hdfsFileInfo *info = nullptr;
    int num = 0;
    const std::string path;
    std::string msg;
};

Napi::Value FileSystem::List(const Napi::CallbackInfo &info)
{
    REQUIRE_ARGUMENTS(2);
    std::string path = info[0].As<Napi::String>();
    Napi::Function cb = info[1].As<Napi::Function>();
    ListWorker *worker = new ListWorker(path, cb, this->fs);
    worker->Queue();
    return info.Env().Null();
}

class PathInfoWorker : public Napi::AsyncWorker
{
  public:
    PathInfoWorker(const std::string &p, Napi::Function cb, hdfsFS fs) : AsyncWorker(cb), fs(fs), path(p), msg("") {}
    virtual ~PathInfoWorker()
    {
        if (this->info)
            hdfsFreeFileInfo(this->info, 1);
    }
    void Execute() override
    {
        this->info = hdfsGetPathInfo(this->fs, this->path.c_str());
        if (!this->info)
        {
            this->msg = hdfsGetLastError();
        }
    }
    void OnOK() override
    {
        auto nv = this->Env().Null(); //either error or result list
        if (this->info)
        {
            Napi::Object result = FileInfoToObject(this->info, this->Env());
            this->Callback().MakeCallback(this->Receiver().Value(), std::initializer_list<napi_value>{nv, result});
        }
        else
        {
            auto err = Napi::String::New(this->Env(), this->msg);
            this->Callback().MakeCallback(this->Receiver().Value(), std::initializer_list<napi_value>{err, nv});
        }
    }

  private:
    hdfsFS fs;
    hdfsFileInfo *info = nullptr;
    const std::string path;
    std::string msg;
};

Napi::Value FileSystem::GetPathInfo(const Napi::CallbackInfo &info)
{
    REQUIRE_ARGUMENTS(2);
    std::string path = info[0].As<Napi::String>();
    Napi::Function cb = info[1].As<Napi::Function>();
    PathInfoWorker *worker = new PathInfoWorker(path, cb, this->fs);
    worker->Queue();
    return info.Env().Null();
}

} // namespace nhdfs
