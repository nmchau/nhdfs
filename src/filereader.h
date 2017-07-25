#ifndef NH_FILEREADER_H
#define NH_FILEREADER_H

#include <string>
#include <napi.h>
#include <hdfs/hdfs.h>

namespace nhdfs
{

class FileReader;

class FileReader : public Napi::ObjectWrap<FileReader>
{
  public:
    static Napi::FunctionReference constructor;

    static void Init(Napi::Env env, Napi::Object exports);

    FileReader(const Napi::CallbackInfo &info);
    ~FileReader();

    Napi::Value Read(const Napi::CallbackInfo &info);

    Napi::Value Close(const Napi::CallbackInfo &info);

  private:
    
    int close();

    hdfsFS fs;
    std::string path;
    hdfsFile file;
};

}

#endif //NH_FILEREADER_H