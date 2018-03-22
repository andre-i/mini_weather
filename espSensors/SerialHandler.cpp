
#include "SerialHandler.h"

extern bool DEBUG;


SerialHandler ::SerialHandler(Util *u){
  util = u;
}

void SerialHandler::setFSstate( bool fsState){
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
  Serial.println(" Console command:\n1) DEBUG : \n\t\"y\" - ON  \n\t\"n\" - OFF ");
  Serial.println("2) Check to work with SPIFFS :\n\t\"sp\" - print whether available SPIFFS");
  Serial.println("3) SPIFFS : \n\tread : r_/full/path/with/fileName.ext  where:\n\t  r_  - is signal on print file(or directory) content \n\t  /full/path/with/fileName.ext - fullFile name");
  Serial.println("\twrite : w_/full/path/with/fName.ext=after \"equals\" sign be char sequense for write");
  Serial.println("\tappend : a_/full/path/with/fName.ext=after \"equals\" sign be char sequense for append");
  Serial.println("\t\tWARNING if after \"=\" nothing to set then file be removed!");
  Serial.println("\t append new_line : nl_/full/path/with/fName.ext");
  Serial.println("\t\tWARNING: if after equals sign to be nothing, then file be REMOVE !");
  Serial.println("  NOTE in /props.txt file first simbols in string have meaning\n\ta)'#' - it is comment \n\tb)'ss' - STA ssid, 'sp' - STA password, 'as' - AP ssid, 'ap' - AP password \n\tc) record format first simbol(s)+white space+value");
  Serial.println("4) See SPIFFS info : \n\t\"si\" - print info about SPIFFS of chip");
  Serial.println("5) Set date/time - enter it in format:\"year/month/day/hour/minute\"");
  Serial.println("\tWARNING - date must be strongly follow format, else date be break");
  Serial.println("\tyear - 4 numbers\n\t\tmonth - 3 chars( first must be Upper Case)\n\t\tday, hour and minute consist from 2 numbers, if value less than 10 firs must be 0(null)");

}

void SerialHandler::handle(void) {
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
      isFS = util->initFS();
    } else if (answ[0] == 's' && answ[1] == 'i') {
      Serial.println(util->fsINFO());
    } else {
      showManual();
    }
  } else {
    if (answ[0] == 'r' && answ[1] == '_') {
      if(isFS)showFile(answ);
      else Serial.println("WARNING: Can`t access to file system");
      return;
    }
    if((answ[0] == 'a' || answ[0] == 'w') && answ[1] == '_'){
      if(isFS)writeToFile(answ);
      else Serial.println("WARNING: Can`t access to file system");
      return;
    }
    if(answ[0] == 'n' && answ[1] == 'l' && answ[2] == '_'){
      if(isFS)appendNewLine(String(answ).substring(3));
    }
    //  set date-time
    if (answ[0] == '2' && answ[1] == '0' ) {
      if (util->assignTime(r)) {
        String answ = "(Ok) Success set date-time: " + util->getYear() + "/" + util->getMonth() + "/" + util->getDay() + " " + util->getHour() + "h";
        Serial.println(answ);
      } else {
        Serial.println("(No) Fail set date-time (No get access on file system)");
      }
    }
  }

}

bool SerialHandler::appendNewLine(String fName){
  if(!SPIFFS.exists(fName)){
    Serial.print("Error: file \"");
    Serial.print(fName);
    Serial.println("\" not found");
    return false;
  }
  File file = SPIFFS.open(fName, "a");
  if(!file){
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

bool SerialHandler::writeToFile(char req[]){
  String all = req;
  int ind = all.indexOf("=");
  if(ind < 1){
    Serial.println("Error: bad request format, it must contain \"=\"(equals sign) !");
    return false;
  }
  String fName = all.substring(2,ind);
  String toWrite = all.substring(++ind);
  File file = SPIFFS.open(fName,(req[0] == 'w') ? "w" : "a");
  if(file){
    Serial.print("Write to File: ");
    Serial.println(fName);
    if(toWrite.length()<1){
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
  }else {
    Serial.print( "Can`t write to file: ");
    Serial.println(fName);
    return false;
  }
  
}








