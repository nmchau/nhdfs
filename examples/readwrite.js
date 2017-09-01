'use strict';

const createFS = require('nhdfs').createFS;
const fs = createFS({service:"namenode", port:9000});
//const fs = createFS({service:"nameservice1"});
//const fs = createFS({service:"nameservice1", configurationPath:'/opt//hadoop/conf/hdfs-site.xml'});
//const fs = createFS({service:"nameservice1", user:"testuser", configurationPath:'/opt/hadoop/conf/hdfs-site.xml'});
//const fs = createFS();

const name = "writertest";

async function write() {
    const out = fs.createWriteStream(name);
    out.on('error', (err) => {
        console.log(err);
    });
    for (let i = 0; i <= 1000; ++i ) {
        var buf = Buffer.from(`{name: 'line ${i}', msg: 'hello world number ${i}}', num: ${i}}\n`, 'utf8');
        out.write(buf);
    }
    out.end(`{name: 'last line 1001', msg: 'later on world number 1001', num: 1001}`);
    return new Promise((resolve, reject) => {
        out.on('finish', () => {
            setTimeout(resolve, 1000);
        })
    });
}

async function read() {
    console.log("starting reading");
    const ins = fs.createReadStream(name);
    ins.on('error', (err) => {
        console.log(err);
    });
    ins.on('data', (data) => {
        console.log(`on data ${data.length}`);
        let s = data.toString('utf8');        
        console.log(s);
    })
    return new Promise((resolve, reject) => {
        ins.on('end', () => {
            ins.close();
            resolve();
        })
    });
}

write().then( () => {
    read();
}).then ( () => {
    console.log("done");
}).catch( (err) => {
    console.log(err);
});