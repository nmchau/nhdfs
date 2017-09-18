'use strict';

const createClusterInfo = require('nhdfs').createClusterInfo;
//const ci = createClusterInfo(); //LIBHDFS3_CONF env var must be defined, or HADOOP_CONF_DIR/HDFS_SITE
const ci = createClusterInfo({configurationPath:'/opt/hadoop/conf/hdfs-site.xml'});

ci.namenodes("nameservice1").then((list) => {
    list.forEach((element) => {
        console.log(element);
    });
})
.catch( (err) => {
    console.log(err);
})
