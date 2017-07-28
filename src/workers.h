#ifndef NHDFS_WORKERS_H_
#define NHDFS_WORKERS_H_

#include <errno.h>
#include <napi.h>

#include "macros.h"

namespace nhdfs
{

/**
* Worker to call simple async op and return result as callback
**/
class SimpleResWorker : public Napi::AsyncWorker
{
  public:
    static void Start(std::function<int()> f, Napi::Function cb)
    {
        SimpleResWorker *worker = new SimpleResWorker(f, cb);
        worker->Queue();
    }

    SimpleResWorker(std::function<int()> f, Napi::Function cb) : AsyncWorker(cb), func(f), msg("") {}

    void Execute() override
    {
        this->res = func();
        if (this->res < 0)
        {
            this->err = errno;
            this->msg = hdfsGetLastError();
        }
    }
    void OnOK() override
    {
        auto err = getErr();
        auto v = Napi::Number::New(this->Env(), this->res);
        this->Callback().MakeCallback(this->Receiver().Value(), std::initializer_list<napi_value>{err, v});
    }

  private:
    Napi::Value getErr()
    {
        Napi::Env env = this->Env();
        if (this->res < 0)
        {           
            Napi::Object e = Napi::Object::New(env);
            e.Set(NAPISTRING(env, "message"), NAPISTRING(env, this->msg));
            e.Set(NAPISTRING(env, "errno"), NAPIINT(env, this->err));
            return e;
        }
        else
            return env.Null();
    }

  private:
    std::function<int()> func;
    int res = 0;
    std::string msg;
    int err = 0;
};

} //namespace nhdfs

#endif //NHDFS_WORKERS_H_