#ifndef Sensors_h
#define Sensors_h

#include "Arduino.h"
#include "props.h"

#include "DHT.h"
#include <Wire.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BMP280.h>
#include <OneWire.h>
#include <DallasTemperature.h>


class Sensors {
  private:
    // bool debug;
    // memebers
    OneWire *oneWire;
    DallasTemperature *sensors;
    DHT *dht;
    Adafruit_BMP280 *bme;
    //  max number members of values array init with -1 
    int maxCounterNumber = - 1;
    //  media
    int mediaIn, mediaOut, mediaBaro, mediaHumid;
    //  numbers of last measurent value in values array
    int lastDS18B20 = REQUEST_COUNT, lastDHT = REQUEST_COUNT, lastBMP280 = REQUEST_COUNT;
    int tIn, tOut, humid, baro;
    int dsTemp = -200;
    String currentInJSON = "";
    String allIn[REQUEST_COUNT] , allOut[REQUEST_COUNT] , allHumid[REQUEST_COUNT], allBaro[REQUEST_COUNT];
    //  methods
  public:
    //methods
    Sensors(uint8_t wBus, int dhtType);
    int init();// if success return 0, else 1
    void readDHT();
    void readDS18B20();
    void readBMP280();
    void makeCurrentToJSON();
    String getCurrentAsJSON();
    String getLastAsJSON(String sensorDataType);
    int getMedia(String sensorDataType);
    
};


#endif

