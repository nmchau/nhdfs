'use strict';

const timeout = require("./utils").timeout

const createFS = require('nhdfs').createFS;
const fs = createFS({service:"localhost", port:9000});
//const fs = createFS({service:"nameservice1"});
//const fs = createFS({service:"nameservice1", configurationPath:'/opt/hadoop/conf/hdfs-site.xml'});
//const fs = createFS({service:"nameservice1", user:"testuser", configurationPath:'/opt/hadoop/conf/hdfs-site.xml'});

const name = "/writertest";

function write() {
    console.log(`Write to ${name}`);
    const out = fs.createWriteStream(name, {replication:1});
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
            console.log(`Done with write to ${name}, sleeping`);
            setTimeout(resolve, 1000);
        })
    });
}

async function truncate() {
    let stat = await fs.stats(name);
    console.log("stat size before");
    console.log(stat.size);
    let res = await fs.truncate(name, 10000);
    console.log(res);
    await timeout(2000);
    console.log("stat size after");
    stat = await fs.stats(name);
    console.log(stat.size);
    console.log("done");
}

write().then( () => {
    return truncate();
}).catch( (err) => {
    console.log(err);
});