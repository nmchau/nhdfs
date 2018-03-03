
const bindings = require('bindings')('nhdfs');
const NativeFs = bindings.FileSystem;
const NativeClusterInfo = bindings.ClusterInfo;
const NativeReader = bindings.FileReader;
const NativeWriter = bindings.FileWriter;
const { Readable, Writable } = require('stream');

const lfs = require('fs');

/**
 * Default Max Path Length 
 */
const DEFAULT_PATH_LENGTH = 1024;
/**
 * Default Service
 */
const DEFAULT_SERVICE = "default";

const HDFS_SITE = "hdfs-site.xml";

function checkFile(filePath) {
    try {
        lfs.accessSync(filePath, lfs.constants.R_OK);
        return true;
    } catch (e) {
        return false;
    }
}

function setConfPath(configurationPath, useHadoopConfEnv) {
    if (configurationPath) {
        process.env.LIBHDFS3_CONF = configurationPath;
    } else if ((!process.env.LIBHDFS3_CONF) && useHadoopConfEnv) {
        if (process.env.HADOOP_CONF_DIR && checkFile(`${process.env.HADOOP_CONF_DIR}/${HDFS_SITE}`)) {
            process.env.LIBHDFS3_CONF = `${process.env.HADOOP_CONF_DIR}/${HDFS_SITE}`;
        } else if (process.env.HADOOP_INSTALL && checkFile(`${process.env.HADOOP_INSTALL}/hadoop/conf/${HDFS_SITE}`)) {
            process.env.LIBHDFS3_CONF = `${process.env.HADOOP_INSTALL}/hadoop/conf/${HDFS_SITE}`
        }
    }
}

function getOptions(options, defaultOptions) {
    if (options === null || options === undefined || typeof options === 'function') {
        return defaultOptions;
    }
    if (typeof options !== 'object') {
        throw new Error('options must be an object'); //TODO:
    }
    return options;
}
  
function copyObject(source) {
    var target = {};
    for (var key in source)
        target[key] = source[key];
    return target;
}

const createFS = (
    {
        service,
        port,
        configurationPath = '',
        useHadoopConfEnv = true,
        user = '',
        kerbTicketCachePath = '',
        authToken = '',
        options = {} } = {}) => {
    setConfPath(configurationPath, useHadoopConfEnv);
    return new FileSystem({ service: service, port: port, user: user, options: options });
}

const createClusterInfo = (
    {
        configurationPath = '',
        useHadoopConfEnv = true,
    } = {}) => {
    setConfPath(configurationPath, useHadoopConfEnv);
    return new ClusterInfo();
}

class ClusterInfo {

    constructor() {
        this.ci = new NativeClusterInfo();
    }

    /**
     * If hdfs is configured with HA namenode, return all namenode informations as an array.
     * Else return NULL.
     * Using configure file which is given by environment parameter LIBHDFS3_CONF
     * or "hdfs-client.xml" in working directory.
     * or by "configurationPath" and "useHadoopConfEnv" factory method paramters,
     * @param nameservice hdfs name service id.
     * @return return an array of all namenode information.
     */
    namenodes(service) {
        return new Promise((resolve, reject) => {
            this.ci.GetHANamenodes(service, (err, data) => {
                if (err) {
                    reject(err);
                } else {
                    resolve(data);
                }
            })
        });
    }
}

class FileSystem {

    constructor({ service, port, user, kerbTicketCachePath, authToken, options }) {
        this.maxPath = (Number.isInteger(options.maxPathLength) && options.maxPathLength > 0) ? options.maxPathLength : DEFAULT_PATH_LENGTH;
        if (!port) port = 0;
        if (!service) service = DEFAULT_SERVICE;
        let params = {}
        if (user) params.user = user;
        if (kerbTicketCachePath) params.kerbTicketCachePath = kerbTicketCachePath;
        if (authToken) params.authToken = authToken;
        this.fs = new NativeFs(service, port, params);
    }

    list(path = ".") {
        return new Promise((resolve, reject) => {
            this.fs.List(path, (err, data) => {
                if (err) {
                    reject(err);
                } else {
                    const d = data.map( f => {
                        if ( f.path.startsWith('//') ) { //todo: strange bug from libhdfs3 ????
                            f.path = f.path.slice(1);
                        } else if ( f.path.endsWith('/') && f.path.length > 1 ) { 
                            f.path = f.path.slice(0, -1);
                        }
                        return f;
                    });
                    resolve(d);
                }
            })
        });
    }

    stats(path) {
        return new Promise((resolve, reject) => {
            this.fs.GetPathInfo(path, (err, data) => {
                if (err) {
                    reject(err);
                } else {
                    resolve(data);
                }
            })
        });
    }

    /**
     * Delete file.
     * @param path The path of the file.
     * @param recursive if path is a directory, the directory is deleted else throws an exception. In
     * case of a file the recursive argument is irrelevant.
     */
    delete(path, recursive = false) {
        const r = (recursive) ? 1 : 0;
        return new Promise((resolve, reject) => {
            this.fs.Delete(path, r, (err) => {
                if (err) {
                    reject(err);
                } else {
                    resolve();
                }
            })
        });
    }

    /**
     * Create Directory with parents (async)
     * @param {String} path - path to directory
     * @return {Promise}  
     */
    mkdir(path) {
        return new Promise((resolve, reject) => {
            this.fs.CreateDirectory(path, (err) => {
                if (err) {
                    reject(err);
                } else {
                    resolve();
                }
            })
        });
    }

    exists(path) {
        return new Promise((resolve, reject) => {
            this.fs.Exists(path, (err, data) => {
                resolve(!err); //TODO: if err is real
            })
        });
    }

    async isDirectory(path) {
        let s = await this.stats(path);
        return s.type === "directory";
    }

    async isFile(path) {
        let s = await this.stats(path);
        return s.type === "file";
    }

    rename(oldPath, newPath) {
        return new Promise((resolve, reject) => {
            this.fs.Rename(oldPath, newPath, (err) => {
                if (err) {
                    reject(err);
                } else {
                    resolve();
                }
            })
        });
    }

    // copy(oldPath, newPath) {
    //     return new Promise((resolve, reject) => {
    //         this.fs.Copy(oldPath, newPath, (err) => {
    //             if (err) {
    //                 reject(err);
    //             } else {
    //                 resolve();
    //             }
    //         })
    //     });
    // }

    setWorkingDirectory(path) {
        return new Promise((resolve, reject) => {
            this.fs.SetWorkingDirectory(path, (err) => {
                if (err) {
                    reject(err);
                } else {
                    resolve();
                }
            })
        });
    }

    getWorkingDirectory(pathLength = this.maxPath) {
        const l = (Number.isInteger(pathLength) && Number.isInteger(pathLength) > 0) ? pathLength : pathLength;
        return new Promise((resolve, reject) => {
            const buf = Buffer.alloc(l);
            this.fs.GetWorkingDirectory(buf, l, (err, size) => {
                if (err) {
                    reject(err);
                } else {
                    let b = buf.slice(0, size);
                    const str = b.toString('utf8');
                    resolve(str);
                }
            })
        });
    }

    /**
     * Set the replication of the specified file to the supplied value
     * @param path The path of the file.
     * @param replication Replication factor.
     */
    setReplication(path, replication) {
        return new Promise((resolve, reject) => {
            this.fs.SetReplication(path, replication, (err) => {
                if (err) {
                    reject(err);
                } else {
                    resolve();
                }
            })
        });
    }

    /**
     * Get the default blocksize.
     * @return The default blocksize, or error.
     */
    getDefaultBlockSize() {
        return new Promise((resolve, reject) => {
            this.fs.GetDefaultBlockSize((err, size) => {
                if (err) {
                    reject(err);
                } else {
                    resolve(size);
                }
            })
        });
    }

    /**
     *  Return the raw capacity of the filesystem.
     * @return The raw-capacity or error. 
     */
    getCapacity() {
        return new Promise((resolve, reject) => {
            this.fs.GetCapacity((err, size) => {
                if (err) {
                    reject(err);
                } else {
                    resolve(size);
                }
            })
        });
    }

    /**
     * Return the total raw size of all files in the filesystem.
     */
    getUsed() {
        return new Promise((resolve, reject) => {
            this.fs.GetUsed((err, size) => {
                if (err) {
                    reject(err);
                } else {
                    resolve(size);
                }
            })
        });
    }

    /**
     * Change the user and/or group of a file or directory.
     * @param {String}path          the path to the file or directory
     * @param {String}owner         User string.  Set to NULL for 'no change'
     * @param {String}group         Group string.  Set to NULL for 'no change'
     * @return {Promise} 
     */
    chown(path, owner=null, group=null) {
        return new Promise((resolve, reject) => {
            this.fs.Chown(path, owner, group, (err) => {
                if (err) {
                    reject(err);
                } else {
                    resolve();
                }
            })
        });
    }

    /**
     * Change the user and/or group of a file or directory.
     * @param {String}path the path to the file or directory
     * @param {Number}mode the bitmask to set it to
     * @return {Promise} 
     */
    chmod(path, mode)
    {
        return new Promise((resolve, reject) => {
            this.fs.Chmod(path, mode, (err) => {
                if (err) {
                    reject(err);
                } else {
                    resolve();
                }
            })
        });
    }

      /**
     * Utime
     * @param {String}path the path to the file or directory
     * @param {Number}mtime new modification time or -1 for no change
     * @param {Number}atime new access time or -1 for no change
     * @return {Promise}
     */
    utime(path, mtime=-1, atime=-1)
    {
        return new Promise((resolve, reject) => {
            this.fs.Utime(path, mtime, atime, (err) => {
                if (err) {
                    reject(err);
                } else {
                    resolve();
                }
            })
        });
    }

     /**
     * Truncate - Truncate the file in the indicated path to the indicated size.
     * @param {String}path the path to the file.
     * @param {Number}pos the position the file will be truncated to.
     * @return true if and client does not need to wait for block recovery,
     * false if client needs to wait for block recovery.
     */
    truncate(path, pos) {
        return new Promise((resolve, reject) => {
            this.fs.Truncate(path, pos, (err, data) => {
                if (err) {
                    reject(err);
                } else {
                    resolve(data);
                }
            })
        });
    }

    createReadStream(path, options={}) {
        let r = new NativeReader(path, this.fs);  //TODO: make async
        return new HReadStream(r, options);
    }

    createWriteStream(path, options) {
        let w = new NativeWriter(path, this.fs); //TODO: make async
        return new HWriteStream(w, options);
    }
}

class HReadStream extends Readable {
    constructor(reader, options) {
        super(options);
        this.opened = false;
        this.destroyed = false;
        this.closed = false;
        this.poolSize = 64 * 1024; //TODO: param
        this.reader = reader;
        this.on('end', () => {
            this.destroy();
        });
        this.reader.Open((err) => {
            if (err) {
                this.emit('error', err);
            } else {
                this.opened = true;
                //this.read();
                this.emit('open', 'open');
            }
        })
    }

    _read(size) {
        if (this.closed) return;
        if (!this.opened) {
            return this.once('open', () => {
                this._read(size);
            });
        }
        const pool = Buffer.allocUnsafe(size);
        const onread = (err, bytesRead) => {
            if (err) {
                this.destroy();
                this.emit('error', err);
            } else {
                // console.log(`${size} - ${bytesRead}`);
                var b = null;
                if (bytesRead > 0) {
                    b = pool.slice(0, bytesRead);
                }
                this.push(b);
            }
        }
        this.reader.Read(pool, size, onread);
    }

    _destroy(err1, cb) {
        this.close((err2) => {
            cb(err1 || err2);
        });
    }

    close(cb) {
        if (cb) {
            this.once('close', cb);
        }
        if (this.closed) {
            return process.nextTick(() => this.emit('close'));
        }
        this.closed = true;
        this.reader.Close((er) => {
            if (er)
                this.emit('error', er);
            else
                this.emit('close');
        });
    }
}

class HWriteStream extends Writable {

    constructor(writer, options) {
        super();
        this.options = copyObject(options, {});
        if (this.options.replication === undefined) this.options.replication = 0;
        this.bytesWritten = 0;
        this.writer = writer;
        this.on('end', () => {
            this.destroy();
        });
        this.once('finish', () => {
            this.close();
        });
        this.opened = false;
        this.writer.Open(this.options.replication, (err) => {
            if (err) {
                this.emit('error', err);
            } else {
                this.opened = true;
                this.emit('open', 'open');
            }
        })
    }

    _write(data, encoding, cb) {
        if (!(data instanceof Buffer)) {
            return this.emit('error', new Error('Invalid data'));
        }
        if (!this.opened) {
            return this.once('open', () => {
                this._write(data, encoding, cb);
            });
        }
        this.writer.Write(data, data.length, (err, bytes) => {
            if (err) {
                this.destroy();
                return cb(err);
            }
            this.bytesWritten += bytes;
            cb();
        });
    }

    _destroy(err1, cb) {
        this.close((err2) => {
            cb(err1 || err2);
        });
    }

    close(cb) {
        if (cb) {
            this.once('close', cb);
        }
        if (this.closed) {
            return process.nextTick(() => this.emit('close'));
        }
        this.closed = true;
        this.writer.Close((er) => {
            if (er)
                this.emit('error', er);
            else
                this.emit('close');
        });
    }

}
//module.exports.FileSystem = FileSystem;
module.exports.createFS = createFS;
module.exports.createClusterInfo = createClusterInfo;