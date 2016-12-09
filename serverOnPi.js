var SerialPort = require("serialport"),
    app = require('express')(),
    xbee_api = require('xbee-api'),
    express = require('express'),
    http = require('http').Server(app),
    io = require('socket.io')(http),
    fs = require('fs'),
    kNN = require("k.n.n"),
    traindata = new Array(),
    knn_cluster;
//var socket = io();
var count = 0,
    C = xbee_api.constants,
    sampleDelay = 3000;
var XBeeAPI = new xbee_api.XBeeAPI({
      api_mode: 2
    });
//var portName = process.argv[2];
var sp;
portConfig = {
      baudRate: 9600,
      parser: XBeeAPI.rawParser()
    };
sp = new SerialPort.SerialPort("/dev/ttyUSB1", portConfig);
var sp_arduino = new SerialPort.SerialPort("/dev/ttyUSB0");


// KNN, training with the dataset in dataset_update.txt file
var lineReader = require('readline').createInterface({
  input: require('fs').createReadStream('data/dataset.txt')
});
lineReader.on('line', function (line) {
    var lll = line.split("/")[0]+"";
    var group = line.split("/")[1]+"";
    var pos = lll.split(",");
    var train_node = new kNN.Node({paramA: parseInt(pos[0]), paramB: parseInt(pos[1]), paramC: parseInt(pos[2]), paramD: parseInt(pos[3]), type: group});
    traindata.push(train_node);
});
knn_cluster = new kNN(traindata);
var rssi = [];

var RSSIRequestPacket = {
  type: C.FRAME_TYPE.ZIGBEE_TRANSMIT_REQUEST,
  destination64: "000000000000ffff",
  broadcastRadius: 0x01,
  options: 0x00,
  data: "test"
}
var requestRSSI = function(){
  sp.write(XBeeAPI.buildFrame(RSSIRequestPacket));
}
sp.on("open", function () {
  console.log('Xbee open');
  requestRSSI();
  setInterval(requestRSSI, sampleDelay);
});
// Recieve RSSI 
var lastType = 0;
XBeeAPI.on("frame_object", function(frame) {
    if (frame.type == 144){
    console.log("Beacon ID: " + frame.data[1] + ", RSSI: " + (frame.data[0]) + "  count : " + count);
    count++;
      switch(frame.data[1]){
              case 1:
                  rssi[0] = frame.data[0];
                  break;
              case 2:
                  rssi[1]  = frame.data[0];
                  break;
              case 3:
                  rssi[2]  = frame.data[0];
                  break;
               case 4:
                   rssi[3]  = frame.data[0];
                   break;
          }
    }
    if(count == 4){ 
        var valid = true; 
            //lastType = 0;
        for(var i = 0; i < 4; i ++){ 
          if(rssi[i] == 0||rssi[i] == undefined||rssi[i] == 255) valid = false; 
        }
        if(valid){            
            var result = knn_cluster.launch(3, new kNN.Node({paramA: rssi[0], paramB: rssi[1], paramC: rssi[2], paramD: rssi[3], type: false}));
            console.log("current location is : " + result.type);
            if(lastType != result.type){
                io.emit('knn_result',result.type);
                /*try{
                    if(result.type != 6){
                        sp_arduino.write('T');
                    }else{
                        sp_arduino.write('K');
                    }
                    
                }catch(error){ console.log("error : " + error);}    
                */
                lastType = result.type;          
            }
        }  
        count = 0; 
    }        
});

io.on('connection', function(socket){
    var status = 0;
    socket.on('auto_drive', function(message2){
        if(status != 1){
            sp_arduino.write('A;');
            console.log('tell car auto_drive');
        }
        status = 1;            
    });
    socket.on('remote_control', function(message3){ 
        if(status != 2){
            console.log('tell car remote_control');
            sp_arduino.write('C;');
        }
        status = 2;
    });
    socket.on('control', function(message1){ 
        status = 3;
        sp_arduino.write('S' + message1+';');
        console.log('tell car control ' + message1);
    });
    socket.on('start', function(message4){ 
        status = 4;
        console.log('tell car start');
        sp_arduino.write('S0,90;');
    });
    socket.on('stop', function(message5){ 
        status = 5;
        console.log('tell car stop');
        sp_arduino.write('S0,90;');
    });
});
//sp_arduino.on('open', function(){
  //console.log('arduino open');
  sp_arduino.on('data', function(msg){
      console.log("back from arduino : " + msg.toString('utf8'));
  });
//});

app.get('/', function(req, res){
  res.sendfile('page_knn.html');
});
app.get('/joystick', function(req, res){
  res.sendfile('try_joyStick.html');
});
app.use('/static',express.static(__dirname));

http.listen(3000, function(){
  console.log('listening on *:3000');
});