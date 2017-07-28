
const FileSystem = require('./lib/nhdfs').FileSystem;

const lfs = require('fs');

const h1 = new FileSystem("lga-grid107.contextweb.prod", 9000);
h1.list(".").then((list) => {
    console.log("list 1");
    list.forEach((element) => {
        console.log(element);
    });
})
.catch( (err) => {
    console.log("err 2");
    console.log(err);
})
.then ( async () => {
    let r = await h1.isDirectory('/user/tabakumov/writertest');
    console.log(`/user/tabakumov/writertest is directory=${r}`);
})
.then ( async () => {
    let r = await h1.isFile('/user/tabakumov/writertest');
    console.log(`/user/tabakumov/writertest is file=${r}`);
})
.then ( async () => {
    let r = await h1.exists('/user/tabakumov/writertest');
    console.log(`/user/tabakumov/writertest exists=${r}`);
})
.then ( async () => {
    let r = await h1.exists('/user/tabakumov/writertes');
    console.log(`/user/tabakumov/writertes exists=${r}`);
})
// .then( () => {
//     console.log("mkdir 3");
//     return h1.mkdir("kk1/yy4/yy2")
// })
// .then( () => {
//     console.log("delete 4");
//     return h1.delete("kk1/yy4")
// })
// .catch( (err) => {
//     console.log("err 5");
//     console.log(err);
// })
// .then( () => {
//     console.log("list 6");
//     return h1.delete("kk1/yy4",true)
// })
// .catch( (err) => {
//     console.log("list 7");
//     console.log(err);
// })
// .then( () => {
//     console.log("list 8");
//     return h1.delete("kk1")
// })
// .then( () => {
//     console.log("list 9");
//     return h1.ls(".")
// })
// .then((list) => {
//     console.log("list 10");
//     list.forEach((element) => {
//         console.log(element.path);
//     });
// })
// .then( () => {
//     console.log("list 11");
//     return h1.stats(".")
// })
// .then((s) => {
//     console.log("list 12");
//     console.log(s);
// })
// .then( () => {
//     console.log("list 13");
//     return h1.exists("rkf")
// })
// .then((s) => {
//     console.log("list 14");
//     console.log(s);
// })
// .then ( () => {
//     const o = h1.createWriteStream("writertest");
//     o.on('error', (err) => {
//         console.log(err);
//     });
//     let i = 6;
//     var buf = Buffer.from(`hello word 1${i} \n`, 'utf8');    
//     o.write(buf);
//     buf = Buffer.from(`hello word 2${i}\n`, 'utf8');
//     o.write(buf);
//     buf = Buffer.from(`hello word 3${i} \n`, 'utf8');
//     o.write(buf);
//     o.end(`end writing 4${i}\n`);
//     return new Promise ( (resolve, reject) => {
//         o.on('finish', () => {
//             console.log("dd");
//             setTimeout(resolve, 1000);
//         })
//     });
// })
// .then ( () => {
//     const o = h1.createReadStream("writertest");
//     o.on('error', (err) => {
//         console.log(err);
//     });
//     o.on('data', (data) => {
//         console.log("rr");
//     })
//     o.on('end', () => {
//         o.close();
//     } )
// })
.catch( (err) => {
    console.log("list 100");
    console.log(err);
});