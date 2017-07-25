
const bindings = require('bindings')('nhdfs');
const NativeFs = bindings.FileSystem;
const NativeReader = bindings.FileReader;
const { Readable } = require('stream');

class FileSystem {

    constructor(nameNode, port) {
        this.fs = new NativeFs(nameNode, port);
    }

    ls(path) {
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
        return new Promise((resolve, reject) => {
            this.fs.Delete(path, recursive, (err) => {
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
                if (err) {
                    reject(err);
                } else {
                    resolve(data);
                }
            })
        });
    }

    isDir(path) {
        //
    }

    isFile(path) {
        //
    }

    createReadStream(path, options) {
        let r = new NativeReader(path, this.fs);
        return new HReadStream(r);
    }

    //def read_parquet(self, path, columns=None, metadata=None, schema=None, nthreads=1):
}

class HReadStream extends Readable {
    constructor(reader) {
        super();
        this.poolSize = 1024; //TODO: param
        this.reader = reader;
        this.on('end', () => {
            console.log("done with read");
            this.destroy();
        });
        this.emit('open', 'open'); //TODO:
        this.read();
    }

    _read(size) {
        if (this.destroyed) return;
        const pool = Buffer.allocUnsafe(this.poolSize);
        const onread = ( err, bytesRead) => {
            //console.log("on read");
            if (err) {
                this.destroy();
                this.emit('error', err);
            } else {
                var b = null;
                if (bytesRead > 0) {
                    //console.log("returned something len: " + bytesRead );
                    b = pool.slice(0, bytesRead);
                    //console.log(b.toString('utf8'));
                }
                this.push(b);
            }
        }
        this.reader.Read(pool, this.poolSize, onread);
    }

    _destroy(err1, cb) {
        console.log("destroy");
        this.close((err2) => {
            cb(err1 || err2);
        });
    }

    close(cb) {
        console.log("close");
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

module.exports.FileSystem = FileSystem;