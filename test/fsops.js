'use strict';

const chai = require('chai');
const assert = chai.assert;
const expect = chai.expect;
const createFS = require('../lib/nhdfs').createFS;

describe('HDFS operations', () => {
    const fs = createFS({service:"localhost", port:9000});
    describe('Fs info', () => {
        it('should return fs info', async () => {
            let dbs = await fs.getDefaultBlockSize();
            assert(dbs > 0, 'default block size must be  > 0'); //TODO: set docker image
            let capacity = await fs.getCapacity();
            assert(capacity > 0, 'hdfs capacity must be  > 0'); //TODO: set docker image
            let used = await fs.getUsed();
            assert(used > 0, 'used must be  > 0');
        });
    });
    describe('directories', () => {
        it('should create and remove dir ', async () => {
            let d = '/dir1';
            let r = await fs.exists(d);
            assert.isNotOk(r, `${d} does not exist`);
            await fs.mkdir(d);
            r = await fs.exists(d);
            assert.isOk(r, `${d} exists`);
            r = await fs.isFile(d);
            assert.isNotOk(r, `${d} is not a file`);
            r = await fs.isDirectory(d);
            assert.isOk(r, `${d} is a directory`);
            await fs.delete(d);
            r = await fs.exists(d);
            assert.isNotOk(r, `${d} does not exist`);
        });
        it('should recursive create and remove dir ', async () => {
            let d = '/dir2/subdir2';
            let pd = '/dir2';
            let r = await fs.exists(d);
            assert.isNotOk(r, `${d} does not exist`);
            await fs.mkdir(d);
            r = await fs.exists(d);
            assert.isOk(r, `${d} exists`);
            r = await fs.isFile(d);
            assert.isNotOk(r, `${d} is not a file`);
            r = await fs.isDirectory(d);
            assert.isOk(r, `${d} is a directory`);
            let errMsg = ''; //TODO: throws does not catch?
            try {
                await fs.delete(pd)
            } catch (err) {
                errMsg = err.message;
            }
            assert(errMsg.includes('Directory is not empty'), 'Cannot delete recursivelly');
            await fs.delete(pd, true);
            r = await fs.exists(d);
            assert.isNotOk(r, `${d} does not exist`);
        });
    });
  });
