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

#include <string.h>

namespace nhdfs
{

void setParam(std::string key, Napi::Object params, std::function<void(std::string)> f)
{
    if (params.Has(key))
    {
        Napi::Value v = params[key];
        std::string vs = v.As<Napi::String>();
        f(vs);
    }
}

hdfsFS connect(std::string node, tPort port, Napi::Object params, Napi::Env env)
{
    //std::cout << "trying to connect " << node << ":" << port << std::endl;
    struct hdfsBuilder *builder = hdfsNewBuilder();
    hdfsBuilderSetNameNode(builder, node.c_str());
    if (port > 0)
    {
        hdfsBuilderSetNameNodePort(builder, port);
    }
    setParam(USER, params, [builder](std::string user) { hdfsBuilderSetUserName(builder, user.c_str()); } );
    setParam(TOKEN, params, [builder](std::string token) { hdfsBuilderSetToken(builder, token.c_str()); } );
    setParam(TICKETPATH, params, [builder](std::string path) { hdfsBuilderSetKerbTicketCachePath(builder, path.c_str()); } );
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
             //  InstanceMethod("Copy", &FileSystem::Copy),
             InstanceMethod("GetWorkingDirectory", &FileSystem::GetWorkingDirectory),
             InstanceMethod("SetWorkingDirectory", &FileSystem::SetWorkingDirectory),
             InstanceMethod("CreateDirectory", &FileSystem::CreateDirectory),
             InstanceMethod("Delete", &FileSystem::Delete),
             InstanceMethod("SetReplication", &FileSystem::SetReplication),
             InstanceMethod("List", &FileSystem::List),
             InstanceMethod("GetPathInfo", &FileSystem::GetPathInfo),
             InstanceMethod("GetDefaultBlockSize", &FileSystem::GetDefaultBlockSize),
             InstanceMethod("GetCapacity", &FileSystem::GetCapacity),
             InstanceMethod("GetUsed", &FileSystem::GetUsed),
             InstanceMethod("Chown", &FileSystem::Chown),
             InstanceMethod("Chmod", &FileSystem::Chmod),
             InstanceMethod("Utime", &FileSystem::Utime),
             InstanceMethod("Truncate", &FileSystem::Truncate)
             });            
    constructor = Napi::Persistent(t);
    constructor.SuppressDestruct();
    (exports).Set(Napi::String::New(env, "FileSystem"), t);
}

FileSystem::FileSystem(const Napi::CallbackInfo &info) : Napi::ObjectWrap<FileSystem>(info)
{
    REQUIRE_ARGUMENTS(3)
    REQUIRE_ARGUMENT_STRING(0, nn);
    REQUIRE_ARGUMENT_UINT(1, p);
    REQUIRE_ARGUMENT_OBJECT(2, params);

    this->nameNode = nn;
    this->port = p;
    this->params = params;
    this->fs = connect(this->nameNode, this->port, this->params, info.Env());
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

// Napi::Value FileSystem::Copy(const Napi::CallbackInfo &info)
// {
//     REQUIRE_ARGUMENTS(3)
//     REQUIRE_ARGUMENT_STRING(0, oldPath)
//     REQUIRE_ARGUMENT_STRING(1, newPath)
//     REQUIRE_ARGUMENT_FUNCTION(2, cb)
//     std::function<int()> f = [this, oldPath, newPath] {
//         return hdfsCopy(fs, oldPath.c_str(), fs, newPath.c_str());
//     };
//     SimpleResWorker::Start(f, cb);
//     return info.Env().Null();
// }

Napi::Value FileSystem::GetWorkingDirectory(const Napi::CallbackInfo &info)
{
    REQUIRE_ARGUMENTS(3);
    REQUIRE_ARGUMENT_BUFFER(0, buffer)
    REQUIRE_ARGUMENT_INT(1, l)
    REQUIRE_ARGUMENT_FUNCTION(2, cb)
    char *b = buffer.Data();
    std::function<int()> f = [this, b, l] {
        char *res = hdfsGetWorkingDirectory(fs, b, l);
        if (!res)
            return -1;
        int s = strnlen(res, l);
        return s;
    };
    SimpleResWorker::Start(f, cb);
    return info.Env().Null();
}

Napi::Value FileSystem::SetWorkingDirectory(const Napi::CallbackInfo &info)
{
    REQUIRE_ARGUMENTS(2)
    REQUIRE_ARGUMENT_STRING(0, path)
    REQUIRE_ARGUMENT_FUNCTION(1, cb)
    std::function<int()> f = [this, path] {
        int r = hdfsSetWorkingDirectory(fs, path.c_str());
        return r;
    };
    SimpleResWorker::Start(f, cb);
    return info.Env().Null();
}

Napi::Value FileSystem::CreateDirectory(const Napi::CallbackInfo &info)
{
    REQUIRE_ARGUMENTS(2);
    REQUIRE_ARGUMENT_STRING(0, path)
    REQUIRE_ARGUMENT_FUNCTION(1, cb)

    std::function<int()> f = [this, path] {
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
    std::function<int()> f = [this, path, recursive] {
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
    std::function<int()> f = [this, path, repNum] {
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

/**
 * hdfsGetDefaultBlockSize - Get the default blocksize.
 * @return the default blocksize, or -1 on error.
 */
 Napi::Value FileSystem::GetDefaultBlockSize(const Napi::CallbackInfo &info)
 {
    REQUIRE_ARGUMENTS(1);
    Napi::Function cb = info[0].As<Napi::Function>();
    std::function<res_ptr<tOffset>()> f = [this] (){
        tOffset r = hdfsGetDefaultBlockSize(fs);      
        AsyncResult<tOffset> * ar = new AsyncResult<tOffset>(0, r);
        return res_ptr<tOffset>(ar);
    };
    ValueWorker<tOffset>::Start(f, cb);
    return info.Env().Null();
 }

/**
 * hdfsGetCapacity - Return the raw capacity of the filesystem.
 * @return Returns the raw-capacity; -1 on error.
 */
Napi::Value FileSystem::GetCapacity(const Napi::CallbackInfo &info)
{
    REQUIRE_ARGUMENTS(1);
    Napi::Function cb = info[0].As<Napi::Function>();
    std::function<res_ptr<tOffset>()> f = [this] (){
        tOffset r = hdfsGetCapacity(fs);      
        AsyncResult<tOffset> * ar = new AsyncResult<tOffset>(0, r);
        return res_ptr<tOffset>(ar);
    };
    ValueWorker<tOffset>::Start(f, cb);
    return info.Env().Null();
}

/**
 * hdfsGetUsed - Return the total raw size of all files in the filesystem.
 * @return Returns the total-size; -1 on error.
 */
Napi::Value FileSystem::GetUsed(const Napi::CallbackInfo &info)
{
    REQUIRE_ARGUMENTS(1);
    Napi::Function cb = info[0].As<Napi::Function>();
    std::function<res_ptr<tOffset>()> f = [this] (){
        tOffset r = hdfsGetUsed(fs);      
        AsyncResult<tOffset> * ar = new AsyncResult<tOffset>(0, r);
        return res_ptr<tOffset>(ar);
    };
    ValueWorker<tOffset>::Start(f, cb);
    return info.Env().Null();
}

/**
 * Change the user and/or group of a file or directory.
 * @param path          the path to the file or directory
 * @param owner         User string.  Set to NULL for 'no change'
 * @param group         Group string.  Set to NULL for 'no change'
 */
Napi::Value FileSystem::Chown(const Napi::CallbackInfo &info)
{
    REQUIRE_ARGUMENTS(4)
    REQUIRE_ARGUMENT_STRING(0, path)
    std::string owner = ( info[1].IsUndefined() || info[1].IsNull() ) ? std::string() : info[1].As<Napi::String>();
    std::string group = ( info[2].IsUndefined() || info[2].IsNull() ) ? std::string() : info[2].As<Napi::String>();
    REQUIRE_ARGUMENT_FUNCTION(3, cb)
    std::function<int()> f = [this, path, owner, group] {
        const char * o = ( owner.empty() ) ? nullptr : owner.c_str(); 
        const char * g = ( group.empty() ) ? nullptr : group.c_str(); 
        return hdfsChown(fs, path.c_str(), o, g);
    };
    SimpleResWorker::Start(f, cb);
    return info.Env().Null();
}
/**
 * hdfsChmod
 * @param fs The configured filesystem handle.
 * @param path the path to the file or directory
 * @param mode the bitmask to set it to
 */
Napi::Value FileSystem::Chmod(const Napi::CallbackInfo &info)
{
    REQUIRE_ARGUMENTS(3)
    REQUIRE_ARGUMENT_STRING(0, path)
    REQUIRE_ARGUMENT_INT(1, mode)
    REQUIRE_ARGUMENT_FUNCTION(2, cb)
    std::function<int()> f = [this, path, mode] {
        return hdfsChmod(fs, path.c_str(), mode);
    };
    SimpleResWorker::Start(f, cb);
    return info.Env().Null();
}

/**
 * hdfsUtime
 * @param path the path to the file or directory
 * @param mtime new modification time or -1 for no change
 * @param atime new access time or -1 for no change
 */
 Napi::Value FileSystem::Utime(const Napi::CallbackInfo &info)
 {
    REQUIRE_ARGUMENTS(4)
    REQUIRE_ARGUMENT_STRING(0, path)
    REQUIRE_ARGUMENT_LONG(1, mtime)
    REQUIRE_ARGUMENT_LONG(2, atime)
    REQUIRE_ARGUMENT_FUNCTION(3, cb)
    std::function<int()> f = [this, path, mtime, atime] {
        return hdfsUtime(fs, path.c_str(), mtime, atime);
    };
    SimpleResWorker::Start(f, cb);
    return info.Env().Null();
 }

/**
 * hdfsTruncate - Truncate the file in the indicated path to the indicated size.
 * @param fs The configured filesystem handle.
 * @param path the path to the file.
 * @param pos the position the file will be truncated to.
 * @return true if and client does not need to wait for block recovery,
 * false if client needs to wait for block recovery.
 */
 Napi::Value FileSystem::Truncate(const Napi::CallbackInfo &info) 
 {
    REQUIRE_ARGUMENTS(3)
    REQUIRE_ARGUMENT_STRING(0, path)
    REQUIRE_ARGUMENT_LONG(1, pos)
    REQUIRE_ARGUMENT_FUNCTION(2, cb)
    std::function<res_ptr<bool>()> f = [this, path, pos] {
        int shouldWait = 0;
        int res = hdfsTruncate(fs, path.c_str(), pos, &shouldWait); //TODO: node gets null
        AsyncResult<bool> * ar = new AsyncResult<bool>(res, shouldWait);
        return res_ptr<bool>(ar);
    };
    ValueWorker<bool>::Start(f, cb);
    return info.Env().Null();
 }

} // namespace nhdfs
