
const bindings = require('bindings')('nhdfs');
const NativeFs = bindings.FileSystem;
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
                    resolve(data);
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
     * hdfsChmod
     * @param fs The configured filesystem handle.
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

    createReadStream(path, options) {
        let r = new NativeReader(path, this.fs);  //TODO: make async
        return new HReadStream(r);
    }

    createWriteStream(path, options) {
        let w = new NativeWriter(path, this.fs); //TODO: make async
        return new HWriteStream(w);
    }
}

class HReadStream extends Readable {
    constructor(reader) {
        super();
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

    constructor(writer) {
        super();
        this.bytesWritten = 0;
        this.writer = writer;
        this.on('end', () => {
            this.destroy();
        });
        this.once('finish', () => {
            this.close();
        });
        this.opened = false;
        this.writer.Open((err) => {
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