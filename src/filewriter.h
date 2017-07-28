#ifndef NHDFS_FILEWRITER_H_
#define NHDFS_FILEWRITER_H_

#include <napi.h>
#include <hdfs/hdfs.h>

namespace nhdfs
{

class FileWriter;

/**
* Wrapper for file writer.
*
* TODO: options for O_WRONLY|O_APPEND|O_SYNC 
**/
class FileWriter : public Napi::ObjectWrap<FileWriter>
{
  public:
    static Napi::FunctionReference constructor;

    static void Init(Napi::Env env, Napi::Object exports);

    FileWriter(const Napi::CallbackInfo &info);
    ~FileWriter();

    Napi::Value Open(const Napi::CallbackInfo &info);

    Napi::Value Write(const Napi::CallbackInfo &info);
    
    Napi::Value Flush(const Napi::CallbackInfo &info);
    
    Napi::Value HFlush(const Napi::CallbackInfo &info);

    Napi::Value Sync(const Napi::CallbackInfo &info);

    Napi::Value Close(const Napi::CallbackInfo &info);

  private:
    
    int close();

    hdfsFS fs;
    std::string path;
    hdfsFile file;
};

}

#endif //NHDFS_FILEWRITER_H_