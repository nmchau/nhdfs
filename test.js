
const FileSystem = require('./lib/nhdfs').FileSystem;

const lfs = require('fs');

const h1 = new FileSystem("localhost", 9000);
h1.ls(".").then((list) => {
    console.log("list 1");
    list.forEach((element) => {
        console.log(element);
    });
})
.catch( (err) => {
    console.log("err 2");
    console.log(err);
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
.catch( (err) => {
    console.log("list 100");
    console.log(err);
});