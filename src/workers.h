#ifndef NHDFS_WORKERS_H_
#define NHDFS_WORKERS_H_

#include <errno.h>
#include <napi.h>
#include <exception>
#include <hdfs/hdfs.h>
#include <memory>

#include <tuple>

#include "macros.h"

#include <iostream>

namespace nhdfs
{

struct AsyncError
{
    const int err;
    const std::string message;
    AsyncError() : err(0), message() {}
    AsyncError(int e, std::string m) : err(e), message(m) {}
};

/**
* Worker to call simple async op and return result as callback
*/
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

/**
*  Result of native function call
*/
template <typename T>
struct AsyncResult
{
    const int result;
    const T value;
    AsyncResult(int r, T v) : result(r), value(v) {}
};

/**
*  Alias for std::shared_ptr<AsyncResult<T>>
*/
template <typename T>
using res_ptr = std::shared_ptr<AsyncResult<T>>;

/**
*  Convert native function result to JS value
*/
template <typename T>
inline Napi::Value convert(Napi::Env env, T v)
{
    return env.Null();
}

/**
*  Convert native function tOffset result to JS value
*/
template <>
inline Napi::Value convert<tOffset>(Napi::Env env, tOffset v)
{
    return Napi::Number::New(env, v);
}

/**
* Asynchronious Worker to call native functon
*/
template <typename T>
class ValueWorker : public Napi::AsyncWorker
{
  public:
    template <typename S>
    static void Start(std::function<res_ptr<S>()> f, Napi::Function cb)
    {
        ValueWorker<S> *worker = new ValueWorker<S>(f, cb);
        worker->Queue();
    }

    ValueWorker(std::function<res_ptr<T>()> f, Napi::Function cb) : AsyncWorker(cb), func(f) {}

    void Execute() override
    {
        this->result = func();
        if (this->result->result < 0)
        {
            AsyncError * err = new AsyncError(errno, hdfsGetLastError());
            this->error = std::shared_ptr<AsyncError>(err);
        }
    }
    void OnOK() override
    {
        auto err = getErr();
        auto v = convert<T>(this->Env(), this->result->value);
        this->Callback().MakeCallback(this->Receiver().Value(), std::initializer_list<napi_value>{err, v});
    }

  private:
    Napi::Value getErr()
    {
        Napi::Env env = this->Env();
        if ( this->error ) {
            Napi::Object e = Napi::Object::New(env);
            e.Set(NAPISTRING(env, "message"), NAPISTRING(env, this->error->message));
            e.Set(NAPISTRING(env, "errno"), NAPIINT(env, this->error->err));
            return e;
        } else {
            return env.Null();
        }
    }

  private:
    std::function<res_ptr<T>()> func;
    res_ptr<T> result;
    std::shared_ptr<AsyncError> error;
};

} //namespace nhdfs

#endif //NHDFS_WORKERS_H_