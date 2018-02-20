'use strict';

const chai = require('chai');
const assert = chai.assert;
const expect = chai.expect;
const timeout = require("./testutil").timeout

const createFS = require('../lib/nhdfs').createFS;
const fs = createFS({service:"localhost", port:9000});
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
            setTimeout(resolve, 10000);
        })
    });
}

describe('Truncate test', () => {

    it(`should truncate file ${name} to `, async () => {        
        await write();
        let stat = await fs.stats(name);
        console.log(`stat size before: ${stat.size}`);
        let res = await fs.truncate(name, 10000);
        await timeout(5000);
        stat = await fs.stats(name);
        console.log(`stat size after: ${stat.size}`);
        assert.equal(stat.size, 10000, "file size should be 10000");
    });
});
