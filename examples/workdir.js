'use strict';

const FileSystem = require('nhdfs').FileSystem;
const fs = new FileSystem("namenode", 9000);

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

