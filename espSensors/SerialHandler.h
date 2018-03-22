#ifndef SerialHandler_h
#define SerialHandler_h

#include <Arduino.h>
#include <FS.h>

#include "props.h"
#include "Util.h"


class SerialHandler {
  public:
    SerialHandler(Util *u);
    //void setDebug(bool isDebug);
    void handle(void);
    void setFSstate( bool isFS);


  private:
    Util *util;
    bool isFS = false;
    // read
    void showManual();
    void showFile(char answ[]);
    // write
    bool writeToFile(char req[]);
    bool appendNewLine(String fName);
};


#endif
