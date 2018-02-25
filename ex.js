#!/usr/bin/nodejs

/*
 *	This script purpose - imitation ESP8266 chip
 *	there emulate answers of chip on request`s from browser
 *  query`s list:
 * 		1) /src?resource_name=resource.file
 *		2) /current
 * 		3) /lastValues?sensor=sensor_name(tIn tOut baro humid)
 * 		4) /availablePeriod
 *
 *   month names "Jan", "Feb", "Mar", "Apr", "May", "Jun", "Jul", "Aug", "Sep", "Oct", "Nov", "Dec"
 */



console.log("Start node app on port 3000 !!!");

var fs=require('fs');
var http=require('http');
var url = require('url');
var path = require('path');
var root = './espSensors/data';



http.createServer(function(req, res){
	console.log('url: ' + req.url);
	var all = url.parse(req.url,true);
	var query = all.query;
	var pathName = all.pathname;
	var extension = path.extname(all.pathname);
	if(pathName == '/')pathName = '/index.htm';
	switch(pathName){
		case '/current':
			sendCurrent(res);  break;
		case '/lastValues':
			sendLast(query,res); break;
		case '/src':
			parseSrc(query, res); break;
		case '/favicon.ico':
				sendFile('image/png','/src/pic/okay.png',res);
		case '/index.htm':
			sendFile('text/html', '/index.htm', res); break;
		case '/availablePeriod':
			sendPeriods(res); break;
		case '/sensorData':
			sendSensorData(query,res); break;	
		default:
		console.log('resource "' + pathName + '" Not Found');
			notFound(res);			
	}
	
}).listen(3000);

function parseSrc(query, res){
	var contentType = 'text/plain';
	var fName = '/src';
	if(query['js']){
		contentType = 'application/javascript'
		fName += '/js/' + query['js'];
	}
	else if(query['css']){
		contentType = 'text/css';
		fName += '/css/' + query['css'];
	}	
	else if(query['pic']){
		contentType = 'image/svg+xml';
		fName += '/pic/' + query['pic'];
	}else{
		console.log('not found for query: ' + JSON.stringify(query) );
		notFound(res);
		return;
	}
	sendFile(contentType, fName, res);
}

function sendCurrent(res){
	res.writeHead(200,{'Content-Type':'application/json'});
	res.end('{"tIn": 20, "tOut": -10, "baro": 756, "humid":40}');
}

function sendLast(query, resp){
	var lIn = '{ "tIn":[ 0,0,0,22,22,22,22,22,22,23,24,23,22], "last":3 }';
	var lOut = '{"tOut":[0,0,0,-2,-2,-3,-3,-4,-5,-5,-3,-1, -1], "last":3 }';
	var lBaro = '{"baro":[0,0,0,0,650,745,746,744,740,740,744,746,747], "last":12 }';
	var lHumid = '{"humid":[-1,20,20,20,21,20,20,20,21,22,22,23,22], "last":12}';
	var res = query['sensor'];
	if(res == 'tIn')res = lIn;
	else if(res == 'tOut')res = lOut;
	else if(res == 'baro')res = lBaro;
	else if(res == 'humid')res = lHumid;
	else res = '{"ERROR":["not have sensor for:' + JSON.stringify(query) + '",""],"last":0}';
	resp.writeHead(200,{'Content-Type':'application/json'});
	resp.end(res);
}

function sendFile(contentType, fName, res){
	fName = root + fName;
	console.log('try send file: ' + fName);
	fs.readFile(fName, 'utf8', function(err, data){
		if(err){
			console.log('ERROR on load file: ' + fName + " error=" + err);
			res.writeHead(500,{'Content-Type':'text/plain'});
			res.end('can`t read file: '+ fName);
		}
		else {
			res.writeHead(200,{'Content-Type':contentType});
			res.end(data);
		}
	});
}

function sendPeriods(res){
	var periods = '["/data/2018/Jan.txt", "/data/2018/Feb.txt"  , "/data/2018/Mar.txt" ' +
	', "/data/2018/Jun.txt",  "/data/2018/Jul.txt", ' +
			' "/data/2019/Oct.txt", "/data/2019/Nov.txt"  ]';
    var curt = '[ "/data/2018/Feb.txt" ]';
	res.writeHead(200,{'Content-Type':'application/json'});
	res.end(curt);
}

function sendSensorData(query, res){
    //  period format: yyyy/month.txt
    var answer = '';
    var period = query['period'];
    if(period.length < 1)answer = 'Can`t get query from request \"sensorData\"';
    else answer = getPeriodData(period);
	res.writeHead(200, {'Content-Type':'text/plain'});
	res.end(answer);
}


function notFound(res){
	res.writeHead(404,{'Content-Type':'text/html'});
			res.end('<html><body><h1 align="center">Resource not found</h1></body></html>');
}

function getPeriodData(period){
    //day hour tIn tOut pressure humidites
    var periods = {
        '2018/Jan.txt' :
            ' 01 01 2 -2 777 10\n' +
            ' 01 02 2 -2 773 10\n' +
            ' 01 03 2 -2 774 10\n' +
            ' 01 04 2 -2 776 10\n' +
            ' 01 05 2 -2 775 10\n' +
            ' 01 06 2 -2 772 10\n' +
            ' 01 07 2 -2 777 10\n',
        '2018/Feb.txt' :
            ' 02 01 2 -2 777 10\n' +
            ' 02 02 2 -2 727 10\n' +
            ' 02 03 2 -2 737 10\n' +
            ' 02 04 2 -2 747 10\n' +
            ' 02 05 2 -2 757 10\n' +
            ' 02 06 2 -2 767 10\n' +
            ' 02 07 2 -2 727 10\n',
        '2018/Mar.txt' :
            ' 02 01 2 -21 777 10\n' +
            ' 02 02 2 -21 777 10\n' +
            ' 02 03 2 -22 777 10\n' +
            ' 02 04 2 -23 777 10\n' +
            ' 02 05 2 -24 777 10\n' +
            ' 02 06 2 -25 777 10\n' +
            ' 02 07 2 -26 777 10\n',
        '2018/Jun.txt' :
            ' 03 01 22 -2 777 10\n' +
            ' 03 02 23 -2 777 10\n' +
            ' 03 03 24 -2 777 10\n' +
            ' 03 04 25 -2 777 10\n' +
            ' 03 05 26 -2 777 10\n' +
            ' 03 06 27 -2 777 10\n' +
            ' 03 07 28 -2 777 10\n',
        '2018/Jul.txt' :
            ' 04 01 2 -2 777 12\n' +
            ' 04 02 2 -2 777 14\n' +
            ' 04 03 2 -2 777 14\n' +
            ' 04 04 2 -2 777 14\n' +
            ' 04 05 2 -2 777 16\n' +
            ' 04 06 2 -2 777 16\n' +
            ' 04 07 2 -2 777 17\n',
        '2019/Oct.txt' :
            ' 05 11 2 -2 777 10\n' +
            ' 05 12 2 -2 777 10\n' +
            ' 05 13 2 -2 777 10\n' +
            ' 05 14 2 -2 777 10\n' +
            ' 05 15 2 -2 777 10\n' +
            ' 05 16 2 -2 777 10\n' +
            ' 05 17 2 -2 777 10\n',
        '2019/Nov.txt' :
            ' 16 01 2 -2 777 10\n' +
            ' 16 02 2 -2 777 10\n' +
            ' 16 03 2 -2 777 10\n' +
            ' 16 04 2 -2 777 10\n' +
            ' 16 05 2 -2 777 10\n' +
            ' 16 06 2 -2 777 10\n' +
            ' 16 07 2 -2 777 10\n'
    }
    if(periods.hasOwnProperty(period))return periods[period];
    console.log("Bad request for \"sensorData\"");
    return 'ERROR: bad request \"' + period + '\"';
}
