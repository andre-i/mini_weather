

#include "props.h"
#include "Sensors.h"
#include "Util.h"
#include <Ticker.h>

#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <ESP8266mDNS.h>
#include <FS.h>

// mark for version software
String version = "0.1";

bool DEBUG = true;
int wifiMode = DEVICE_NOT_WIFI;

Sensors s(ONE_WIRE_BUS, DHT_TYPE);
Util util(true);
Ticker timer;

bool isSetDate = false; // if current date appointed must be set to true
bool isFS = true; // set to true if server content place on SD else place is SPIFFS

//  semaphore
const byte DHT_SENSOR = 1;
const byte DS_SENSOR = 2;
const byte BMP_SENSOR = 3;
byte sensorToCheck = DHT_SENSOR;

bool isCheckSensors = false;
bool isAddMinute = false;

long cycleDuration = CYCLE_DURATION;
long tickDuration = SENSORS_REQUEST_PERIOD;
int maxTickCount;
int tickCount = 0;

void tick() {
  // if (DEBUG)Serial.println("Tick");
  tickCount++;
  isCheckSensors = true;
  if (tickCount == maxTickCount) {
    // in debug mode write to file all minute
    if (DEBUG) util.setDebug(true);
    isAddMinute = true;
    tickCount = 0;
  }
}



/** print current date */
String getFullDate() {
  String full = "date: " + util.getDay() + "/" + util.getMonth() + "/" + util.getYear() + "  h:" + util.getHour() + " ]";
  return full;
}

void printFullDate() {
  String full = " NOW[ y:" + util.getYear() + "  m:" + util.getMonth() + "  d:" + util.getDay() + "   h:" + util.getHour() + " ]";
  Serial.println(full);
}

void readFile(char answ[]) {
  String fName = answ;
  fName = fName.substring(2, fName.length());
  File file = SPIFFS.open(fName.c_str(), "r");
  if (!file) {
    Serial.print("Can`t open file: ");
    Serial.println(fName);
    Dir dir = SPIFFS.openDir(fName.c_str());
    String content = "";
    while (dir.next()) {
      content += "\t" + dir.fileName() + "\t";
      File f = dir.openFile("r");
      content += String(f.size()) + "\n";
      f.close();
      delay(60);
    }
    if (content.length() > 1) {
      Serial.println("File is directory it contains:");
      Serial.println(content);
    }
    return;
  } else {
    Serial.println("\t______ file content : ____");
    while (file.available())Serial.write(file.read());
    Serial.println("\n\t  ________   EOF  _______  ");
    file.close();
  }
}

void showManual() {
  Serial.println(" Console command:\n1) DEBUG : \n\t\"y\" - ON  \n\t\"n\" - OFF ");
  Serial.println("2) Check to work with SPIFFS :\n\t\"sp\" - print whether available SPIFFS");
  Serial.println("3) Read file from SPIFFS : r_/full/path/with/fileName.ext  where:\n\t r_  - is signal on print file(or directory) content \n\t /full/path/with/fileName.ext - fullFile name");
  Serial.println("4) See SPIFFS info : \n\t\"si\" - print info about SPIFFS of chip");
  Serial.println("5) Set date/time - enter it in format:\"year/month/day/hour/minute\"");
  Serial.println("\tWARNING - date must be strongly follow format, else date be break");
  Serial.println("\tyear - 4 numbers\n\t\tmonth - 3 chars( first must be Upper Case)\n\t\tday, hour and minute consist from 2 numbers, if value less than 10 firs must be 0(null)");

}

void handleSerial() {
  char *r;
  char answ[40];
  int i = 0;
  delay(20);
  while (Serial.available() > 0) {
    answ[i] = Serial.read();
    i++;
    delay(20);
  }
  answ[i] = '\0';
  r = answ;
  /*
    if (DEBUG) {
    Serial.print("I get:" );
    Serial.println(r);
    }
  */
  if (i < 3) {
    if (answ[0] == 'y' || answ[0]  == 'Y') {
      DEBUG = true;
      Serial.println(" ON DEBUG output!");
    } else if (answ[0] == 'n' || answ[0] == 'N') {
      DEBUG = false;
      Serial.println(" OFF DEBUG output!");
    } else if (answ[0] == 's' && answ[1] == 'p') {
      isFS = util.initFS();
    } else if (answ[0] == 's' && answ[1] == 'i') {
      Serial.println(util.fsINFO());
    } else {
      showManual();
    }
  } else {
    if (answ[0] == 'r' && answ[1] == '_') {
      readFile(answ);
      return;
    }
    //  set date-time
    if (answ[0] == '2' && answ[1] == '0' ) {
      if (util.assignTime(r)) {
        String answ = "(Ok) Success set date-time: " + util.getYear() + "/" + util.getMonth() + "/" + util.getDay() + " " + util.getHour() + "h";
        Serial.println(answ);
        isSetDate = true;
        //    checkShiftTime();
      } else {
        Serial.println("(No) Fail set date-time");
      }
    }
  }

}

// ==========================================
//          esp web server
// ==========================================
const String serverRoot = SERVER_ROOT;

ESP8266WebServer server(PORT);

// ====== standart server specific ========

void returnOK() {
  server.send(200, "text/plain", "");
}

void returnFail(String msg) {
  server.send(500, "text/plain", msg + "\r\n");
}



void handleNotFound() {
  // if (isFS && loadFile(NOT_FOUND)) return;
  String message = "<html><body><br><h1>__RESOURCE_NOT_FOUND __</h1><br>";
  message += "<h3><!-- \n -->URI: ";
  message += server.uri();
  message += "</h3><h3><!-- \n --> Method: ";
  message += (server.method() == HTTP_GET) ? "GET" : "POST";
  message += "</h3><h3><!-- \n --> Arguments: ";
  message += server.args();
  message += "</h3><!-- \n --> ";
  for (uint8_t i = 0; i < server.args(); i++) {
    message += "<h4><!-- \n --> NAME:" + server.argName(i) + "</h4><h4>  VALUE:" + server.arg(i) + "</h4>  ";
  }
  message += "<!-- \n --><br><hr width='60%'><!-- \n -->";
  server.send(404, "text/html", message);
  if (DEBUG)Serial.println(message);
}

void handleRoot() {
  Serial.println("Call \"handleRoot\"");
  if (isFS) loadFile(serverRoot + "index.htm");
  else Serial.println("WARNING : Can`t load root");
}





void sendErrorAsJson( String path) {
  path = "{ \"err\" : \"" + path  + "\" }";
  server.send( 404, "application/json", path);
}

// ------ server read fileSystem ------

bool readSPIFFS(String path, String dataType) {
  fs::File dataFile = SPIFFS.open(path.c_str(), "r");
  bool res = false;
  if (!dataFile) {
    if ( path.startsWith(SENSOR_DATA_DIR)) {
      sendErrorAsJson(path);
      res = false;
    }
    path = "SPIFFS : Can`t read File :" + path;
    Serial.println(path);
    return false;
  } else {
    if (server.streamFile(dataFile, dataType) != dataFile.size()) {
      Serial.println("WARNING : Sent less data than expected!");
    } else {
      if (DEBUG) {
        String out = "SPIFFS send file: " + path + "   dataType: " + dataType;
        Serial.println(out);
      }
    }
  }
  dataFile.close();
  return true;
}

bool loadFile(String path) {
  String dataType = "text/plain";
  if (path.endsWith("/")) path = serverRoot + path + "index.htm";
  if (path.endsWith(".htm")) dataType = "text/html";
  else if (path.endsWith(".css")) dataType = "text/css";
  else if (path.endsWith(".js")) dataType = "application/javascript";
  else if (path.endsWith(".json")) dataType =  "application/json";
  else if (path.endsWith(".png")) dataType = "image/png";
  else if (path.endsWith(".gif")) dataType = "image/gif";
  else if (path.endsWith(".jpg")) dataType = "image/jpeg";
  else if (path.endsWith(".svg")) {
    dataType = "image/svg+xml";
  }
  if (DEBUG) {
    Serial.print("Try get file: ");
    Serial.print(path);
    Serial.println("  from FPIFFS");
  }
  return readSPIFFS(path, dataType);
}
//   ends of read filesystem


void sendFileContent() {
  if (server.hasArg("fName")) {
    loadFile(server.arg("fName"));
  }
}

//  example request: /sensorData?period=2018/Jun.txt
const String sensorDataDir = SENSOR_DATA_DIR;
void sendSensorData() {
  if (server.hasArg("period")) {
    loadFile(sensorDataDir + "/" + server.arg("period"));
  }
}


// resources
// if want file as arguments with name "date" it value must consist from
//    "year/month" string where "month" is first 3 chars
void handleGetSrc() {
  String resPath = serverRoot + "src/";
  if (server.hasArg("js"))resPath += "js/" + server.arg("js");
  else if (server.hasArg("pic")) resPath += "pic/" + server.arg("pic");
  else if (server.hasArg("css"))resPath += "css/" + server.arg("css");
  else if (server.hasArg("date")) {
    resPath = SENSOR_DATA_DIR ;
    resPath += "/" + server.arg("date") + ".txt";
  } else {
    if (server.args() < 1) {
      handleNotFound();
      return;
    } else {
      resPath = server.arg(server.argName(0));
    }
  }
  loadFile(resPath);

}


//  sensors data
// send current values all sensors
void sendCurrent() {
  if (DEBUG) {
    Serial.print("Server get request \"lastValues\" ");
  }
  server.send(200, "application/json", s.getCurrentAsJSON());
}

/*
 	send last values of checked sensors
    reques looks - /lastValues?sensor=sensor_name
*/
void sendLast() {
  if (DEBUG)Serial.print(" HTTP SERVER request of \"lastValues\" for: ");
  String sType = "";
  if (server.hasArg("sensor")) {
    sType = server.arg("sensor");
    if (DEBUG) {
      Serial.println(sType);
    }
    if (sType == T_IN || sType == T_OUT || sType == BARO || sType == HUMID) {
      server.send(200, "application/json", s.getLastAsJSON(sType));
    } else {
      server.send(400, "text/plain", String("{\"Not have sensor\" : \"") + String(sType) + "\" }");
      return;
    }
  }
}

void sendPeriods() {
  if (DEBUG) Serial.println("browser req: /availablePeriod ");
  server.send(200, "application/json", util.getPeriodsAsJSON());
}

void prepareServer() {
  server.on("/", HTTP_GET, handleRoot);
  // send resource
  server.on("/src", HTTP_GET, handleGetSrc);
  // send values
  server.on("/current", HTTP_GET, sendCurrent);
  server.on("/lastValues", HTTP_GET, sendLast);
  server.on("/readFile", HTTP_GET, sendFileContent);
  server.on("/sensorData", HTTP_GET, sendSensorData);
  server.on("/availablePeriod", HTTP_GET, sendPeriods);
  server.begin();
  delay(1000);
}

//  ==========================================
//            end sever
//  ==========================================




void setup() {
  Serial.begin(115200);
  Serial.println(String("\n\t version:" + version));
  Serial.println("\n\n\tDHTxx DS18b20 BMP280 test!");
  Serial.print(" Flash size:\n\treal = ");
  Serial.print(ESP.getFlashChipRealSize());
  Serial.print("\t programm = ");
  Serial.println(ESP.getFlashChipSize());
  maxTickCount = cycleDuration / tickDuration;
  int sensInit = s.init();
  // try init sd card
  if (!util.initFS()) {
    Serial.println("\tWARNING : SPIFFS not init");
    isFS = false;
  } else isFS = true;
  wifiMode = util.initWIFI();
  switch (wifiMode) {
    case DEVICE_STA_MODE:
      isSetDate = util.sync();
      if (isSetDate) {
        printFullDate();
        if (isFS) {
          String res = (sensInit == 0) ? " success init" : " fail init";
          res = "[ SPIFFS: init,  sensors: " + res + ",  wi-fi: STA_MODE ]";
          res = getFullDate() + res;
          util.writeLog(res);
        }
      }
      break;
    case  DEVICE_AP_MODE:
      Serial.println("Device start WiFi as AP");
      if (isFS)Serial.println("Perhaps need set date for write sensors data to SD card.\nPrint \"h\" for detail");
      isSetDate = false;
      break;
    case DEVICE_NOT_WIFI:
      Serial.println("WARNING: device can`t start of WiFi!!!\nPerhaps need set date for write sensors data to SD card.\nPrint \"h\" by serial for detail");
      isSetDate = false;
      break;
  }
  /*
    // file system
    if (!SPIFFS.begin()) {
    Serial.println("WARNING: SPIFS not mount");
    }
  */
  bool isMDNS = MDNS.begin(HOST) ;
  if (isMDNS) {
    Serial.println("MDNS responder started");
    Serial.print("You can now connect to http://");
    Serial.println(HOST);
    Serial.println(".local");
  } else Serial.println("Can`t start mDNS");
  if (DEBUG) {
    Serial.print("tickDuration = "); Serial.print(tickDuration);
    Serial.print("    maxTickCount = "); Serial.println(maxTickCount);
  }
  prepareServer();
  if (isMDNS)MDNS.addService("http", "tcp", PORT);

  timer.attach(tickDuration, tick);
}



void loop() {
  if (Serial.available() > 0)handleSerial();
  // for WiFi
  server.handleClient();
  if (isCheckSensors) {
    switch (sensorToCheck) {
      case DHT_SENSOR:
        s.readDHT();
        sensorToCheck = DS_SENSOR;
        break;
      case DS_SENSOR:
        s.readDS18B20();
        sensorToCheck = BMP_SENSOR ;
        break;
      case BMP_SENSOR:
        s.readBMP280();
        sensorToCheck = DHT_SENSOR;
        s.makeCurrentToJSON();
        isCheckSensors = false;
        break;
      default:   delay(70);
    }
  } else delay(70);
  if (isSetDate && isAddMinute) {
    if (DEBUG) {
      Serial.println("call util.addMinute()");
      Serial.println(s.getCurrentAsJSON());
    }
    util.addMinute();   // shift time on 1 minute
    isAddMinute = false;
  }
  if (util.hasWrite()) {
    // if (DEBUG)Serial.println("Try write sensors values to SD card");
    if (isFS)util.writeSensorsValues(s.getMedia(T_IN), s.getMedia(T_OUT), s.getMedia(BARO), s.getMedia(HUMID));
    else("WARNING - not access to SD card - Can`t write sensors values");
  }
}











