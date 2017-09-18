#ifndef NHDFS_CLUSTERINFO_H_
#define NHDFS_CLUSTERINFO_H_

#include <string>
#include <napi.h>
#include <hdfs/hdfs.h>

namespace nhdfs
{

class ClusterInfo;

class ClusterInfo : public Napi::ObjectWrap<ClusterInfo>
{
  public:
    static Napi::FunctionReference constructor;

    static void Init(Napi::Env env, Napi::Object exports);

    ClusterInfo(const Napi::CallbackInfo &info);
    ~ClusterInfo();

    /**
    * If hdfs is configured with HA namenode, return all namenode informations as an array.
    * Else return NULL.
    * Using configure file which is given by environment parameter LIBHDFS3_CONF
    * or "hdfs-client.xml" in working directory.
    * @param nameservice hdfs name service id.
    * @return return an array of all namenode information.
    */
    Napi::Value GetHANamenodes(const Napi::CallbackInfo &info);

  private:
    
    int close();

    hdfsFS fs;
    std::string path;
    hdfsFile file;
};

}

#endif //NHDFS_CLUSTERINFO_H_