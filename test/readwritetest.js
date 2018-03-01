'use strict';

const chai = require('chai');
const assert = chai.assert;
const expect = chai.expect;
const timeout = require("./testutil").timeout

const createFS = require('../lib/nhdfs').createFS;
const fs = createFS({service:"localhost", port:9000});
const name = "/readwritetest";

function write() {
    console.log(`Write to ${name}`);
    const out = fs.createWriteStream(name, {replication:1});
    out.on('error', (err) => {
        console.log(err);
    });
    for (let i = 0; i <= 1000; ++i ) {
        var buf = Buffer.from(`{"name": "line ${i}", "msg": "hello world number ${i}", "num": ${i}}\n`, 'utf8');
        out.write(buf);
    }
    out.end(`{"name": "last line 1001", "msg": "later on world number 1001", "num": 1001}`);
    return new Promise((resolve, reject) => {
        out.on('finish', () => {
            console.log(`Done with write to ${name}, sleeping`);
            setTimeout(resolve, 10000);
        })
    });
}

async function read() {
    console.log("starting reading");
    const ins = fs.createReadStream(name);
    let str = "";
    return new Promise((resolve, reject) => {
        ins.on('error', (err) => {
            console.log(err);
            reject(err);
        });
        ins.on('data', (data) => {
            let s = data.toString('utf8');        
            str = str + s;
        })
        ins.on('end', () => {
            ins.close();
            resolve(str);
        })
    });
}

describe('Write Read Test', () => {

    it(`should write file ${name} to `, async () => {        
        await write();
        let stat = await fs.stats(name);
        console.log(`stat size: ${stat.size}`);
        let str = await read();
        let arr = str.split("\n");
        assert.equal(arr.length, 1002, "file size should be 1002 lines");
        let i = 0;
        arr.forEach(l => {
            const line = JSON.parse(l);
            assert.equal(line.num, i, `num == ${i}`);
            if ( i < 1001 ) {
                assert.equal(line.name, `line ${i}`, `name == 'line ${i}'`);
                assert.equal(line.msg, `hello world number ${i}`, `msg == 'hello world number ${i}'`);
            } else {
                assert.equal(line.name, `last line ${i}`, `name == 'last line ${i}'`);
                assert.equal(line.msg, `later on world number ${i}`, `msg == 'later on number ${i}'`);
            }
            i += 1;
        });
    });
});
