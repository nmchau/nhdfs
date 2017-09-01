'use strict';

const createFS = require('nhdfs').createFS;
const fs = createFS({service:"namenode", port:9000});
//const fs = createFS({service:"nameservice1"});
//const fs = createFS({service:"nameservice1", configurationPath:'/opt//hadoop/conf/hdfs-site.xml'});
//const fs = createFS({service:"nameservice1", user:"testuser", configurationPath:'/opt/hadoop/conf/hdfs-site.xml'});
//const fs = createFS();

const tmpWorkDir = 'testWorkDir';
var curDir = "";
fs.getWorkingDirectory()
.then( async (workDir) => {
    console.log(`working dir=${workDir}`);
    curDir = workDir;
    await fs.mkdir(tmpWorkDir);
    var i = await fs.exists(tmpWorkDir);
    console.log(`${tmpWorkDir} exists=${i}`);
    i = await fs.isDirectory(tmpWorkDir);
    console.log(`${tmpWorkDir} is directory=${i}`);
    i = await fs.isFile(tmpWorkDir);
    console.log(`${tmpWorkDir} is file=${i}`);
    let fullPath = `${curDir}/${tmpWorkDir}`; //needs to be full path
    console.log(`working dir to set=${fullPath}`);
    await fs.setWorkingDirectory(fullPath);
    let nwd = await fs.getWorkingDirectory();    
    console.log(`working dir=${nwd}`);
    await fs.setWorkingDirectory(curDir);
    nwd = await fs.getWorkingDirectory();    
    console.log(`working dir=${nwd}`);
    await fs.delete(fullPath);
    i = await fs.exists(fullPath);
    console.log(`${fullPath} exists=${i}`);
})
.catch( (err) => {
    console.log(err);
});

