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
var sp_arduino = new SerialPort.SerialPort("/dev/ttyUSB0" );


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
  console.log('open');
  requestRSSI();
  setInterval(requestRSSI, sampleDelay);
});
// Recieve RSSI 
XBeeAPI.on("frame_object", function(frame) {
  if (frame.type == 144){
    console.log("Beacon ID: " + frame.data[1] + ", RSSI: " + (frame.data[0]));
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
});

io.on('connection', function(socket){
    sp_arduino.on("open", function () {    
        if(count == 4){ 
            var valid = true, 
                lastType = 0;
            for(var i = 0; i < 4; i ++){ 
              if(rssi[i] == 0||rssi[i] == undefined||rssi[i] == 255) valid = false; 
            }
            try{
                if(valid){            
                    var result = knn_cluster.launch(5, new kNN.Node({paramA: rssi[0], paramB: rssi[1], paramC: rssi[2], paramD: rssi[3], type: false}));
                    console.log(result.type);
                    socket.emit('knn_result',result.type+"");
                    
                    if(lastType != result.type){
                        if(result.type == 1 || result.type == 3 || result.type == 5 || reuslt.type == 7){
                            sp_arduino.write('T');
                        }else{
                            sp_arduino.write('K');
                        }
                        lastType = result.type;
                    }
                }else{
                    console.log("error");
                    socket.emit('error',"error");
                }   
            }catch(error){
                console.log(error);
            }            
            count = 0; 
        }        
        socket.on('auto_drive', function(message2){ 
            sp_arduino.write('A');
        });
        socket.on('remote_control', function(message3){ 
            sp_arduino.write('C');
        });
        socket.on('control', function(message1){ 
            sp_arduino.write('S' + message1);
        });
        socket.on('start', function(message4){ 
            sp_arduino.write('C' + '90,180');
        });
        socket.on('end', function(message5){ 
            sp_arduino.write('C' + '90,180');
        });
    });
    sp_arduino.on( "data", function(msg) {
        console.log(String.fromCharCode(msg));
    });
}); 


app.get('/', function(req, res){
  res.sendfile('page.html');
});
app.get('/joystick', function(req, res){
  res.sendfile('try_joyStick.html');
});
app.use('/static',express.static(__dirname));

http.listen(3000, function(){
  console.log('listening on *:3000');
});