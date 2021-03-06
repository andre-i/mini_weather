#ifndef props_h
#define props_h

//   -----------------------------------------------
//   ------------  SERVER NOTES  -------------------
//   -----------------------------------------------

/*  =========  the structure of file system    ==============
 *  /
 *  --props.txt  // set current wi-fi parameters, debug mode
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
 * 	day  hour  tIn  tOut  pressure  humidites
 * 	day2 hour2 tIn2 tOut2 pressure2 humidites2
 * 				...
 * _________________________________________
 * 			...
 *  the separator for values in month files defined here in SEP property from 'STORAGE' section
 *	(default - whitespace)
 *
 * 
 * _________________________________________
 *      props.txt
 * (contains start parameters of wi-fi for two cases STA and AP mode)
 * # first phund - comment
 * ## ONLY_STA
 * os false
 * ## STA
 * ss ssid 
 * sp password
 * ## AP 
 * as ssid 
 * ap password
 * ## AP IP may be walid IP 4bit address
 * ip 192.168.0.1
 * ## debug mode
 * dm true
 * ## thingspeak write api key
 * ts OLALALA
 * 
 * 
 * where:     
 *    if ONLY_STA true - chip try work only as STA 
 *          any other walue - chip try work as STA,if not successed -  try as AP
 *    ssid - string with lan name,  
 *    password - string with password      
 *    if debug mode true - debug on
 *          any other value - debug off
 *    empty values - permitted
 */

/* ==================== JSON  answer formats for   browser =========================
 *        query           |       answer
 *        
 *  1) /current           | { "tIn": val, "tOut" : val, "baro" : val, "humid" : val }
 *  2) /lastValues        | { "data_type" : [ "val1", "val2", .... ], "last" : lastGottenNumFromSensor }
 *  3) /availablePeriod   | [ "/data/year/month1.txt", "...month2.txt", "...", ... ]
 * 
 *  
 */


 /* ====================== request with parameters looks =============================
  * 1) /lastValues?sensor=s_name  ( s_name may be: tIn, tOut, baro, humid )
  * 		return result as JSON
  * 2) /readFile?fName=file_name   ( file_name is full file name e.g /some/dir/file.ext )
  * 		return file content as byte stream
  * 3) /src?res_type=fName
  * 			res_type may be: js, pic, css ( link on directory /src/js , /src/css, /src/pic )
  * 			fName - name of resource file with extensions e.g myScript.js
  * 		return resource as file content
  * 4) /log.log - return content Log file(contains some startup parameters)
  * 5) /getProps - return current properties
  * 6) /help - return small help page on russian language
  */





//   ------------------------------------------------------------
//                parameters
//   ------------------------------------------------------------

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

// if the themperature values for some sensor is wrong may
//  be set some shift for it after check with other sensor(I take DS18B20)
#define DHT_THEMP_SHIFT  0
#define BMP_THEMP_SHIFT  -1

//  cycle requests 
#define REQUEST_COUNT 13

//
// ================= STORAGE ===================
//

//  SPIFFS
#define SENSOR_DATA_DIR   "/data"
#define SEP " "   // divide sensors data by write to file in one string. see top 
// report to SPIFFS
#define LOG_FILE "/log.log"

// ================  properties file ==============
#define PROPS_FILE "/props.txt"
// starts symbols for wifi params
#define ONLY_STA "os"  // default false
// if chip work in onlySta need set AP network 
// interface address for check connect on suspend
#define AP_NETWORK_ADDRESS "an"
//  sta parameters
#define STA_SSID "ss"
#define STA_PASSWD "sp"
//  ap parameters
#define AP_SSID "as"
#define AP_PASSWD "ap"
#define AP_IP "ip"
//mode of debug output to serial if true - debug ON, otherwise debug OFF
#define DEBUG_MODE "dm"  // default - false
// thingspeak write key
#define THINGSPEAK_KEY "ts"
// DS18B20 included whether? If 'true' - sensor inwork, otherwise - not work
#define DS18B20_MODE  "ds"  // default false


//
// =================   WI-FI ====================

enum wifiMODE { DEVICE_AP_MODE, DEVICE_STA_MODE, DEVICE_NOT_WIFI};
//  WIFI_mode WEB_SERVER
//  STA
#define STA_SSID_DEF  "mywifi"
#define STA_PASSWD_DEF "mypasswd"
//  AP
#define AP_IP_ADDR  "192.168.0.1"
#define AP_IP_SUBNET  255,255,255,0
#define AP_SSID_DEF "espWeather"
#define AP_PASSWD_DEF  "espWeather"

//
//  ==================  SERVER  ==================
//

// time request
#define HOST "weather"
#define PORT 80
#define SERVER_ROOT  "/"
#define REQUEST_DATE_URL "www.yandex.ru"
// "www.google.com"
#define  NOT_FOUND "/NotFound.htm"

// thingspeak update request
#define THING_SPEAK_HOST "api.thingspeak.com"




//
// ====================  SEMAPHORE  ==================
//

#define CYCLE_DURATION 5 // call ticker period in seconds
#define SENSORS_REQUEST_PERIOD 300 //  in seconds





#endif

