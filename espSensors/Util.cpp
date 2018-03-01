#include "Util.h"

extern bool DEBUG;


Util::Util(bool isDebug) {
  DEBUG = isDebug;
  //  init file system
  isFS = (SPIFFS.begin()) ? true : false;
}

void Util::setDebug(bool isDebug) {
  DEBUG = isDebug;
}
//  =================== work with init params  ====================

/**
   try read WiFi init param from file,
   if not , then set to param '\0' - null string

   parName - name parameter in props file( see for help props.h file)
   dest - string for write parameter value
*/
void Util::fillParam(char *parName, char *dest) {
  if (DEBUG)Serial.println(String("Try get value for: ") + String(parName));
  File file;
  if (SPIFFS.exists(PROPS_FILE)) {
    file =  SPIFFS.open(PROPS_FILE, "r");
    if (!file) {
      dest[0] = '\0';;
      if (DEBUG)Serial.println(" Can`t open PROPS_FILE - fill wi-fi params with default values");
      return;
    }
  } else {
    dest[0] = '\0';
    if (DEBUG)Serial.println("PROPS_FILE not found - fill wi-fi param with default value");
    return;
  }
  char res[31];
  int i = 0; // char number in file
  int n = 0; // char number in string
  char c;
  for ( i = 0;  file.available(); i++) {
    c = file.read();
    if ( c != '\n' && n < 20 ) {
      res[n] = c;
    } else {
      res[n] = '\0';
      if (res[0] != '#') { // comment not parse
        if (parName[0] == res[0] && parName[1] == res[1]) {
          if (DEBUG) Serial.println(String("for parse : ") + String(res));
          setStartValue(res + 2, dest);
          file.close();
          return;
        }
      }
      n = -1;
    }
    n++;
  }
  file.close();

}

/**
   read param value from *val(delete all white space) and copy it to *buf
*/
void Util::setStartValue(char *val, char *buf) {
 // if (DEBUG)Serial.println(String("GET FOR parse: ") + String(val));
  int i = 0, n = 0;
  while (val[i] != '\0' && n < 10) {
    if (val[i] != ' ') {
      buf[n] = val[i];
      n++;
    }
    i++;
  }
  //n++;
  buf[n] = '\0';
  if (DEBUG)Serial.println(String("read param :") + String(buf));
}

// ==================== SPIFFS =====================

//  public :
bool Util::initFS() {
  if (DEBUG)Serial.print("init SPIFFS : ");
  if (isFS) {
    Serial.println(" already DONE");
    return isFS;
  }
  if (SPIFFS.begin()) {
    Serial.println("success");
    isFS = true;
    return isFS;
  } else {
    Serial.println("fail!  WARNING \"esp_server can`t write data to storage\"");
    return false;
  }
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
      return;
    }
    logFile.println(mess);
    logFile.close();
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
  if (DEBUG)Serial.println((String("periodsAsJSON: ") + files));
  return files;

}

// =============== write sensors values
//
bool Util::hasWrite() {
  return isFS && isSetTime;
}

void Util::writeSensorsValues(int tIn, int tOut, int baro, int humid) {
  if (DEBUG)Serial.println("Call write sensors values");
  String fullMonthFileName = sensorDataDir + pathSeparator + year + pathSeparator + month + ".txt";
  if (DEBUG) {
    Serial.print(" Try write data to file \"");
    Serial.println( fullMonthFileName);
  }
  File monthF = SPIFFS.open(fullMonthFileName, "a");
  if (DEBUG)Serial.println("try open month file");
  if (!monthF) {
    writeLog("Failed to open : " + fullMonthFileName);
    monthF.close();
    return;
  }
  String output = getDay() + valSeparator + getHour() + valSeparator + tIn + valSeparator + tOut + valSeparator + baro + valSeparator + humid;
  if (DEBUG) {
    Serial.print("Try Write sensors valuers to file: ");
    Serial.print(fullMonthFileName);
    Serial.print(" [ ");
    Serial.print(output);
    Serial.println(" ]");
  }
  monthF.println();
  monthF.print(output);
  monthF.close();
  if (DEBUG) Serial.println( " Success write data to file!");
}

File Util::openFileToRead(const char *fName) {
  if (!opened && SPIFFS.exists(fName)) {
    opened = SPIFFS.open(fName, "r");
    if (opened) {
      return opened;
    } else opened.close();
  }
  return SPIFFS.open("directory/not/found/FILE_NOT_FOUND", "r");
}

void Util::closeFile() {
  if (opened)opened.close();
}

bool Util::hasFS() {
  return isFS;
}
//    private:


//
//  ===============  Wi-Fi  =======================
//

//  public

int Util::initWIFI() {
  // read StartParams from file sysytem
  wifiMode = DEVICE_NOT_WIFI;
  if ( isStaConnect()) {
    Serial.println("work as STA ");
    wifiMode = DEVICE_STA_MODE;
  } else {
    if ( isSetApMode() ) {
      Serial.println("work as AP");
      wifiMode = DEVICE_AP_MODE;
    }
  }
  return wifiMode;
}

// private

// return true if conneted to wifi
bool Util::isStaConnect() {
  char s[15], p[15];
  s[0] = '\0';
  p[0] = '\0';
  char *ssid = s, *passwd = p;
  fillParam(STA_SSID, ssid);
  if (!ssid || ssid[0] == '\0') ssid = STA_SSID_DEF;
  fillParam(STA_PASSWD, passwd);
  if (!passwd || passwd[0] == '\0')passwd = STA_PASSWD_DEF;
  WiFi.begin(ssid, passwd);
  // Wait for connection
  int counter = 0;
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
    counter ++;
    if ( counter > 15 ) {
      WiFi.disconnect();
      delay(100);
      if (DEBUG)Serial.println(String("Can`t connect to ") + String(ssid) + String("  ") + String(passwd));
      return false;
    }
  }
  Serial.println("start as STA ");
  if (DEBUG) {
    Serial.print("Connected to ");
    Serial.print(ssid);
    Serial.print("  IP address: ");
    Serial.println(WiFi.localIP());
  }
  return true;
}

// return true if success up AP mode on chip
bool Util::isSetApMode() {
  // If AP mode
  char s[15], p[15];
  s[0] = '\0';
  p[0] = '\0';
  char *ssid = s, *passwd = p;
  fillParam(AP_SSID, ssid);
  if (!ssid || ssid[0] == '\0') ssid = AP_SSID_DEF;
  fillParam(AP_PASSWD, passwd);
  if (!passwd || passwd[0] == '\0')passwd = AP_PASSWD_DEF;
  if (WiFi.softAP(ssid, passwd)) {
    WiFi.softAPConfig(ap_ip, ap_ip, subnet);
    if (DEBUG) {
      Serial.print("start WI-FI AP MODE: ssid=");
      Serial.print(ssid);
      Serial.print("  passwd=");
      Serial.println(passwd);
    } else Serial.println("Start Wi-Fi as AP");
    return true;
  } else {
    Serial.println("ERROR esp_server can`t UP wifi AP");
    if (DEBUG)Serial.println(String("\t param : ") + String(ssid) + String(" ") + String(passwd));
    return false;
  }
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

bool Util::sync() {
  isCheck = true;
  if (wifiMode != DEVICE_NOT_WIFI && connect(server_addr, serverPort) ) {
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
        if (DEBUG)Serial.println("Success sync date-time by WEB");
      }
    }
    disconnect();
    isCheck = false;
    return true;
  }
  isCheck = false;
  return false;
}

void Util::addMinute() {
  minute++;
  if ( minute == 60 ) {
    minute = 0;
    if ( h_count < 23) {
      h_count++;
    } else {
      h_count = 0;
      syncDay();
      sync();
    }
  }
}

String Util::getYear() {
  return String(y_count);
}

String Util::getMonth() {
  return month_names[m_count];
}


String Util::getDay() {
  if (DEBUG) {
    Serial.print("Day_count: ");
    Serial.println(d_count);
  }
  return num[d_count];
}

String Util::getHour() {
  return num[h_count];
}



//
//   private
//


/**
   whether sync date-time by web
*/
bool Util::hasSyncTime() {
  return isCheck;
}

// private

// Open connection to the HTTP server
bool Util::connect(const char* hostName, const int port) {
  if ( DEBUG ) {
    Serial.print("Connect to ");
    Serial.println(hostName);
  }
  bool ok = client.connect(hostName, port);
  if ( DEBUG ) Serial.println(ok ? "Connected" : "Connection Failed!");
  return ok;
}

// Send the HTTP GET request to the server
bool Util::sendRequest(const char* host, const char* resource) {
  if ( DEBUG ) {
    Serial.print("GET ");
    Serial.println(resource);
  }
  client.print("GET ");
  client.print(resource);
  client.println(" HTTP/1.1");
  client.print("Host: ");
  client.println(host);
  client.println("Accept: */*");
  client.println("Connection: close");
  client.println();
  return true;
}

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
    if ( DEBUG ) {
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

// Close the connection with the HTTP server
void Util::disconnect() {
  // if ( DEBUG ) Serial.println("Disconnect from HTTP server");
  client.stop();
}

/* set date and time after get time
     if from web: isWeb true
     if from user: isWeb false
*/
void Util::setDateTime(bool isWeb) {
  /*
    if (DEBUG) {
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
    if (DEBUG) {
      Serial.print( "after decode date: [ year ="); Serial.print(String(y_count));
      Serial.print(";   month = "); Serial.print(num[m_count]);
      Serial.print(";   day = "); Serial.print(num[d_count]);
      Serial.print(";   hour = "); Serial.print(num[h_count]);
      Serial.print(";   minute = "); Serial.print((int)minute);
      Serial.println(" ]");
    }
  */
}


// ================== check and set date-time  methods ===========

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













