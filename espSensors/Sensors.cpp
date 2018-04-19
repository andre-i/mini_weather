#include "Sensors.h"

extern bool LOG;

//  =========== PUBLIC ==================

/** set class variables for sensors */
Sensors::Sensors(uint8_t wBus, int dhtType) {
  for (int i = 0; i < maxCounterNumber ; i++) {
    allIn[i] = allOut[i] = allBaro[i] = allHumid[i] = "-100";
  }
  tIn = tOut = baro = humid = DEVICE_DISCONNECTED_C;
  mediaIn = mediaOut = mediaBaro = mediaHumid = 0;
  oneWire = new OneWire(wBus);
  if (IS_DS18B20)sensors = new DallasTemperature(oneWire);
  dht = new DHT(wBus, dhtType);
  bme = new Adafruit_BMP280();
}

/** Try prepare sensors */
int Sensors::init() {
  maxCounterNumber = REQUEST_COUNT - 1;
  // ds18b20
  if (IS_DS18B20)sensors->begin();
  if (IS_DS18B20)Serial.println("Init ds18b20");
  // dht 11
  dht->begin();
  if (LOG)Serial.println("Init DHT");
  //  BMP 280
  if (!bme->begin(0x76)) {
    Serial.println("Could not find a valid BMP280 sensor, check wiring!");
    return 1;
  } else {
    if (LOG)Serial.println("Init BMP280");
  }
  return 0;
}

//  request sensors, compute media , fill arrays of values

// ds 18B20 purpose comparison with themperature other sensors
void Sensors::readDS18B20() {
  if (IS_DS18B20) {
    //if (++lastDS18B20 > maxCounterNumber)lastDS18B20 = 0;
    sensors->requestTemperatures(); // Send the command to get temperatures
    dsTemp = (int)(sensors->getTempCByIndex(0) * 10 + 0.5) / 10;
  }
}

void Sensors::readDHT() {
  if (++lastDHT > maxCounterNumber)lastDHT = 0;
  tIn = (int)(dht->readTemperature() + 0.5);
  humid = (int)(dht->readHumidity() + 0.5);
  if ( isnan(tIn) || isnan(humid)) {
    if (LOG)Serial.println("Can`t read DHT sensor data");
    tIn = humid = DEVICE_DISCONNECTED_C;
  }
  allIn[lastDHT] = String(tIn);
  allHumid[lastDHT] = String(humid);
  if (lastDHT == 0) {
    mediaIn = tIn;
    mediaHumid = humid;
  } else {
    mediaIn = (int)((mediaIn + tIn) / 2 + 0.5);
    mediaHumid = (int) ((mediaHumid + humid) / 2 + 0.5);
  }
}

void Sensors::readBMP280() {
  if (++lastBMP280 > maxCounterNumber) lastBMP280 = 0;
  baro = (int)(bme->readPressure() / 133.32239F + 0.5);
  allBaro[lastBMP280] = String(baro);
  if (lastBMP280 == 0)mediaBaro = baro;
  else mediaBaro = (int)((mediaBaro + baro) / 2 + 0.5);
  tOut = bme->readTemperature() - 1;
  allOut[lastBMP280] = String(tOut);
  if (lastBMP280 == 0)mediaOut = tOut;
  else mediaOut = (int)((mediaOut + tOut) / 2 + 0.5);

}



//  get sensors values
void Sensors::makeCurrentToJSON() {
  currentInJSON = String("{ \"tIn\":") + String(tIn) + String(", \"tOut\":") + String(tOut) +
                  String(", \"baro\":") + String(baro) + String(", \"humid\":") + String(humid) + String(" }");
}

String Sensors::getCurrentAsJSON() {
  if (LOG) {
    Serial.print("\"getCurrentAsJSON\": ");
    Serial.print(currentInJSON);
    String res = " [ BMP280 t= " + String(tOut);
    if(IS_DS18B20) res += " Dallas t= " + String(dsTemp) + " ]";
    Serial.println(res);
  }
  return currentInJSON;
}


String Sensors::getLastAsJSON(String dataType) {
  String *type;
  int len = maxCounterNumber, last ;
  String res = "{ \"" + dataType + "\" : [ ";
  if (dataType == T_IN ) {
    type = allIn;
    last = lastDHT;
  }
  else if (dataType == HUMID) {
    type = allHumid;
    last = lastDHT;
  }
  else if (dataType == T_OUT) {
    type = allOut;
    last = lastBMP280;
  }
  else if (dataType == BARO) {
    type = allBaro;
    last = lastBMP280;
  }
  else {
    res = (String("{ \"ERROR\":[\"Not have sensors for ") + String(dataType) + String(", \" \" ], \"last\":0 }")) ;
    len = 0;
  }
  if ( len > 0 ) {
    for ( int i = 0; i < len; i++) {
      res += *(type + i) + ", ";
    }
    res += *(type + len) + "], \"last\": " + String(last) + " }";
  }
  if (LOG) {
    Serial.print("\"getLastAsJSON return: ");
    Serial.println(res);
    Serial.println("------------\n");
  }
  return res;
}


int Sensors::getMedia(String dataType) {
  if (dataType == T_IN)return mediaIn;
  if (dataType == T_OUT)return mediaOut;
  if (dataType == BARO) return mediaBaro;
  if (dataType == HUMID)return mediaHumid;
  return  DEVICE_DISCONNECTED_C;
}

// ============= PRIVATE ===================









