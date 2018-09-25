#include "Util.h"

extern bool LOG;
extern bool DEBUG;


Util::Util(bool isLog) {
  LOG = isLog;
  DEBUG = getDebugMode();
  //  init file system
  isFS = (SPIFFS.begin()) ? true : false;
}

void Util::setDebug(bool isDebug) {
  DEBUG = isDebug;
}

bool Util::getDebugMode() {
  char debugMode[6];
  debugMode[0] = '\0';
  fillParam(DEBUG_MODE, debugMode);
  if (debugMode[0] == '\0')return false;
  if ( debugMode[0] == 't' && debugMode[1] == 'r' && debugMode[2] == 'u' && debugMode[3] == 'e')return true;
  else return false;
}
//  =================== work with init params  ====================

/**
   try read init param from file by it name,
   if not , then set to param '\0' - null string

   key - name parameter in props file( see for help props.h file)
   dest - string for write parameter value
*/
void Util::fillParam(const char *key, char *dest) {
  if (!isFS) {
    dest[0] = '\0';
    return;
  }
  if (LOG)Serial.println(String("Try get value for : ") + String(key));
  File file;
  if (SPIFFS.exists(PROPS_FILE)) {
    file =  SPIFFS.open(PROPS_FILE, "r");
    if (!file) {
      dest[0] = '\0';;
      if (LOG)Serial.println(" Can`t open PROPS_FILE - fill wi-fi params with default values");
      return;
    }
  } else {
    dest[0] = '\0';
    if (LOG)Serial.println("PROPS_FILE not found - fill wi-fi param with default value");
    return;
  }
  String cur;
  while ( file.available()) {
    cur = file.readStringUntil('\n');
    // if (LOG)Serial.println(cur);
    if (cur.startsWith(key)) {
      cur = cur.substring(2, cur.length());
      cur.trim();
      int i;
      for (i = 0; i < cur.length(); i++)*(dest + i) = cur.charAt(i);
      *(dest + i) = '\0';
      if (DEBUG)Serial.println(String("On request - ") + String(key) + String("  return - '") + String(dest) + String("' "));
      file.close();
      return;
    }
  }
  // value by key not found
  if (LOG)Serial.println(String("Can`t read property ") + String(key));
  dest[0] = '\0';
  file.close();
  return;
}

// ==================== SPIFFS =====================

// private
void Util::prepareLogFile() {
  File logFile = SPIFFS.open(LOG_FILE, "w+");
  int size = logFile.size();
  if (size < 10000) {
    logFile.close();
    return;
  }
  int last = 1000;
  logFile.seek(last, SeekEnd);
  unsigned char res[1001];
  last = 0;
  while (logFile.available()) {
    res[last] = logFile.read();
    last++;
  }
  logFile.write(*res);
}

//  public :
bool Util::initFS() {
  if (LOG)Serial.print("init SPIFFS : ");
  if (isFS) {
    Serial.println(" FS already started");
  } else if (SPIFFS.begin()) {
    Serial.println("success");
    isFS = true;
    prepareLogFile();
  } else {
    Serial.println("fail!  WARNING \"esp_server can`t write data to storage\"");
    isFS = false;
  }
  return isFS;
}

void Util::writeLog(String mess) {
  if (DEBUG) {
    Serial.print("Send to log: ");
    Serial.println( mess );
    return;
  }
  if (isFS) {
    File logFile = SPIFFS.open(LOG_FILE, "a");
    if (!logFile) {
      String fail = "Can`t write to \"" + String(LOG_FILE) + "\" \nmessage: [ " + mess + " ]";
      Serial.println(fail);
    } else {
      logFile.println(mess);
      logFile.close();
    }
  } else {
    Serial.print("FileSystem not work can`t write to log message \n  [");
    Serial.print(mess);
    Serial.println("]");
  }
}

String Util::fsINFO() {
  String res = "\t\tFS info:\n";
  if (!isFS)return res += "SPIFFS not init, INFO not available.";
  FSInfo fs_info;
  if (SPIFFS.info(fs_info)) {
    /*   FSInfo
      size_t totalBytes;
      size_t usedBytes;
      size_t blockSize;
      size_t pageSize;
      size_t maxOpenFiles;
      size_t maxPathLength;
    */
    res += "\ttotalBytes: " + String(fs_info.totalBytes);
    res += ",\tusedBytes: " + String(fs_info.usedBytes);
    res += "\n\t files - maxOpen: " + String(fs_info.maxOpenFiles) + ",\tmaxPathLength: " + String(fs_info.maxPathLength);

  } else {
    res = "SPIFFS is available, but can`t read FS info";
  }
  return res;
}

String Util::getPeriodsAsJSON() {
  Dir dataDir = SPIFFS.openDir(SENSOR_DATA_DIR);
  String files = "[";
  while (dataDir.next())files += " \"" + dataDir.fileName() + "\",";
  int last = files.lastIndexOf(",");
  if (last > 0) files = files.substring(0, last) + " ]";
  else files = "[ ]";
  if (LOG)Serial.println((String("periodsAsJSON: ") + files));
  return files;

}

//
// =============== write sensors values  ========================================
//

bool Util::hasWrite() {
  return isFS && isSetTime;
}

void Util::writeSensorsValues(int tIn, int tOut, int baro, int humid) {
  String fullMonthFileName = sensorDataDir + pathSeparator + year + pathSeparator + month + ".txt";
  File monthF = SPIFFS.open(fullMonthFileName, "a");
  if (!monthF) {
    writeLog("Failed to open : " + fullMonthFileName);
    if (LOG)Serial.println("WARNING - fail open file for write sensors data to file system");
    monthF.close();
    return;
  }
  String output = getDay() + valSeparator + getHour() + valSeparator + tIn + valSeparator + tOut + valSeparator + baro + valSeparator + humid;
  if (LOG) {
    Serial.print("Try Write sensors values to file: ");
    Serial.print(fullMonthFileName);
    Serial.print(" [ ");
    Serial.print(output);
    Serial.print(" ] ");
  }
  monthF.println();
  monthF.print(output);
  monthF.close();
  if (LOG) Serial.println( "- Success write!");
}

bool Util::hasFS() {
  return isFS;
}

//
//  ===============  Wi-Fi  =======================
//

//  public

bool Util::isOnlySta() {
  return hasOnlySta;
}

/**
   Try connect to AP by address from property file and 80 port, if success return true
   it is maked for check wifi connect on suspend state
   If this method return false -> chip can`t connect with AP and
   leave wifi is suspend.

   Method be work, if access point have network interface on given address otherwise
   return true.
   return &
      true if 1)success connect with access point
              2)AP mode
              3)address not set
      false on fail connect
*/
bool Util::isApConnected() {
  if (wifiMode == DEVICE_AP_MODE)return true;
  if (apInterfaceAddress[0] == '\0')return true;
  const char *addr = apInterfaceAddress;
  if (!connect(addr, defaultPort))return false;
  disconnect();
  return true;
}

/**
    restart WiFi on demand.
    return @
    true if success restart otherwise - false
*/
bool Util::restartWiFi() {
  WiFi.disconnect();
  if (LOG)Serial.println("Try restart WiFi");
  long now = millis();
  int n = 0;
  while ((millis() - now) < 10000) {
    delay(20);
    if (LOG) {
      Serial.print(".");
      if (n % 100 == 0)Serial.println();
      n++;
    }
  }
  return initWIFI() != DEVICE_NOT_WIFI;
}

/*
   tru init wifi as STA or AP
   return wif mode (sta, ap or not wifi)
*/
int Util::initWIFI() {
  wifiMode = DEVICE_NOT_WIFI;
  //  set from file whether only STA mode
  char staOnly[6];
  staOnly[0] = '\0';
  fillParam(ONLY_STA, staOnly);
  hasOnlySta = (strncmp(staOnly, "true", 4) == 0);
  if (LOG)Serial.println("\n-----\n ------  work ONLY STA  ------ \n----- ");

  if ( isStaConnect()) {
    wifiMode = DEVICE_STA_MODE;
    // set from file address network interface of access point
    char apAddr[100];
    apAddr[0] = '\0';
    apInterfaceAddress = apAddr;
    fillParam(AP_NETWORK_ADDRESS, apAddr);
    int n = 0;
    while (apAddr[n] != '\0')n++;
    if ( n < 6) {
      char str[1];
      str[0] = '0';
      apInterfaceAddress = str;
    }
    else {
      char str[n + 1];
      n = 0;
      while (apAddr[n] != '\0') {
        str[n] = apAddr[n];
        n++;
      }
      str[n] = apAddr[n];
      apInterfaceAddress = str;
    }
    return wifiMode;
  }
  if (hasOnlySta) {
    WiFi.disconnect();
    return wifiMode;
  }
  if ( isSetApMode() )wifiMode = DEVICE_AP_MODE;
  return wifiMode;
}

// private

// return true if conneted to wifi access point
bool Util::isStaConnect() {
  char s[15], p[15];
  s[0] = '\0';
  p[0] = '\0';
  char *ssid = s, *passwd = p;
  fillParam(STA_SSID, ssid); // read Start Param from file sysytem
  if (!ssid || ssid[0] == '\0') ssid  = STA_SSID_DEF;
  fillParam(STA_PASSWD, passwd); // read Start Param from file sysytem
  if (!passwd || passwd[0] == '\0')passwd = STA_PASSWD_DEF;
  WiFi.begin(ssid, passwd);
  // Wait for connection
  if (DEBUG)Serial.println(String("Try connect by WiFi [ ssid=") + String(ssid) + String("  password=") + String(passwd) + String(" ]"));
  int counter = 0;
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
    counter ++;
    if ( counter > 15 ) {
      WiFi.disconnect();
      delay(100);
      if (LOG)Serial.println(String("\nWi-Fi STA mode: can`t connect to ") + String(ssid));
      return false;
    }
  }
  Serial.println("");
  if (LOG) {
    Serial.print("\nWiFi work as STA connected to ");
    Serial.print(String(ssid) + String("  password:'") + String("?????' "));
    Serial.print("  IP address: ");
    Serial.println(WiFi.localIP());
  }
  return true;
}

// return true if success up AP mode on chip
bool Util::isSetApMode() {
  String err = "";
  // If AP mode
  char s[15], p[15], addr_str[18];
  s[0] = '\0';
  p[0] = '\0';
  addr_str[0] = '\0';
  char *ssid = s, *passwd = p;
  fillParam(AP_SSID, ssid); // read Start Param from file sysytem
  if (!ssid || ssid[0] == '\0') ssid = AP_SSID_DEF;
  fillParam(AP_PASSWD, passwd); // read Start Param from file sysytem
  if (!passwd || passwd[0] == '\0')passwd = AP_PASSWD_DEF;
  //  ip address
  char *ip = addr_str;
  fillParam(AP_IP, ip); // read Start Param from file sysytem
  if (!ip || ip[0] == '\0') ip = AP_IP_ADDR;
  int addr[4];
  parseAddr( ip, addr);
  // check on valid
  if ( !addr[0] || addr[0] < 0) {
    err += "ERROR [ I get wrong IP address for AP mode wi-fi = '";
    err += ip;
    err += "'] check props.txt file";
    writeLog(err);
    if (LOG)Serial.println(err);
    return false;
  }
  if (WiFi.softAP(ssid, passwd)) {
    const IPAddress ap_ip(addr[0], addr[1], addr[2], addr[3]);
    WiFi.softAPConfig(ap_ip, ap_ip, subnet);
    if (LOG) {
      Serial.print("WI-FI work as AP ssid=");
      Serial.print(ssid);
      Serial.print("  passwd=");
      Serial.println(passwd);
    } else Serial.println("Start Wi-Fi as AP");
    return true;
  } else {
    err = "ERROR esp_server can`t UP wifi AP ssid='";
    err += ssid ;
    err += "' password='";
    err += passwd;
    err += "' IP address='";
    err += addr_str;
    writeLog(err);
    if (LOG)Serial.println(err);
    return false;
  }
}

/**
   make divide given string into integer array
   if can`t make this first memeber be -1
*/
void Util::parseAddr(char *ip, int addr[4]) {
  char c;
  char curr[4];
  int j = 0, k = 0;
  c = *ip;
  for (int i = 0; c != '\0'; i++) {
    c = *(ip + i);
    if (c == ' ')continue;
    if ( c != '.' && c != 0) {
      int n = c + 0;
      if ( c < 48 || c > 57 ) {

        addr[0] = -1;
        return;
      }
      curr[k] = c;
      k++;
    } else if ( c == '.') {
      curr[k] = '\0';
      k = 0;
      addr[j] = atoi(curr);
      j++;
    }
  }
  curr[k] = '\0';
  addr[j] = atoi(curr);
  if (j < 3) addr[0] = -1;
}


//  ==================================   date-time =========================

// public

// time string must have format "year/month/day/hour/minute"
bool Util::assignTime(char * current) {
  char * part = strtok(current, "/");
  if (part != NULL) year = part;
  else return false;
  part = strtok(NULL, "/");
  if (part != NULL) month = part;
  else return false;
  part = strtok( NULL, "/");
  if ( part != NULL)day = part;
  else return false;
  part = strtok( NULL, "/");
  if ( part != NULL)hour = part;
  else return false;
  part = strtok( NULL, "/");
  if ( part != NULL) {
    s_min = part;
    setDateTime(false);
  }
  else return false;
  return true;
}

/*
    if connect successed return true
    if success get time from server hasSyncTime return true
*/
bool Util::sync() {
  isCheck = true;
  if (wifiMode != DEVICE_NOT_WIFI && connect(server_addr, defaultPort) ) {
    if ( sendRequest(server_addr, resource) ) {
      if ( findDateAndTimeInResponseHeaders() ) {
        uint8_t index = dateAndTime.indexOf(' ');      // get by space
        day = dateAndTime.substring(index + 1, index + 3);
        index = dateAndTime.indexOf(' ', 7);
        month = dateAndTime.substring(index + 1, index + 4);
        index = dateAndTime.indexOf(' ', 10);
        year = dateAndTime.substring(index + 1, index + 5);
        index = dateAndTime.indexOf(':');              // get by colon
        hour  = dateAndTime.substring(index , index - 2);
        index = dateAndTime.lastIndexOf(':');
        s_min = dateAndTime.substring(index, index - 2);
        setDateTime(true);
        if (LOG)Serial.println("Success sync date-time by WEB");
        disconnect();
        return true;
      }
    }
    disconnect();
    isCheck = false;
    return true;
  }
  isCheck = false;
  return false;
}

/*
    add minute by current time,
    for sta moder every day check internet connect
    if connect is succes - restart chip
*/
void Util::addMinute() {
  minute++;
  if ( minute == 29 && wifiMode == DEVICE_STA_MODE)everyDayReboot();
  if ( minute == 60 ) {
    minute = 0;
    if ( h_count < 23) {
      h_count++;
    } else {
      h_count = 0;
      syncDay();
    }
    if (!isApConnected())restartWiFi();
  }
}

String Util::getYear() {
  return String(y_count);
}

String Util::getMonth() {
  return month_names[m_count];
}


String Util::getDay() {
  if (LOG) {
    Serial.print("Day_count: ");
    Serial.println(d_count);
  }
  return num[d_count];
}

String Util::getHour() {
  return num[h_count];
}

String Util::getFullDate() {
  String full = "[ " + getDay() + "/" + getMonth() + "/" + getYear() + " " + getHour() + ":" + String(minute) + " ]";
  return full;
}

//
//   private
//

/**
   reboot system if have web connect
*/
void Util::everyDayReboot() {
  //  check connect if success - restart
  if ( h_count == 1) {
    if (LOG)Serial.print("Every Days restart: ");
    if (sync() && hasSyncTime()) {
      if (LOG) {
        if (isFS) {
          File logFile = SPIFFS.open("/reboot.txt", "w");
          if (!logFile) {
            String fail = "Can`t write reboot";
            Serial.println(fail);
          } else {
            logFile.println(getFullDate() + String("  EveryDay chip reboot") );
            logFile.close();
          }
        }
        Serial.println(" reboot chip");
      }
      ESP.restart();
    } else {
      Serial.println("  fail reboot chip");
    }
  }
  //  check connect after fail check if success - restart
  if (hasOnlySta && !hasSyncTime() && (h_count + 1) % 7 == 0) {
    if ( restartWiFi()) {
      if (LOG)Serial.println("On Fail Day restart: try yet once restart");
      if ((sync() && hasSyncTime())) ESP.restart();
    }else{
      if(LOG)writeLog(getFullDate() + " WARNING [ module reboot on fail restart WiFi in ONLY_STA mode ]");
      ESP.restart();
    }
  }
}


/**
   whether sync date-time by web
*/
bool Util::hasSyncTime() {
  return isCheck;
}

// private

//
//  ========================  WEB ================================
//

// Open connection to the HTTP server
bool Util::connect(const char* hostName, const int port) {
  if ( LOG ) {
    Serial.print(" Connect to ");
    Serial.print(hostName);
  }
  bool ok = client.connect(hostName, port);
  // in fail connection - try yet one
  if (!ok) {
    if (LOG)Serial.print("\nreconnect ");
    unsigned long start = millis();
    int n = 0;
    while ((millis() - start) < 3000) {
      delay(20);
      // display reconnect process
      if (LOG) {
        Serial.print('.');
        n++;
        if (n % 50 == 0 ) {
          n = 0;
          Serial.println(" ");
        }
      }
    }
    if (LOG)Serial.println(" ");
    ok = client.connect(hostName, port);
  }
  if ( LOG ) Serial.println(ok ? " : Connected" : "  : Connection Failed!");
  return ok;
}

// Close the connection with the HTTP server
void Util::disconnect() {
  // if ( LOG ) Serial.println("Disconnect from HTTP server");
  client.stop();
}

// Send the HTTP GET request to the server
bool Util::sendRequest(const char* host, const char* resource) {
  if ( LOG ) {
    Serial.print("GET ");
    Serial.println(resource);
  }
  client.print("GET ");
  client.print(resource);
  client.print(" HTTP/1.1\r\n");
  client.print("Host: ");
  client.print(host);
  client.print("\r\nAccept: */*");
  client.print("\r\nConnection: close");
  client.print("\r\n\r\n");
  return true;
}


//
//  ========================= date time =====================
//

// push header parts with time
bool Util::findDateAndTimeInResponseHeaders() {
  // date and time string starts with Date: and ends with GMT
  // example: Date: Sun, 29 May 2016 10:00:14 GMT
  client.setTimeout(HTTP_TIMEOUT);
  char header[85];
  size_t length = client.readBytes(header, 85);
  header[length] = 0;
  // String headerString = String(header);
  dateAndTime = String(header).substring(
                  String(header).indexOf("Date: ") + 6, String(header).indexOf("GMT"));
  /*
    if ( LOG ) {
      Serial.print("HTTP response header ");
      Serial.println(String(header).c_str());
      Serial.print("DATE index start = ");
      Serial.print(String(header).indexOf("Date: "));
      Serial.print(";  index end = ");
      Serial.println(String(header).indexOf("GMT"));
      Serial.print("date and time: ");
      Serial.println(dateAndTime.c_str());
    }
  */
  return dateAndTime.length() > 15;
}


/* set date and time after get time
     if from web: isWeb true
     if from user: isWeb false
*/
void Util::setDateTime(bool isWeb) {
  /*
    if (LOG) {
      Serial.print( "Get date-time for decode: [  year = "); Serial.print(year);
      Serial.print( "  month = "); Serial.print(month);
      Serial.print( "  day = "); Serial.print(day);
      Serial.print( "  hour = "); Serial.print(hour);
      Serial.print( "  minutes = "); Serial.print(s_min);
      Serial.println("  ]");
    }
  */
  if (s_min.startsWith("0"))s_min = s_min.substring(1);
  minute = s_min.toInt();
  int i = 0;
  // set hour
  while ( i < 24 ) {
    if ( hour.indexOf(num[i]) > -1) {
      h_count = isWeb ? (24 + i + timezone) % 24 : (24 + i) % 24;
      hour = num[h_count];
      break;
    }
    i++;
  }
  //set day
  i = 0;
  while ( i < 32 ) {
    if ( day.indexOf(num[i]) > -1) {
      d_count = i;
      break;
    }
    i++;
  }
  // set month
  i = 0;
  while ( i < 12 ) {
    if ( month.indexOf(month_names[i]) > -1) {
      m_count = i;
      break;
    }
    i++;
  }
  // set Year
  y_count = year.toInt();
  isSetTime = true;
  /*
    if (LOG) {
      Serial.print( "after decode date: [ year ="); Serial.print(String(y_count));
      Serial.print(";   month = "); Serial.print(num[m_count]);
      Serial.print(";   day = "); Serial.print(num[d_count]);
      Serial.print(";   hour = "); Serial.print(num[h_count]);
      Serial.print(";   minute = "); Serial.print((int)minute);
      Serial.println(" ]");
    }
  */
}

void Util::syncMonth() {
  d_count = 1;
  if (m_count < 11) {
    m_count++;
  } else {
    y_count++;
    m_count = 0;
  }
}

void Util::syncDay() {
  if (d_count < 28) {
    d_count++;
    return;
  }
  if (d_count == 28 ) {
    if (m_count != 1) {
      d_count++;
      return;
    }
    if ((y_count % 4) != 0) {
      syncMonth();
      return;
    }
    d_count++;
    return;
  }
  if (d_count == 29) {
    if ((y_count % 4) == 0 && m_count == 1) {
      syncMonth();
      return;
    }
    d_count++;
    return;
  }
  if (d_count == 30 && (m_count == 3 || m_count == 5 || m_count == 8 || m_count == 10)) {
    syncMonth();
    return;
  } else if (d_count == 30) {
    d_count++;
    return;
  }
  syncMonth();
}

//
// --------  public  all -------------
//

String Util::getCurrentProps() {
  String res = "\ntime on chip(установленное на ESP время) : " + getFullDate() + "\n\nWiFi mode : ";
  //  wi-fi mode
  // ssid  AP and STA mode
  char s[15], p[15], ts[20], addr_str[20];
  char *ssidAP = s, *ssidSTA = p, *tsKey = ts, *ip = addr_str;
  s[0] = '\0';
  p[0] = '\0';
  ts[0] = '\0';
  addr_str[0] = '\0';
  res += "   WiFi (Параметры вай-фай)\n";
  switch (wifiMode) {
    case DEVICE_AP_MODE:
      res += "AP (точка доступа)";
      fillParam(AP_SSID, ssidAP); // read Start Param from file sysytem
      if (!ssidAP || ssidAP[0] == '\0') ssidAP = AP_SSID_DEF;
      fillParam(AP_IP, ip); // read Start Param from file sysytem
      if (!ip || ip[0] == '\0') ip = AP_IP_ADDR;
      res += "  ssid=" + String(ssidAP) + "  ip=" + String(ip);
      break;
    case DEVICE_STA_MODE:
      res += "STA (клиент)";
      fillParam(STA_SSID, ssidSTA);
      if (!ssidSTA || ssidSTA[0] == '\0') ssidSTA = STA_SSID_DEF;
      res += "  ssid=" + String(ssidSTA);
      res += "\n ONLY_STA (работает ли чип в режиме 'только клиент' ?) : ";
      if(isOnlySta()) res += "yes(да)";
      else res += "no(нет)";
      res += "\n address for check on suspend (адрес для проверки  работоспособности WiFi):\n   " + String(apInterfaceAddress);
      break;
    case DEVICE_NOT_WIFI:
      res += "WARNING WiFi not work!!!(вай-фай не работает)";
      break;
  }
  res += "\n\nfile system (файловая система) : ";//
  // file sysytem access
  if (hasFS()) res += " accessed (доступна) \n";
  else res += " not access (недоступна) \n";
  res += "\nwork mode(режим работы)    : ";
  // debug mode
  if (DEBUG) res += " debug (отладка)";
  else res += " default (обычный)";
  // DS18B20
  res += "\n\nwhether work DS18B20 (включен ли DS18B20 датчик?) : ";
  
  if (getDS18B20Mode()) res += " yes(да)";
  else res += " no(нет)";
  res += "\n\nThingspeak write (передача данных на Thingspeak.com) :";
  fillParam(THINGSPEAK_KEY, tsKey);
  if (!tsKey || tsKey[0] == '\0')res += " no(нет)";
  else res += " yes(да)";

  res += "\n\nlog file request(строка запроса лог файла) : weather.local/log\n";
  return res;
}

/**
   read ds18b20 mode return true if ds18b20 want be ON
*/
bool Util::getDS18B20Mode() {
  char m[10];
  m[0] = '\0';
  char* dsMode = m;
  fillParam( DS18B20_MODE, dsMode);
  return (strncmp( dsMode, "true", 4) == 0 );
}

void Util::getThingSpeakKey(char* key) {
  fillParam(THINGSPEAK_KEY, key);
  if (LOG) {
    Serial.print("Util.getThingSpeakKey =");
    Serial.println(key);
  }
}


/**
   send data to thingspeak and check answer
   if content lenght = 1 and last symbols in answer is 0 - data not reseived
   @ return 0 - success
            1 - indefinite(not response)
            2 - fail connect
            3 - server error
*/
int Util::writeDataToThingspeak(const char* host, String contentGetRequest) {
  if (connect(host, defaultPort)) {
    if ( sendRequest( host, contentGetRequest.c_str())) {
      unsigned long timeout = millis();
      while (client.available() == 0) {
        if (millis() - timeout > 6000) {
          if (LOG)Serial.println(" <<< Client Timeout >>> !");
          disconnect();
          return 1;
        }
      }
      bool isCheck = false; // if some string of header is 'Content-Length: 1' then need check last symbols on '0'
      String line = "";
      while (client.available()) {
        line = client.readStringUntil('\n');
        // if (LOG)Serial.print(line);
        if (line.startsWith("Content-Length: 1")) {
          isCheck = true;
          // if (LOG)Serial.print(" isCheck = TRUE");
        }
        // if (LOG)Serial.write('\n');
      }
      /*
        if (LOG) {
        Serial.print("Last string in answer is '");
        Serial.print(line);
        Serial.print("' \nlast line ");
        if (line.equals("0"))Serial.println("equals noll");
        else Serial.println(" not equals noll");
        }
      */
      // succes check - return 0
      if ( !isCheck || !line.equals("0")) {
        disconnect();
        return 0;
      }
    }
    disconnect();
    return 3;
  }
  return 2;
}
















