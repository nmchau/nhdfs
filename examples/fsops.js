'use strict';

const createFS = require('nhdfs').createFS;
const fs = createFS({service:"namenode", port:9000});
//const fs = createFS({service:"nameservice1"});
//const fs = createFS({service:"nameservice1", configurationPath:'/opt//hadoop/conf/hdfs-site.xml'});
//const fs = createFS({service:"nameservice1", user:"testuser", configurationPath:'/opt/hadoop/conf/hdfs-site.xml'});
//const fs = createFS();

fs.list(".").then((list) => {
    list.forEach((element) => {
        console.log(element);
    });
})
.catch( (err) => {
    console.log(err);
})
.then ( async () => {
    let r = await fs.isDirectory('dir1');
    console.log(`dir1=${r}`);
})
.then ( async () => {
    let r = await fs.isFile('file1');
    console.log(`file1=${r}`);
})
.then ( async () => {
    let r = await fs.exists('file1');
    console.log(`file1=${r}`);
})
.then ( async () => {
    let r = await fs.exists('filexxx');
    console.log(`filexxx=${r}`);
})
.then ( async () => {
    let capacity = await fs.getCapacity();
    console.log(`capacity: ${capacity}`);
    let used = await fs.getUsed();
    console.log(`used: ${used}`);
})
.catch( (err) => {
    console.log(err);
});