
const bindings = require('bindings')('nhdfs');
const NativeFs = bindings.FileSystem;
const NativeReader = bindings.FileReader;
const NativeWriter = bindings.FileWriter;
const { Readable, Writable } = require('stream');

class FileSystem {

    constructor(nameNode, port) {
        this.fs = new NativeFs(nameNode, port);
    }

    list(path) {
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
                resolve( ! err ); //TODO: if err is real
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
        this.poolSize = 64 * 1024; //TODO: param
        this.reader = reader;
        this.on('end', () => {
            console.log("done with read");
            this.destroy();
        });
        this.reader.Open((err) => {
            if ( err ) {
                this.emit('error', err);
            } else {
                this.opened = true;
                this.emit('open', 'open'); 
                //this.read();
            }
        })
    }

    _read(size) {
        if (this.destroyed) return;
        if ( ! this.opened ) {
             return this.once('open', () => {
                this._read(size);
            });
        }
        let s = ( size == undefined ) ? this.poolSize : size;
        const pool = Buffer.allocUnsafe(s);
        const onread = (err, bytesRead) => {
            console.log("on read");
            if (err) {
                console.log("err");
                console.log(err);
                this.destroy();
                this.emit('error', err);
            } else {
                var b = null;
                console.log(`on read with data ${bytesRead}`);
                if (bytesRead > 0) {
                    console.log("returned something len: " + bytesRead );
                    b = pool.slice(0, bytesRead);
                    console.log(b.toString('utf8'));
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

class HWriteStream extends Writable {

    constructor(writer) {
        super();
        this.bytesWritten = 0;
        this.writer = writer;
        this.on('end', () => {
            console.log("done with write");
            this.destroy();
        });
        this.once('finish', () => {
            console.log("on finish ins");
            this.close();
        });
        this.opened = false; 
        this.writer.Open((err) => {
            if ( err ) {
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
        if ( ! this.opened ) {
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
        this.writer.Close((er) => {
            if (er)
                this.emit('error', er);
            else
                this.emit('close');
        });
    }

}
module.exports.FileSystem = FileSystem;