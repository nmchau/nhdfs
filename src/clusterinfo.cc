
#include <iostream>
#include <napi.h>
#include <utility>
#include <initializer_list>

#include "clusterinfo.h"
#include "workers.h"
#include "macros.h"

namespace nhdfs
{

Napi::Object nameNodeToObject(Namenode *node, Napi::Env env)
{
    Napi::Object res = Napi::Object::New(env);

    res.Set(NAPISTRING(env, "rpcAddress"), NAPISTRING(env, node->rpc_addr));
    res.Set(NAPISTRING(env, "httpAddress"), NAPISTRING(env, node->http_addr));
    return res;
}

class HANameNodesWorker : public Napi::AsyncWorker
{
  public:
    HANameNodesWorker(const std::string s, Napi::Function cb) : AsyncWorker(cb), service(s) {}

    virtual ~HANameNodesWorker()
    {
        if (this->nodes)
            hdfsFreeNamenodeInformation(this->nodes, this->size);
    }

    void Execute() override
    {
        this->nodes = hdfsGetHANamenodes(this->service.c_str(), &this->size);
        if ( ! this->nodes)
        {
            this->error = createAsyncError(errno, hdfsGetLastError());
        }
    }

    void OnOK() override
    {
        auto err = getErr();
        if (this->nodes)
        {
            Napi::Array result(Napi::Array::New(this->Env(), this->size));
            for (int i = 0; i < this->size; i++)
            {
                Napi::Object o = nameNodeToObject(&this->nodes[i], this->Env());
                (result).Set(i, o);
            }
            this->Callback().MakeCallback(this->Receiver().Value(), std::initializer_list<napi_value>{err, result});
        }
        else
        {
            this->Callback().MakeCallback(this->Receiver().Value(), std::initializer_list<napi_value>{err});
        }
    }

  private:
    Napi::Value getErr()
    {
        return converError(this->error, this);
    }

  private:
    std::shared_ptr<AsyncError> error;
    Namenode *nodes = nullptr;
    int size = 0;
    const std::string service;
};

Napi::FunctionReference ClusterInfo::constructor;

void ClusterInfo::Init(Napi::Env env, Napi::Object exports)
{
    Napi::HandleScope scope(env);
    Napi::Function t =
        DefineClass(
            env,
            "ClusterInfo",
            {InstanceMethod("GetHANamenodes", &ClusterInfo::GetHANamenodes)});
    constructor = Napi::Persistent(t); //TODO : ??
    constructor.SuppressDestruct();
    (exports).Set(Napi::String::New(env, "ClusterInfo"), t);
}

ClusterInfo::ClusterInfo(const Napi::CallbackInfo &info) : Napi::ObjectWrap<ClusterInfo>(info)
{
}

ClusterInfo::~ClusterInfo()
{
}

/**
  * If hdfs is configured with HA namenode, return all namenode informations as an array.
  * Else return NULL.
  * Using configure file which is given by environment parameter LIBHDFS3_CONF
  * or "hdfs-client.xml" in working directory.
  * @param nameservice hdfs name service id.
  * @return return an array of all namenode information.
  */
Napi::Value ClusterInfo::GetHANamenodes(const Napi::CallbackInfo &info)
{
    REQUIRE_ARGUMENTS(2);
    std::string service = info[0].As<Napi::String>();
    Napi::Function cb = info[1].As<Napi::Function>();
    HANameNodesWorker *worker = new HANameNodesWorker(service, cb);
    worker->Queue();
    return info.Env().Null();
}

} //namespace