const net = require('net');
const app = require('express')();
const util = require('util');
const fs = require('fs');
const qt = require('qtdatastream');
const http = require('http');
const imageToBase64 = require('image-to-base64');
const {SocketClientTCP} = require('netlinkwrapper');
require('events').EventEmitter.defaultMaxListeners = 100;

const port = 8081;
const portA = 8083;


let client = new SocketClientTCP(portA, 'localhost');

/*-------------IPC Config----------------*/

/*-------------Server - Client communication-------------*/
const server = http.createServer(function (req, res) {
    res.statusCode = 200;
    res.setHeader('Content-Type', 'text/plain');
    res.end('Hello world!\n');
});


server.listen(port, function() {
    console.log('Server running at port: ', port);
})

const WebSocketServer = require('node-qwebsocket');
const { exec } = require('child_process');
const { publicDecrypt } = require('crypto');
const socketServer = new WebSocketServer(server);

//function to encode file data to base64 encoded string
function base64_encode(file){
    //read binary data
    let bitmap = fs.readFileSync(file);
    //convert binary data to base64 encoded string
    let buffer = Buffer.from(bitmap);
    return buffer.toString('base64');
}

let server_directory = "C:/Users/CGLAB/Desktop/chaen/";

let count = 0;
let data_from_mgr;
socketServer.on('connection', function (socket){
    count++;
    console.log('currently connected with : ', count, 'users');
    
    socket.on('message', function (data) {
        try{
            
            console.log("message received: %s", JSON.stringify(data, null, 2));
            data_from_viewer = JSON.parse(data);

            current_server_directory = server_directory + data_from_viewer['UserID'] + '/';
            
            let test_json = {
                "UserId" : data_from_viewer['UserID'],
                "RenderingType" : data_from_viewer['RenderingType'], 
                "Action" : data_from_viewer['Action'],
                "Directory" : current_server_directory, 
                "ImageType" : data_from_viewer["ImageType"], 
                "Param" : data_from_viewer["Params"]
            }
        
            let message = JSON.stringify(test_json);
            
            client.send(message);
            console.log("message sent");

            let data_received = client.receive();
            
            /*-------------------- Node - JS SERVER IPC COMMUNICATION---------------------- */
               
            console.log(" From Server: " + data_received.toString());     
            //TODO: Change so that the image_dir is based on what the client chooses
            let axial_dir = current_server_directory + '/axial.jpeg';    //data_from_mgr[]
            let sagittal_dir = current_server_directory + '/sagittal.jpeg'
            let coronal_dir = current_server_directory + '/coronal.jpeg'
            let axial_json = JSON.parse(fs.readFileSync(current_server_directory + '/axial.json'));
            let sagittal_json = JSON.parse(fs.readFileSync(current_server_directory + '/sagittal.json'));
            let coronal_json = JSON.parse(fs.readFileSync( current_server_directory + '/coronal.json'));
            let axial_data = base64_encode(axial_dir);
            let sagittal_data = base64_encode(sagittal_dir);
            let coronal_data = base64_encode(coronal_dir);


            let final_packet = {
                axial: {
                    img: axial_data, 
                    features: axial_json
                }, 
                sagittal: {
                    img: sagittal_data, 
                    features: sagittal_json
                },
                coronal: {
                    img: coronal_data, 
                    features: coronal_json
                }
            }

            socket.emit('response', final_packet);
            console.log("emitted to viewer");       

        
            /*----------------------------------------------------------------------------------------------------------- */

            
        }
        catch(exception){
            console.log(exception);
        }
        
    });

    
    socket.on('close', function (data) {
        count--;
        console.log('currently connected with : ', count, 'users');
        
    });

});


/*
let server = net.createServer(function (socket){

    console.log(socket.address().address + " is connected.");

    socket.write("Hey, client!");

    socket.on('data', function(data){
        console.log('received: ' + data);
        
        filePath = 'C:/Users/CGLAB/Desktop/serverTest/test_images/axial.jpeg';
        let stream = fs.createReadStream(filePath);
        //const imageByteArray = qt.types.QByteArray.from(stream);
        //qt.socket.write(imageByteArray);
    })
    
    socket.on('close', function(){
        console.log('client disconnected');
    })
    
    socket.write('welcome to the server');

});

*/

//let server = net.createServer();




