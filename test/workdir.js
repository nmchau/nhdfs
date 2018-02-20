'use strict';

const chai = require('chai');
const assert = chai.assert;
const expect = chai.expect;
const createFS = require('../lib/nhdfs').createFS;

const userName = require("os").userInfo().username;
const fs = createFS({service:"localhost", port:9000});

describe('Working dir operations', () => {

    it(`should return /user/${userName}`, async () => {
        const curDir = await fs.getWorkingDirectory();
        assert.equal(curDir, `/user/${userName}`, "current user working dir must be home dir");
    });

    it(`should set current work dir to /user/${userName}/testWorkDir`, async () => {
        const curDir = await fs.getWorkingDirectory();
        const tmpWorkDir = `${curDir}/testWorkDir`;
        await fs.mkdir(tmpWorkDir);
        let i = await fs.exists(tmpWorkDir);
        console.log(`${tmpWorkDir} exists=${i}`);
        i = await fs.isDirectory(tmpWorkDir);
        console.log(`${tmpWorkDir} is directory=${i}`);
        await fs.setWorkingDirectory(tmpWorkDir);
        let nwd = await fs.getWorkingDirectory();  
        assert.equal(nwd, tmpWorkDir, `current user working dir must be ${tmpWorkDir}`);
        await fs.setWorkingDirectory(curDir);
        nwd = await fs.getWorkingDirectory();  
        assert.equal(nwd, curDir, `current user working dir must be ${curDir}`);  
        await fs.delete(tmpWorkDir);
    });
});
