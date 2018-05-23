#ifndef Util_h
#define Util_h

#include <Arduino.h>

#include "props.h"
#include <FS.h>
#include <ESP8266WiFi.h>
#include <WiFiClient.h>

#ifndef IPAddress_h
#define IPAddress_h
#endif

//const IPAddress ap_ip(AP_IP_ADDR);
const IPAddress subnet(AP_IP_SUBNET);

/*      SD card pins
 ** CLK - pin 14
 ** MISO - pin 12
 ** MOSI - pin 13
 **  CS - set on call initFS(cs_pin) - props.h set it to 16.
*/


class Util {
  private:
    // bool LOG;
    // ========= SPIFFS =========
    // variables
    const String sensorDataDir = SENSOR_DATA_DIR;
    const char *pathSeparator = "/";
    const char *valSeparator = SEP;
    File opened;
    bool isFS = false;      
    //methods
    void fillParam(const char *key_for_parameter, char *destination);

    // ===== ThingSpeak =====
    char* writeApiKey;
    bool isThingSpeak = false;
    long oldLen = 0;

    // =======  Wi-Fi =======
    // variables
    bool isStaConnect();
    bool isSetApMode();
    WiFiClient client;

    // ======= date time =========
    int wifiMode = DEVICE_NOT_WIFI;
    String year;
    String month;
    String day;
    String hour;
    String s_min;
    bool isCheck = false; // true if at time try get date-time from web
    bool isSetTime = false; // has set time 
    // date-time params =====
    int y_count = 0;
    uint8_t m_count = 0, /* month_counter*/
            d_count = 0,
            h_count = 0;
    int timezone = 3;
    int minute = -1;
    // 0"Jan 31", 1"Feb28-29", 2"Mar31", 3"Apr30", 4"May31", 5"Jun30", 6"Jul31", 7"Aug31", 8"Sep30", 9"Oct31",10"Nov30", 11"Dec31"
    const String month_names[12] = {"Jan", "Feb", "Mar", "Apr", "May", "Jun", "Jul", "Aug", "Sep", "Oct", "Nov", "Dec"};
    const char* num[32] = {
      "00", "01", "02", "03", "04", "05", "06", "07", "08", "09",
      "10", "11", "12", "13", "14", "15", "16", "17", "18", "19",
      "20", "21", "22", "23", "24", "25", "26", "27", "28", "29",
      "30", "31"
    };
    // ============== Get Data-Time From internet ====
    const unsigned long HTTP_TIMEOUT = 10000;   // max respone time from server
    const int defaultPort = 80;                  // a port number
    const char* resource = "/";                // http resource
    const char* server_addr = REQUEST_DATE_URL; // server for check time
    String dateAndTime;

    // methods
    bool connect(const char* host, const int port);
    bool sendRequest(const char* host, const char* resource);
    void disconnect();
    bool findDateAndTimeInResponseHeaders();
    void setDateTime(bool isWeb);
    void syncMonth();
    void syncDay();

  public:
    Util(bool isDebug);
    void setDebug(bool isDebug); // set debug mode to util class
    bool getDebugMode(); // return debug mode from PROPS_FILE(if it not found - return false)
    //  SPIFFS
    String fsINFO();
    String getPeriodsAsJSON();
    bool initFS();
    void writeLog(String mess);
    void writeSensorsValues(int tIn, int tOut, int baro, int humid); 
    bool hasFS();
    //  WI-FI
    int initWIFI(); // try up wifi inSTA or AP mode
    void parseAddr(char *ip, int addr[4]); // fill addr numbers from IP4V address
    // date time
    bool assignTime(char* userTime);
    bool hasSyncTime();
    void addMinute();
    bool sync(); //  request web to get current date-time
    String getYear();
    String getMonth();
    String getDay();
    String getHour();
    String getFullDate();
    // whether write sensors values to SD card
    bool hasWrite();
    // thingspeak service
    void getThingSpeakKey(char* key);
    int writeDataToThingspeak(const char* host, String getRequest);
    //
    String getCurrentProps();
};


#endif

