
#ifndef NHDFS_MACROS_H_
#define NHDFS_MACROS_H_

#include <napi.h>

#define REQUIRE_ARGUMENTS(n)                                                   \
    if (info.Length() < (n)) {                                                 \
        Napi::TypeError::New(info.Env(), "Expected " #n "arguments").ThrowAsJavaScriptException();                \
    }


#define REQUIRE_ARGUMENT_FUNCTION(i, var)                                      \
    if (info.Length() <= (i) || !info[i].IsFunction()) {                      \
        Napi::TypeError::New(info.Env(), "Argument " #i " must be a function").ThrowAsJavaScriptException();      \
    }                                                                          \
    Napi::Function var = info[i].As<Napi::Function>();


#define REQUIRE_ARGUMENT_STRING(i, var)                                        \
    if (info.Length() <= (i) || !info[i].IsString()) {                        \
        Napi::TypeError::New(info.Env(), "Argument " #i " must be a string").ThrowAsJavaScriptException();        \
    }                                                                          \
    std::string var = info[i].As<Napi::String>();

#define REQUIRE_ARGUMENT_UINT(i, var)                                        \
    if (info.Length() <= (i) || !info[i].IsNumber()) {                        \
        Napi::TypeError::New(info.Env(), "Argument " #i " must be a integer").ThrowAsJavaScriptException();        \
    }                                                                          \
    int var = info[i].As<Napi::Number>().Uint32Value(); 

#define REQUIRE_ARGUMENT_OBJECT(i, var)                                        \
    if (info.Length() <= (i) || !info[i].IsObject()) {                        \
        Napi::TypeError::New(info.Env(), "Argument " #i " must be a object").ThrowAsJavaScriptException();        \
    }                                                                          \
    Napi::Object var = info[i].As<Napi::Object>();

#define REQUIRE_ARGUMENT_INT(i, var)                                        \
    if (info.Length() <= (i) || !info[i].IsNumber()) {                        \
        Napi::TypeError::New(info.Env(), "Argument " #i " must be a integer").ThrowAsJavaScriptException();        \
    }                                                                          \
    int var = info[i].As<Napi::Number>().Int32Value();     

#define REQUIRE_ARGUMENT_LONG(i, var)                                        \
    if (info.Length() <= (i) || !info[i].IsNumber()) {                        \
        Napi::TypeError::New(info.Env(), "Argument " #i " must be a integer").ThrowAsJavaScriptException();        \
    }                                                                          \
    int64_t var = info[i].As<Napi::Number>().Int64Value(); 

#define REQUIRE_ARGUMENT_BUFFER(i, var)                                        \
    if (info.Length() <= (i) || !info[i].IsBuffer()) {                        \
        Napi::TypeError::New(info.Env(), "Argument " #i " must be a byte buffer").ThrowAsJavaScriptException();        \
    }                                                                          \
    Napi::Buffer<char> var = info[i].As<Napi::Buffer<char>>();  

#define OPTIONAL_ARGUMENT_FUNCTION(i, var)                                     \
    Napi::Function var;                                                       \
    if (info.Length() > i && !info[i].IsUndefined()) {                        \
        if (!info[i].IsFunction()) {                                          \
            Napi::TypeError::New(info.Env(), "Argument " #i " must be a function").ThrowAsJavaScriptException();  \
        }                                                                      \
        var = info[i].As<Napi::Function>();                                  \
    }    

#define NAPISTRING(env, s) Napi::String::New(env, s)
#define NAPIINT(env, i) Napi::Number::New(env, i)

#define REQUIRE_ARGUMENT_FS(i, var)                     \
    if ( info.Length() <= i && ! FileSystem::HasInstance(info[i]))                              \
    {                                                   \
        Napi::TypeError::New(info.Env(), "FileSystem object expected").ThrowAsJavaScriptException();   \
    }                                                   \
    FileSystem * var = Napi::ObjectWrap<FileSystem>::Unwrap(info[i].As<Napi::Object>());
    
#endif // NHDFS_MACROS_H_