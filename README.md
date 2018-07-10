# mini_weather
small weather station base on esp8266 read themperature, humidity and pressure from sensors save it and ship on WiFi by client.
The client may be any modern browser with support SVG graphics.
Consist from 2 part:
  1. viring(c++) code base on arduino IDE - work on ESP8266.
  2. HTML + JavaScript for reflect sensors data in browser.
  
Short description of the properties.
 + Read and save sensors values in local storage
 + Show current values for sensors
 + Send sensors values to Thingspeak.com
 + Show diagramm for sensor by hour, day, month, year.(choosed of user`s)
  
![Common view ](https://github.com/andre-i/mini_weather/blob/master/pict/view.png)
![Forecast view](https://github.com/andre-i/mini_weather/blob/master/pict/forecast.png)
Used components:
  1. Hardware
  + esp12 module
  + sensors - BMP280, DHT11 or DHT22,[ optional -dallas DS18B20]
  + power source(3.3v 500mA)
  + bredboard or pcb
  + connectors
  + resistors : 10kOm , 4.7kOm
  + capacitor : 100uF, 100nF
  + USB-TTL converter on 3.3v
  2. software
  + Java 7 or above
  + Arduino IDE

Сircuit
![Circuit](https://github.com/andre-i/mini_weather/blob/master/pict/Schematic_ESP-weather-rev0.png)

