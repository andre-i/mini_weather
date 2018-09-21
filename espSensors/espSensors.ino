
#include "props.h"
#include "Sensors.h"
#include "Util.h"
#include "SerialHandler.h"
#include <Ticker.h>

#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <ESP8266mDNS.h>
#include <FS.h>

// mark for version software
static String version = "0.05";

bool LOG = true;
bool DEBUG = false;
int wifiMode = DEVICE_NOT_WIFI;
int errorCode = 0;

//  led controll
// if wifi STA_mode - up the GPIO12
// if wifi AP_mode - up the GPIO13
int STA_PIN = 12;
int AP_PIN = 13;

// thingSpeak
char* writeApiKey;
char key[30];
bool isThingSpeak = false;
bool isResendThingspeak = false;
long startDelayThingspeak = 0;


Util util(LOG);
Sensors s(ONE_WIRE_BUS, DHT_TYPE);
SerialHandler sHandler(&util);
Ticker timer;

bool isSetDate = false; // if current date appointed must be set to true
bool isFS = true; // access to SPIFFS

// action semaphore
const byte DHT_SENSOR_READ = 1;
const byte DS_SENSOR_READ = 2;
const byte BMP_SENSOR_READ = 3;
const byte THING_SPEAK_WRITE = 4;
byte executeAction = DHT_SENSOR_READ;

static bool isCheckSensors = false;
static bool isAddMinute = false;
static bool isWriteValues = false;

//  semaphore
static int tickDuration = CYCLE_DURATION;
static int checkSensorsTick = SENSORS_REQUEST_PERIOD / CYCLE_DURATION ;
static int tickInMinute = 60 / CYCLE_DURATION;
static int tickInHour = 3600 / CYCLE_DURATION;
static int currentInMinute = 0;
static int currentInHour = 0;
static int currentForCheck = checkSensorsTick - 1;

/**
    set state for
    1.check sensors
    2.set current time
    3.write values to storage
    in debud mode: any tick - check sensors, any minute - write sensors values to storage
*/
void tick() {
  // if (LOG)Serial.println("Tick");
  currentInMinute++;
  currentInHour++;
  currentForCheck++;
  if (currentInMinute == tickInMinute) {
    isAddMinute = true;
    currentInMinute = 0;
  }
  if (currentInHour == tickInHour) {
    isWriteValues = true;
    currentInHour = 0;
  }
  if (currentForCheck == checkSensorsTick) {
    isCheckSensors = true;
    currentForCheck = 0;
  }
  if (DEBUG) {
    isCheckSensors = true;
    if (currentInMinute == 0)isWriteValues = true;
  }
}

void printFullDate() {
  String full = " NOW[ y:" + util.getYear() + "  m:" + util.getMonth() + "  d:" + util.getDay() + "   h:" + util.getHour() + " ]";
  Serial.print(full);
}

/**
   listen serial port for execute some action on
   serial request if error case
*/
void listenSerialIfError( int millisecond) {
  unsigned long now = millis();
  while ((millis() - now) < millisecond) {
    if (Serial.available() > 0)sHandler.handle();
  }
}

//
//      ===============  ERROR`S  ==============
/*
    On error mode with led blink
    blink codes:
    1 times - wrong DHT
    2 times - wrong BMP280
    3 times - wrong both sensors
    4 times - wrong wifi
    5 times - can`t connect with server for get time

    If error cause  wifi chip reboot after ~ 10 minutes
*/
void upErrorMode(int blinkCount) {
  int n = 1;
  digitalWrite(STA_PIN, LOW);
  digitalWrite(AP_PIN, LOW);
  Serial.println("Error code:\n  1 - wrong DHT\n  2  - wrong BMP280\n  3 - wrong both sensors\n  4 - wrong up wifi\n  5 - can`t connect with server for get time");
  Serial.print("\n\n\t\tERROR  code=");
  Serial.print(blinkCount);
  if(errorCode > 0){
    Serial.print( " | error sensors  code=");
    Serial.print(errorCode);
    Serial.print("  ");
    Serial.print(getErrorCase(false));
  }
  Serial.println();
  listenSerialIfError(2000);
  while (true) {
    if (blinkCount > 3) n++;
    if (Serial.available() > 0)sHandler.handle();
    for (int i = 0; i < blinkCount; i++) {
      digitalWrite(AP_PIN, HIGH);
      listenSerialIfError(400);
      digitalWrite(AP_PIN, LOW);
      listenSerialIfError(300);
    }
    listenSerialIfError(8000);
    if (blinkCount > 3 && (n + 20) % 21 == 0) {
      Serial.println( String("\nwifi ERROR = ") +
                      String(blinkCount) +
                      String("  reboot after ") +
                      String((63 - n) * 11) + String(" second!!!"));
    }
    if ( n > 63) ESP.restart();
  }
}

/*
   return error case
   param:
   @ isHTML - if true return error case as part html code
*/
String getErrorCase(bool isHTML) {
  String clause = "";
  switch (errorCode) {
    case 1: clause = " [ DHT error ]"; break;
    case 2: clause = " [ BMP280 error ]"; break;
    case 3: clause = " [ DHT error, BMP280 error ]"; break;
  }
  if(!isHTML)return clause;
  String mess = "<br><h2 style=\"color:red;\"> ERROR code=";
  mess += errorCode + clause + " </h2>";
  return mess;
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
  if (LOG)Serial.println(message);
}

void handleRoot() {
  Serial.println("Call \"handleRoot\"");
  if (errorCode > 0) {
    sendCurrentProperties();
    return;
  }
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
      if (LOG) {
        String out = "SPIFFS send file [ " + path + "   dataType: " + dataType + " ]";
        Serial.println(out);
      }
    }
  }
  dataFile.close();
  return true;
}

bool loadFile(String path) {
  if (path.equals(PROPS_FILE)) return false;
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
  return readSPIFFS(path, dataType);
}
//   ends of read filesystem


void readFile() {
  if (LOG)Serial.println("Call \"readFile\"");
  if (server.hasArg("fName")) {
    String fName = String(server.arg("fName"));
    fName.trim();
    if ( !fName.startsWith("/")) fName = String("/") + fName;
    if (LOG) {
      Serial.print("request on file: ");
      Serial.println(server.arg(fName));
    }
    if (!loadFile(fName))handleNotFound();
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
  if (LOG) {
    Serial.print("Server get request \"/current\" ");
  }
  server.send(200, "application/json", s.getCurrentAsJSON());
}

/*
 	send last values of checked sensors
    reques looks - /lastValues?sensor=sensor_name
*/
void sendLast() {
  if (LOG)Serial.print(" HTTP SERVER request of \"lastValues\" for: ");
  String sType = "";
  if (server.hasArg("sensor")) {
    sType = server.arg("sensor");
    if (LOG) {
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
  if (LOG) Serial.println("browser req: /availablePeriod ");
  server.send(200, "application/json", util.getPeriodsAsJSON());
}

void sendCurrentProperties() {
  if (LOG)Serial.print("browser request : /getCurrentProps  answer: ");
  String res = util.getCurrentProps();
  String header = "<html><head><meta charset=utf-8></head><br><br><br><h2 align='center'>Current application parameters<br>Настройки чипа</h2><h3>";
  res.replace("\n", "<br>");
  res.replace(" ", "&nbsp;");
  res = header + res + String("</h3><hr width='80%'>");
  String ends = (errorCode > 0) ? getErrorCase(true)  + "</html>" : "</html>";
  res +=  ends;
  if (LOG)Serial.println(res);
  server.send(200, "text/html", res.c_str());
}

void loadLogFile() {
  loadFile(String(LOG_FILE));
}

void showHelp() {
  loadFile(String("/Help.htm"));
}

//  ==========================================
//            end sever
//  ==========================================


//
// =============================  SETUP EXECUTION  ==============================
//


void prepareServer() {
  server.on("/", HTTP_GET, handleRoot);
  // send resource
  server.on("/src", HTTP_GET, handleGetSrc);
  // send values
  server.on("/current", HTTP_GET, sendCurrent);
  server.on("/lastValues", HTTP_GET, sendLast);
  server.on("/readFile", HTTP_GET, readFile);
  server.on("/sensorData", HTTP_GET, sendSensorData);
  server.on("/availablePeriod", HTTP_GET, sendPeriods);
  server.on("/getProps", HTTP_GET, sendCurrentProperties);
  server.on("/log", HTTP_GET, loadLogFile);
  server.on("/help", HTTP_GET, showHelp);
  server.on("/gg", HTTP_GET, Sensors::getState); //  ========================================= cannot any thingh only test`s  ==============================================
  server.begin();
  delay(1000);
}

void showStartMessage() {
  // print chip properties
  Serial.println(String("\n\t version:" + version));
  Serial.println("\n\n\tDHTxx DS18b20 BMP280 sensors\n");
  Serial.println("\n\t___On some error will make led blink___\n 4 times blink - wifi not work\n 3 times blink - two sensor not work,\n 1 blink - error DHT\n 2 blink - error BMP280");
  Serial.println("\n\t___При проблемах мигает свеодиод___\n  1 раз - DHT неисправен \n  2 раза BMP280 неисправен\n  3 раза - проблема со всеми датчиками\n  4 раза - нет вайфай\n");
}

/**
    prepare start system
   1) set wifi state led to Up
   2) open serial connection
   3) check file system
   4) try Up wifi
 **/
void prepareStartState() {
  // on start int LOG = true - all message out un UART
  //prepare led state
  pinMode(STA_PIN, OUTPUT);
  pinMode(AP_PIN, OUTPUT);
  digitalWrite(STA_PIN, HIGH);
  digitalWrite(AP_PIN, HIGH);
  // up serial output
  Serial.begin(115200);
  showStartMessage();
  // try init sd card
  if (!util.initFS()) {
    isFS = false;
  } else isFS = true;
  // up sensors
  int sensorCode = s.init(&util);
  if ( sensorCode != 0) {
    digitalWrite(STA_PIN, LOW);
    digitalWrite(AP_PIN, LOW);
    errorCode = sensorCode;
    // on error - message appear in root server page
  }
  // up wifi
  wifiMode = util.initWIFI();
  //  set FS state to serial handler
  sHandler.setFSstate(isFS);
  //  on exit from init set debug mode
  //  if debug is false, LOG  also be false -> supress log messages
  DEBUG = util.getDebugMode();
  LOG = DEBUG;
}

/**
  try start MDNS and prepare wifi server
**/
void startMDNS() {
  if (MDNS.begin(HOST)) {
    Serial.println("\nMDNS responder started");
    Serial.print("You can now connect to http://");
    Serial.print(HOST);
    Serial.println(".local");
    prepareServer();
    MDNS.addService("http", "tcp", PORT);
    return;
  }
  Serial.println("Can`t start mDNS");
}

/**

   set led state appropriate wifi mode and write to log
   information message
*/
void afterInitAction() {
  String res = " Start weather : [ sensors ";
  res += (errorCode > 0) ? getErrorCase(false): "- success " ;
  res += ", ";
  switch (wifiMode) {
    case DEVICE_STA_MODE:
      if (!util.sync() || !util.hasSyncTime()) {
        if ( !util.restartWiFi() || !util.sync()) {
          if (!util.hasSyncTime() && util.isOnlySta()) {
            upErrorMode(5);
          } else {
            Serial.println("Can`t connect to Time server, require set date-time. See help by 'en'\n Установите время для записи значений датчиков, для справки жми 'ru'");
          }
        }
      } else {
        printFullDate();
      }
      if (isFS) {
        res += ",  wi-fi: STA_MODE ]";
        res =  util.getFullDate() + res;
      }
      digitalWrite(STA_PIN, HIGH);
      digitalWrite(AP_PIN, LOW);
      break;
    case  DEVICE_AP_MODE:
      if (isFS) {
        Serial.println("Need set date for write sensors data to storage. Print 'en' for detail\nУстановите время для записи значений датчиков, для справки жми 'ru'");
        res += ", wifi: AP_MODE ] ";
        res += "date_not_set " + res;
      }
      digitalWrite(STA_PIN, LOW);
      digitalWrite(AP_PIN, HIGH);
      break;
    case DEVICE_NOT_WIFI:
      if (isFS) {
        Serial.println("WARNING: device can`t start of WiFi!!!\nCheck start parameters or outer wifi(if work as STA).\nPrint \"en\" by serial for detail");
        Serial.println(" WiFi не стартовал проверьте начальные настройки или наличие wifi сети(если модуль настроен как клиент).\n   Для справки введи 'ru'");
        res += ", can`t start Wi-Fi !!!";
        res += " date_not_set " + res;
      }
      digitalWrite(STA_PIN, LOW);
      digitalWrite(AP_PIN, LOW);
      res = "FATAL EXIT - " + res;
      if (isFS && !util.isOnlySta())util.writeLog(res);
      // enter blink on error
      upErrorMode(4);
      break;
  }
  if (wifiMode == DEVICE_STA_MODE) {
    writeApiKey = key;
    util.getThingSpeakKey(writeApiKey);
    Serial.println(String("thingspeak key = ") + String(writeApiKey));
    if (writeApiKey[0] != '\0' && String(writeApiKey).length() > 14)isThingSpeak = true;
    if (LOG)Serial.println(String("isThingspeak =") + isThingSpeak);
  } else isThingSpeak = false;
  res += "  Thingspeak:";
  if (isThingSpeak) {
    res += "true";
  } else res += "false  ";
  // write to log wifi and sensors state
  if (isFS) util.writeLog(res);
}

/*
   On start execute
*/
void setup() {
  prepareStartState();
  // tune server
  if (wifiMode != DEVICE_NOT_WIFI) {
    startMDNS();
  }
  // try sync date-time from the web
  // and out the message by state system
  afterInitAction();
  if (LOG) {
    Serial.print("tickDuration = "); Serial.println(tickDuration);
  }
  Serial.println("\n For show log press on 'y' \n нажать на 'y' - показ отладочных сообщений\n press 'n' - hide log(скрыть вывод)\n press 'en' for help\n 'ru' показать справку");

  // start timer
  timer.attach(tickDuration, tick);
}


//
//  =============   LOOP  EXECUTION  ===================
//


void writeSensorsValues() {
  if (util.hasWrite()) {
    util.writeSensorsValues(s.getMedia(T_IN), s.getMedia(T_OUT), s.getMedia(BARO), s.getMedia(HUMID));
  } else {
    Serial.println("WARNING - Can`t write sensors values (not access to storage or not set time)");
  }
}

/**
   Call util methods for write data to thingspeak site
*/
void sendDataToThingSpeak() {
  const char* host = THING_SPEAK_HOST;
  String result = "/update?api_key=";
  result += writeApiKey;
  result += "&field1=";
  result += s.getT_In();
  result += "&field2=";
  result += s.getT_Out();
  result += "&field3=";
  result += s.getHumid();
  result += "&field4=";
  result += s.getBaro();
  int res = util.writeDataToThingspeak(host, result);
  if (LOG) {
    switch (res) {
      case 0: Serial.println("Succes"); break;
      case 1: Serial.println("Empty server response"); break;
      case 2: Serial.println("Can`t connect to thingspeak"); break;
      case 3: Serial.println("Server Error"); break;
    }
  }
  // delay 15sec and yet one connection if fail
  if (res > 1) {
    if (isResendThingspeak) {
      // on fail send data to thingspeak check wifi connect
      if ( res == 2 && !util.sync() ) {
        if(LOG)Serial.println(util.getFullDate() + String(" WARNING [ On fail thingspeak try restart wifi or chip] "));
        if (!util.restartWiFi())ESP.restart();
      }
      printFullDate();
      Serial.println(" twice fail send data to thingspeak");
      isResendThingspeak = false;
    } else {
      isResendThingspeak = true;
      startDelayThingspeak = millis();
    }
  } else {
    isResendThingspeak = false;
  }
}

/**
   read sensors values and send data to thingspeak on schedule time
*/
void executeOnSchedule() {
  if (isCheckSensors) {
    switch (executeAction) {
      case DHT_SENSOR_READ:
        s.readDHT();
        executeAction = DS_SENSOR_READ;
        break;
      case DS_SENSOR_READ: // read DS18B20 if IS_DS18B20 in props file set to true
        if (LOG)s.readDS18B20();
        executeAction = BMP_SENSOR_READ ;
        break;
      case BMP_SENSOR_READ:
        s.readBMP280();
        if (DEBUG) {
          executeAction = DHT_SENSOR_READ ;
          isCheckSensors = false;
        } else {
          executeAction = THING_SPEAK_WRITE;
        }
        s.makeCurrentToJSON(); // on end cucle sensors request - make sensors values as JSON string
        break;
      case THING_SPEAK_WRITE:
        isCheckSensors = false;
        executeAction = DHT_SENSOR_READ;
        if (isThingSpeak) {
          if (LOG)Serial.print("Try send data to Thingspeak");
          sendDataToThingSpeak() ;
        }
        break;
    }
  }
}

/**
    call some action on add minute
*/
void executeOnAddMinute() {
  if (DEBUG) {
    Serial.println("\nDEBUG ON");
    if (isThingSpeak) {
      Serial.print( " Try send to Thingspeak ");
      sendDataToThingSpeak();
    }
  }
  if (LOG) {
    Serial.println("call util.addMinute()");
    if (DEBUG) Serial.println(s.getCurrentAsJSON());
  }
  util.addMinute();   // shift time on 1 minute
  isAddMinute = false;

}

void loop() {
  if (Serial.available() > 0)sHandler.handle();
  // on wrong send data to thingspeak - resend it
  if (isResendThingspeak) {
    if ( (millis() - startDelayThingspeak ) > 15100) {
      sendDataToThingSpeak();
      startDelayThingspeak = 0l;
    }
  }
  // server
  server.handleClient();
  // action by schedule(check sensors, send to Thingspeak
  executeOnSchedule();
  delay(70); // is want for good wi-fi work
  // call action on next tick in clock
  if (isAddMinute) executeOnAddMinute();
  // write sensors values in file system if set time and
  // file system is accessed
  if ( isWriteValues ) {
    writeSensorsValues();
    isWriteValues = false;
  }
}











