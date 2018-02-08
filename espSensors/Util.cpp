#include "Util.h"

extern bool DEBUG;



Util::Util(bool isDebug) {
  DEBUG = isDebug;
}

void Util::setDebug(bool isDebug) {
  DEBUG = isDebug;
  if (isDebug)isNeedWrite = true;
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
    if (logFile) {
      logFile.println(mess);
      logFile.close();
      return;
    }
  }
  String fail = "Can`t write to \"" + String(LOG_FILE) + "\" \nmessage: [ " + mess + " ]";
  Serial.println(fail);
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
  while(dataDir.next())files += " \"" + dataDir.fileName() + "\",";
  int last = files.lastIndexOf(",");
  if(last > 0) files = files.substring(0, last) + " ]";
  else files = "[ ]";
  if(DEBUG)Serial.println((String("periodsAsJSON: ") + files));
  return files;
  
}

// =============== write sensors values
//
bool Util::hasWrite() {
  return isNeedWrite;
}

void Util::doWriteToSD() {
  isNeedWrite = false;
}

void Util::writeSensorsValues(int tIn, int tOut, int baro, int humid) {
  if (DEBUG)Serial.println("Call write sensors values");
  isNeedWrite = false;
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
  //  Wi-Fi
  if (DEBUG)Serial.println("Start init WI-FI");
  wifiMode = DEVICE_NOT_WIFI;
  if ( isStaConnect()) {
    Serial.println("work as STA ");
    wifiMode = DEVICE_STA_MODE;
  } else {
    if ( isSetApMode() ) {
      Serial.println("work as AP");
      wifiMode = DEVICE_AP_MODE;
    }
    else Serial.println("\nWARNING: WIFI_CAN`T_START  ( wifi down)\n\n");

  }
  return wifiMode;
}

// private

// return true if conneted to wifi
bool Util::isStaConnect() {
  char *res = getStaParams();
  if (DEBUG)Serial.println(res);
  if (strlen(res) > 4) {
    char *part = strtok(res, "=");
    if (part != NULL) {
      staSSID = part;
      part = strtok(NULL, "=");
      if (part != NULL)staPASSWD = part;
    }
  }
  WiFi.begin(staSSID.c_str(), staPASSWD.c_str());
  // Wait for connection
  int counter = 0;
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
    counter ++;
    if ( counter > 15 ) {
      WiFi.disconnect();
      delay(100);
      return false;
    }
  }
  Serial.println("start as STA ");
  if (DEBUG) {
    Serial.print("Connected to ");
    Serial.print(STA_SSID);
    Serial.print("  IP address: ");
    Serial.println(WiFi.localIP());
  }
  return true;
}

// return true if success up AP mode on chip
bool Util::isSetApMode() {
  // If AP mode
  if (WiFi.softAP(AP_SSID, AP_PASSWD)) {
    WiFi.softAPConfig(ap_ip, ap_ip, subnet);
    Serial.println("start WI-FI AP MODE");
    return true;
  }
  Serial.println("ERROR esp_server can`t UP wifi");
  return false;
}

char* Util::getStaParams() {
  char *c = "";
  File staPar;
  if (SPIFFS.exists(STA_PARAMS_FILE)) {
    staPar =  SPIFFS.open(STA_PARAMS_FILE, "r");
  } else return c;
  char res[staPar.size() + 1];
  for (uint32_t i = 0;  staPar.available(); i++) {
    res[i] = staPar.read();
  }
  res[staPar.size()] = '\0';
  if (DEBUG)Serial.println("sta params file: " + String(res));
  char* l = strtok(res, "\n");
  if (l != NULL)l = strtok(NULL, "\n");
  return (l != NULL) ? l : c;
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
    isNeedWrite = true;
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













