#ifndef props_h
#define props_h

//   ------------  SERVER NOTES  -------------------

/*  file system
 *  /
 *	--index.htm
 *  --log.log
 *  --/data
 *		-----/2018
 * 				--month1.txt
 * 				--mmonth2.txt
 * 				--month3.txt
 * 					...
 * 				--summary.txt
 *		-----/2019
 * 				-- ...
 * 		-----/2020
 * 				-- ...
 * 		----- ...
 * 				-- ...
 *	--/src
 * 		-----/js
 * 				--script1.js
 * 				-- ...
 * 		-----/pic
 * 				--img1.svg
 * 				-- ...
 *		-----/css
 * 				--style1.css
 * 				-- ...
 *	file format:
 * ________________________________________
 * 		for	month_name.txt
 * 
 * 	day hour tIn tOut pressure humidites
 * 	day2 hour2 tIn2 tOut2 pressure2 humidites2
 * 				...

 * _________________________________________
 * 		for summary.txt
 *  month tIn tOut pressure humidites
 * 			...
 *	(whitespace in line for files replaced on value "#defined SEP ... " in this file )
 *
 * 
 */

/* JSON  answer formats for   browser 
 *        query           |       answer
 *        
 *  1) /current           | { "tIn": val, "tOut" : val, "baro" : val, "humid" : val }
 *  2) /lastValues        | { "data_type" : [ "val1", "val2", .... ], "last" : jastGottenNumFromSensor }
 *  3) /availablePeriod   | [ "/data/year/month1.txt", "...month2.txt", "...", ... ]
 *  
 */


 /*  request with parameters looks
  * 1) /lastValues?sensor=s_name  ( s_name may be: tIn, tOut, baro, humid )
  * 		return result as JSON
  * 2) /readFile?fName=file_name   ( file_name is full file name e.g /some/dir/file.ext )
  * 		return file content as byte stream
  * 3) /src?res_type=fName
  * 			res_type may be: js, pic, css ( link on directory /src/js , /src/css, /src/pic )
  * 			fName - name of resource file with extensions e.g myScript.js
  * 		return resource as file content
  */


//
// ==================== SENSORS ===========================
//
//  pin 4 and 5 for BMP280 sensor( 4 - SCL ; 5 - sda )
//  pin 2 - one wire (DHT 11 and DS18B20 sensors)
#define ONE_WIRE_BUS 2  // one wire
#define DHT_TYPE DHT11   // may be DHT11 or  DHT22

//      sensors data Types
#define T_IN "tIn"
#define T_OUT "tOut"
#define BARO "baro"
#define HUMID "humid"

//  cycle requests 
#define REQUEST_COUNT 60

//
// ================= storage ===================
//
//#define CS_PIN 16
//  SPIFFS
#define SENSOR_DATA_DIR   "data"
#define TEST_YEAR_DIR "2019"
#define SEP " "
// report to SPIFFS
#define LOG_FILE "log.log"


//
// =================   WI-FI ====================

enum wifiMODE { DEVICE_AP_MODE, DEVICE_STA_MODE, DEVICE_NOT_WIFI};

//  WIFI_mode WEB_SERVER
//  STA
#define STA_SSID  "mywifi"
#define STA_PASSWD "bezwolos"
//  AP
#define AP_IP_ADDR  192,168,10,1
#define AP_IP_SUBNET  255,255,255,0
#define AP_SSID "espWeather"
#define AP_PASSWD  "esp8266srv"

//
//  ==================  SERVER  ==================
//
#define HOST "weather"
#define PORT 80
#define SERVER_ROOT  "/serv/"
#define STA_PARAMS_FILE "/server/staParams.txt"
#define REQUEST_DATA_URL "www.yandex.ru"
#define  NOT_FOUND "NotFound.htm"

//
// ====================  semaphore ==================
//

#define CYCLE_DURATION 60
#define SENSORS_REQUEST_PERIOD 5





#endif

