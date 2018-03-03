'use strict';

const chai = require('chai');
const assert = chai.assert;
const expect = chai.expect;
const createFS = require('../lib/nhdfs').createFS;
const timeout = require("./testutil").timeout

const userName = require("os").userInfo().username;
const fs = createFS({service:"localhost", port:9000});

async function del() {
    let list = await fs.list("/");
    list.forEach( async (l) => {
        await fs.delete(l.path, true);
    });
    await timeout(1000);
}

function write(name) {
    const out = fs.createWriteStream(name, {replication:1});
    out.on('error', (err) => {
        console.log(err);
    });
    out.end("1");
    return new Promise((resolve, reject) => {
        out.on('finish', () => {
            setTimeout(resolve, 1000);
        })
    });
}

async function createDir(path) {
    await fs.mkdir(path)
    await fs.chmod(path, 777);
}

async function createFile(path) {
    await write(path);
    await fs.chmod(path, 777);
}

async function checkFile(f, path) {
    const blockSize = await fs.getDefaultBlockSize();
    assert.equal(f.type, 'file', 'type == file');
    assert.equal(f.path, path, `path == ${path}`);
    assert.equal(f.size, 1);
    assert.equal(f.block_size, blockSize);
    assert.equal(f.owner, 'root');
    assert.equal(f.group, 'supergroup');
    assert.equal(f.permissions, 777, 'permissions == 777');
    assert(f.last_mod > 0);
    assert(f.last_access > 0);
}

function checkDir(d, path) {
    assert.equal(d.type, 'directory', 'type == directory');
    assert.equal(d.path, path, `path == ${path}`);
    assert.equal(d.size, 0);
    assert.equal(d.block_size, 0);
    assert.equal(d.owner, 'root');
    assert.equal(d.group, 'supergroup');
    assert.equal(d.permissions, 777, 'permissions == 777');
    assert(d.last_mod > 0);
    assert(d.last_access >= 0);
}

describe('list test', () => {

    beforeEach( async ()  => {
        await del();
    });

    afterEach( async ()  => {
        await del();
    });

    it('should throw Error if dir/file does not exist', async () => {
        let errMsg = ''; //TODO: throws does not catch?
        try {
            await fs.list('/notexists')
        } catch (err) {
            errMsg = err.message;
        }
        assert(errMsg.includes('FileNotFoundException'), 'dir/file does not exist');
    });

    it(`should return empty dir`, async () => {
        const list = await fs.list('/')
        assert.equal(list.length, 0, "/ must be empty");
    });

    it(`should return 1 dir item`, async () => {
        const dname = '/listdir';
        await createDir(dname);
        const list = await fs.list('/');
        assert.equal(list.length, 1, '/ must contain 1 item');
        checkDir(list[0], dname);
    });

    it(`should return empty subdir`, async () => {
        const dname = '/listdir';
        await createDir(dname);
        const list = await fs.list(dname);
        assert.equal(list.length, 0, `${dname} must contain 0 items`);
    });

    it(`should return 1 file item`, async () => {
        const fileName = '/listfile';
        await createFile(fileName);
        const list = await fs.list('/');
        assert.equal(list.length, 1, '/ must contain 1 item');
        await checkFile(list[0], fileName);
    });

    it(`should return 2 items at root`, async () => {
        const fileName = '/listfile';
        const dname = '/listdir';
        await createDir(dname);
        await createFile(fileName);
        const list = await fs.list('/');
        assert.equal(list.length, 2, '/ must contain 2 item2');
        const [f,d] = ( list[0].type == 'file' ) ? list : [list[1], list[0]];
        await checkFile(f, fileName);
        checkDir(d, dname);
    });

    it(`should return 2 items at dir`, async () => {
        const dir = '/listdir';
        const fileName = `${dir}/lislistfile`;
        const dname = `${dir}/listsubdir`;
        await createDir(dname);
        await createFile(fileName);
        const list = await fs.list(dir);
        assert.equal(list.length, 2, `${dir} must contain 2 item2`);
        const [f,d] = ( list[0].type == 'file' ) ? list : [list[1], list[0]];
        await checkFile(f, fileName);
        checkDir(d, dname);
    });

    it(`should return 2 items at work dir`, async () => {
        const dir = '/listdir';
        const fileName = `${dir}/lislistfile`;
        const dname = `${dir}/listsubdir`;
        await createDir(dname);
        await createFile(fileName);
        await fs.setWorkingDirectory(dir);
        const list = await fs.list('.');
        assert.equal(list.length, 2, `${dir} must contain 2 item2`);
        const [f,d] = ( list[0].type == 'file' ) ? list : [list[1], list[0]];
        await checkFile(f, fileName);
        checkDir(d, dname);
    });
});
