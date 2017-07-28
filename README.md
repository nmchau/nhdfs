nhdfs
========================

**Node.js Native HDFS library wrapper**

#### Native hdfs library: 

 - [Pivotal libhdfs3](https://github.com/Pivotal-Data-Attic/pivotalrd-libhdfs3)
 

### Requirement

To build nhdfs the following is required

    Node.js(8.0+) the library uses ABI-stable N-API for building the native Addon.
    cmake (2.8+)                    http://www.cmake.org/
    boost (tested on 1.53+)         http://www.boost.org/
    google protobuf                 http://code.google.com/p/protobuf/
    libxml2                         http://www.xmlsoft.org/
    kerberos                        http://web.mit.edu/kerberos/
    libuuid                         http://sourceforge.net/projects/libuuid/
    libgsasl                        http://www.gnu.org/software/gsasl/

### Usage

**Note:** the module must be installed before use and N-API must be enabled on the 
command-line by adding --napi-modules.

``` js
const FileSystem = require('nhdfs').FileSystem;

const fs = new FileSystem("namenode", 9000);
fs.list(".").then((list) => {
    list.forEach((element) => {
        console.log(element);
    });
})
.catch( (err) => {
    console.log(err);
})
```
