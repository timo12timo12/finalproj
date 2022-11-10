var net = require('net');
const fs = require("fs");

fs.readFile("data.txt", "utf8", (err, data) => {
  if (err) {
    console.error(err);
  } else {
    console.log(data);


var client = net.connect({port: 59244, host:'3.37.210.15'},function(){
  //net모듈의 소켓 객체를 사용    
  console.log('\rClient connected');    
  client.write(data+'\r\n');
});


client.on('data',function(data){
  //data 이벤트 발생시 callback
  console.log('\r');   
  console.log(data.toString());    
  client.end();
  });

client.on('end',function(){//end 이벤트 발생시 callback    
  console.log('\rClient disconnected');
});
  }
});
