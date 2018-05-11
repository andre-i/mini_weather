
#include "SerialHandler.h"

extern bool LOG;
extern bool DEBUG;

SerialHandler ::SerialHandler(Util *u) {
  util = u;
}

void SerialHandler::setFSstate( bool fsState) {
  isFS = fsState;
}

void SerialHandler::showFile(char answ[]) {
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

void SerialHandler::showManual() {
  const char *help = (strncmp(HELP_LANG, "ru", 2) == 0) ? help_ru: help_en;
  char ch = *help;
  int i = 0;
  while(ch != '\0'){
    if(ch == '\n')Serial.println("");
    else Serial.print(ch);
    ch = *(help++);
  }
}

void SerialHandler::handle(void) {
  char answ[100];
  int i = 0;
  delay(20);
  while (Serial.available() > 0) {
    answ[i] = Serial.read();
    i++;
    delay(20);
  }
  answ[i] = '\0';
  // toggle log out state 
  if (i < 3) {
    if (answ[0] == 'y' || answ[0]  == 'Y') {
      LOG = true;
      Serial.println(" ON LOG output!");
    } else if (answ[0] == 'n' || answ[0] == 'N') {
      LOG = false;
      Serial.println(" OFF LOG output!");
    } else if (answ[0] == 's' && answ[1] == 'p') {
      isFS = util->initFS();
    } else if (answ[0] == 's' && answ[1] == 'i') {
      Serial.println(util->fsINFO());
    } else {
      showManual();
    }
  } else {
    executeCommand(answ);
  }
}

void SerialHandler::executeCommand(char answ[100]){
      //  read from file system
    if (answ[0] == 'r' && answ[1] == '_') {
      if (isFS)showFile(answ);
      else Serial.println("WARNING: Can`t access to file system");
      return;
    }
    // write or add content by file
    if ((answ[0] == 'a' || answ[0] == 'w') && answ[1] == '_') {
      if (isFS)writeToFile(answ);
      else Serial.println("WARNING: Can`t access to file system");
      return;
    }
    // add by file new line symbols
    if (answ[0] == 'n' && answ[1] == 'l' && answ[2] == '_') {
      if (isFS)appendNewLine(String(answ).substring(3));
    }
    // set debug mode on worked app
    if( strncmp(answ, "debug", 5) == 0){
      Serial.println(setDebug(String(answ).substring(5)));
    }
    // get date-time on chip
    if(strncmp(answ, "time", 4) == 0){
      Serial.print("Now on chip  ");
      Serial.println(util->getFullDate());
    }
    // get current work parameters
    if(strncmp(answ, "curr", 4) == 0){
      Serial.println("  ------- Current application parameters -------");
      Serial.println(util->getCurrentProps());
      Serial.println("  ___________  end current parameters  _________\n");
    }
    
    //  set date-time
    if (strncmp(answ, "date", 4) == 0 ) {
      if(!isFS) Serial.println("Fail set date-time (No get access on file system)");
      if (util->assignTime(answ+5)) {
        String mess = "(Ok) Success set date-time: " + util->getYear() + "/" + util->getMonth() + "/" + util->getDay() + " " + util->getHour() + "h";
        Serial.println(mess);
      } else {
        Serial.println("(No) Fail set Date-time WRONG FORMAT");
      }
    }
    //     PROPERTIES FILE
    // write start wifi params to PROPS_FILE
    if (answ[0] == 'w' && answ[1] == 'i' && answ[2] == 'f' && answ[3] == 'i') {
      if (isFS)Serial.println(writeWifiProps(String(answ).substring(4)));
      else Serial.println("WARNING: Can`t access to file system");
    }
    // write to PROPS_FILE Wi-Fi AP mode AP IP4V address
    if (answ[0] == 'a' && answ[1] == 'p' && answ[2] == 'i' && answ[3] == 'p') {
      if (isFS)Serial.println(writeApModeIpAddr(answ));
      else Serial.println("WARNING: Can`t access to file system");
    }
    // set debug mode to props file
    if( strncmp(answ, "setDebug", 7) == 0){
      if (isFS)Serial.println(writeDebugMode(String(answ).substring(9)));
      else Serial.println("WARNING: Can`t access to file system");
    }
}

bool SerialHandler::appendNewLine(String fName) {
  if (!SPIFFS.exists(fName)) {
    Serial.print("Error: file \"");
    Serial.print(fName);
    Serial.println("\" not found");
    return false;
  }
  File file = SPIFFS.open(fName, "a");
  if (!file) {
    Serial.print("Error: can`t open file \"");
    Serial.print(fName);
    Serial.println("\" !");
    return false;
  }
  file.print("\n");
  file.close();
  Serial.print("Append new line to: ");
  Serial.println(fName);
  return true;
}

bool SerialHandler::writeToFile(char req[]) {
  String all = req;
  int ind = all.indexOf("=");
  if (ind < 1) {
    Serial.println("Error: bad request format, it must contain \"=\"(equals sign) !");
    return false;
  }
  String fName = all.substring(2, ind);
  String toWrite = all.substring(++ind);
  File file = SPIFFS.open(fName, (req[0] == 'w') ? "w" : "a");
  if (file) {
    Serial.print("Write to File: ");
    Serial.println(fName);
    if (toWrite.length() < 1) {
      file.close();
      SPIFFS.remove(fName);
      Serial.print(" Delete file: ");
      Serial.println(fName);
      return true;
    }
    file.print(toWrite.c_str());
    file.close();
    Serial.println(toWrite);
    Serial.println("  ______  success write and closed file ________  ");
    return true;
  } else {
    Serial.print( "Can`t write to file: ");
    Serial.println(fName);
    return false;
  }

}

/**
   write wi-fi properties:
   format:
   wifi ap [ssid password] sta [ssid password]
*/
String SerialHandler::writeWifiProps(String prop) {
  prop.trim();
  struct params wParam = getParamsFromFile();
  if (fillPropsFromString(prop, &wParam)) {
    if (LOG)Serial.println("FAIL get WiFi params. ");
    util->writeLog(String("FAIL write Wifi parameters - Wrong format for string[") + prop + String("]"));
    return "WARNING - can`t write wifi params";
  }
  writeToPropFile(wParam);
  return String("Ok");
}

/**
   parse prop string and get from it wi-fi parameters
   if any error occured - set params.isErr to true and return params
   @return struct params witch filled wifi properties,
     if properties(ap or sta) missing in prop string - appropriate property set to empty string
*/
bool SerialHandler::fillPropsFromString(String prop, struct params *par) {
  //if(LOG)Serial.println(String("wifi for set: ") + prop);
  bool isErr = false;
  String ap_ssid = "", ap_passwd = "", sta_ssid = "", sta_passwd = "";
  String buf = "";
  int sepInd;
  int staInd = prop.indexOf("sta");
  int apInd = prop.indexOf("ap");
  if ( apInd >= staInd && staInd > -1) {
    Serial.println("Wrong parameters for WiFi: ap params - first, sta params - second");
    return isErr = true;
  }
  if (apInd >= 0) {
    //if (LOG)Serial.print("Try get ap params. ");
    buf = prop.substring(prop.indexOf('[') + 1, prop.indexOf(']'));
    buf.trim();
    if ( buf.length() > 10 ) {
      sepInd = buf.indexOf(' ');
      if (sepInd > 2) {
        ap_ssid = buf.substring(0, sepInd);
        ap_passwd = buf.substring( buf.lastIndexOf(' ') + 1);
      } else {
        isErr = true;
      }
    } else {
      isErr = true;
    }
    if (isErr) {
      Serial.println( "Can`t write AP parameters - it missed or wrong");
      return isErr;
    }
  }
  if ( staInd >= 0 ) {
    //if (LOG)Serial.print("Try get sta params. ");
    buf = prop.substring(prop.lastIndexOf('[') + 1, prop.lastIndexOf(']'));
    buf.trim();
    if (buf.length() > 10) {
      sepInd = buf.indexOf(' ');
      if (sepInd > 2) {
        sta_ssid = buf.substring(0, sepInd);
        sta_passwd = buf.substring( buf.lastIndexOf(' ') + 1);
      } else {
        isErr = true;
      }
    } else {
      isErr = true;
    }
    if (isErr) {
      Serial.println( "Can`t write STA parameters - it missed or wrong");
      return isErr;
    }
  }
  if (LOG) {
    Serial.println(" Success get wifi params!");
    Serial.println(String("\tAP [ssid:") + String(ap_ssid) + String(" passwd:") + String(ap_passwd) +
                   String("] \n\tSTA [ssid:") + String(sta_ssid) + String("  passwd:") + String(sta_passwd) + String("] "));
  }
  if (!ap_ssid.equals(""))par->ap_ssid = ap_ssid;
  if (!ap_passwd.equals(""))par->ap_passwd = ap_passwd;
  if (!sta_ssid.equals(""))par->sta_ssid = sta_ssid;
  if (!sta_passwd.equals(""))par->sta_passwd = sta_passwd;
  return isErr;
}

struct SerialHandler::params SerialHandler::getParamsFromFile() {
  struct params par;
  if (SPIFFS.exists(PROPS_FILE)) {
    // if (LOG)Serial.println(String("try get param: ") + String(parName));
    char cur;
    bool flag = false;
    bool isReadProp = false;
    char sec[3];
    int i = 0;
    String res = "";
    File params = SPIFFS.open(PROPS_FILE, "r");
    if (!params) par;
    if (LOG)Serial.println("parse props.txt");
    while ( params.available() ) {
      cur = params.read();
      if (cur == ' ')continue;
      if (cur = '#')flag = false;
      if (cur == '\n') {
        if (isReadProp) {
          if (strncmp(sec, AP_SSID, 2) == 0)par.ap_ssid = res;
          if (strncmp(sec, AP_PASSWD, 2) == 0)par.ap_passwd = res;
          if (strncmp(sec, AP_IP, 2) == 0)par.ap_ip = res;
          if (strncmp(sec, STA_SSID, 2) == 0) par.sta_ssid = res;
          if (strncmp(sec, STA_PASSWD, 2) == 0) par.sta_passwd = res;
          if (strncmp(sec, DEBUG_MODE, 2) == 0) par.isDebug = res;
          res = "";
          isReadProp = false;
        }
        flag = true;
      } else if (flag) {
        if (i == 1) {
          // if (LOG)Serial.println(String("check param") + String(*parName) + String(*(parName + 1)) + String(" with: ") + String(sec[0]) + String(cur));
          sec[1] = cur;
          sec[2] = '\0';
          isReadProp = true;
          i = 0;
          flag = false;
        } else {
          sec[i] = cur;
          i++;
        }
      } else if (isReadProp) {
        res += cur;
      }
    }
    params.close();
    //if (LOG)Serial.println(String("On end check have param = ") + res);
  }
  return par;
}

void SerialHandler::writeToPropFile( struct params par) {
  File file = SPIFFS.open(PROPS_FILE, "w");
  if (!file) {
    Serial.println("WARNING: Can`t access to fileSystem, WiFi properties do not written!");
  }
  String res = "## property file contain properties for Wi-Fi in AP and STA mode\n";
  res += "# AP mode\n";
  if (par.ap_ssid && par.ap_ssid.length() > 2)res += String(AP_SSID) + " " + par.ap_ssid + "\n";
  if (par.ap_passwd && par.ap_passwd.length() > 6)res += String(AP_PASSWD) + " " + par.ap_passwd + "\n";
  if (par.ap_ip && par.ap_ip.length() > 6)res += String(AP_IP) + " " + par.ap_ip + "\n";
  res += "# STA mode\n";
  if (par.sta_ssid && par.sta_ssid.length() > 2)res += String(STA_SSID) + " " + par.sta_ssid + "\n";
  if (par.sta_passwd && par.sta_passwd.length() > 6)res += String(STA_PASSWD) + " " + par.sta_passwd + "\n";
  res += "# debug mode\n";
  if (par.isDebug && par.isDebug.length() > 3)res += String(DEBUG_MODE) + " " + par.isDebug + "\n";
  file.print( res.c_str());
  file.close();
  Serial.println("Success : Wi-Fi parameters write to property file");
  util->writeLog(String(util->getDay()) + "/" + String(util->getMonth()) + "/" + String(util->getYear()) + " - writen new properties ");
}


String SerialHandler::writeApModeIpAddr(char *addr) {
  int n = 0;
  while (*(addr + n) != ' ')n++;
  n++;
  char *ip = (addr + n);
  int res[4];
  if (LOG)Serial.println(String(" IP address = ") + String(ip));
  util->parseAddr(ip, res);
  if (res[0] == -1)return String("ERROR - wrong IP address - \"") + ip + String("\" !");
  struct params par = getParamsFromFile();
  par.ap_ip = String(ip);
  writeToPropFile( par);
  return "Ok";
}


String SerialHandler::writeDebugMode(String isDebug){
  isDebug.trim();
  if(!isDebug.equals("true") && !isDebug.equals("false"))return String("Wrong debug mode  - \"") + isDebug + String("\"! it may be only 'true' or 'false'");
  struct params par = getParamsFromFile();
  par.isDebug = isDebug;
  writeToPropFile( par );
  return "Ok"; 
}



String SerialHandler::setDebug(String mode){
  String res = " DEBUG ";
  mode.trim();
  if(mode.equals("y") || mode.equals("Y")){
    res += "ON";
    DEBUG = true;
    LOG = true;
  }else{
    res += "OFF";
    DEBUG = false;
    LOG = false;
  }
  return res;
}




