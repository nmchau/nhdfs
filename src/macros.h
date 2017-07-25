
#ifndef NH__MACROS_H
#define NH__MACROS_H

#include <napi.h>

#define REQUIRE_ARGUMENTS(n)                                                   \
    if (info.Length() < (n)) {                                                 \
        Napi::TypeError::New(env, "Expected " #n "arguments").ThrowAsJavaScriptException();                \
    }


#define REQUIRE_ARGUMENT_FUNCTION(i, var)                                      \
    if (info.Length() <= (i) || !info[i].IsFunction()) {                      \
        Napi::TypeError::New(env, "Argument " #i " must be a function").ThrowAsJavaScriptException();      \
    }                                                                          \
    Napi::Function var = info[i].As<Napi::Function>();


#define REQUIRE_ARGUMENT_STRING(i, var)                                        \
    if (info.Length() <= (i) || !info[i].IsString()) {                        \
        Napi::TypeError::New(env, "Argument " #i " must be a string").ThrowAsJavaScriptException();        \
    }                                                                          \
    std::string var = info[i].As<Napi::String>();

#define REQUIRE_ARGUMENT_UINT(i, var)                                        \
    if (info.Length() <= (i) || !info[i].IsNumber()) {                        \
        Napi::TypeError::New(env, "Argument " #i " must be a integer").ThrowAsJavaScriptException();        \
    }                                                                          \
    int var = info[i].As<Napi::Number>().Uint32Value(); 
    
#define REQUIRE_ARGUMENT_INT(i, var)                                        \
    if (info.Length() <= (i) || !info[i].IsNumber()) {                        \
        Napi::TypeError::New(env, "Argument " #i " must be a integer").ThrowAsJavaScriptException();        \
    }                                                                          \
    int var = info[i].As<Napi::Number>().Int32Value();     


#define OPTIONAL_ARGUMENT_FUNCTION(i, var)                                     \
    Napi::Function var;                                                       \
    if (info.Length() > i && !info[i].IsUndefined()) {                        \
        if (!info[i].IsFunction()) {                                          \
            Napi::TypeError::New(env, "Argument " #i " must be a function").ThrowAsJavaScriptException();  \
        }                                                                      \
        var = info[i].As<Napi::Function>();                                  \
    }    

#define NAPISTRING(env, s) Napi::String::New(env, s)
    
#endif // NH__MACROS_H