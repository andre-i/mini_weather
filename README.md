# mini_weather
## мини метеостанция(описание на русском ниже).
### The short description.
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

### How to:
 + set java
 + set Arduino IDE
 + in IDE set support for ESP8266
 + Add in IDE ESP8266 scetch data uploader,add need library - adafruit bme280, adafruit sensor, dht, wire, one wire, dallas themperature(support ds18b20)
 + assemble a scheme
 + clone or download this repo and set it into any directory
 + Open witch ArduinoIDE file: repo_dir/mini_weather/espSensors/espSensors.ino
 + Before compile and upload may be you need see "props.h" file in espSensors directory and change some values. 
 + In ArduinoIDE first time execute: Scetch->Verify/Compile, second time press Flash button on assembled and power on, leave in Arduino IDE execute: Scetch->Upload
 + Restart chip with pressed Flash button
 + In Arduino IDE execute: Tools->ESP8266 Scetch Data Upload
 + In IDE execute: Tools->serial monitor( in serial monitor set "no line ending" and speed 115200)
 + Restart chip
 + for first help send to chip in serial monitor "en" witch no double quotes. Some properties may see in root_dir/miniWeather/espSensors/props.h
____
## Краткое описание
  Это небольшая метеостанция работает на чипе ESP8266. Каждые 5 минут производится измерение температуры, давления и влажности, за каждый час производится запись средних значений в память чипа. Одновременно чип работает в wifi сети для того, чтобы можно было увидеть показания датчиков. Текущие значения отображаются в виде показаний измерительных приборов. Записанные в память значения отображаются в виде графиков. По желанию для каждого датчика можно построить график за час, день, месяц, год если они входят в период измерений.
  Принцип работы можно посмотреть в файле Help.htm, который находится в директории */espSensors/data*. Если разметка не мешает можно читать прямо так. Если мешает тогда сохраните его на компьютер и откройте в браузере.

  ### Возможности
  - измерение теемпературы, давления влажности каждые 5 минут
  - запись в память чипа средних значений каждый час
  - отправка данных измерений на сайт [thingspeak.com](http://thingspeak.com)
  - передача результатов измерений для просмотра по wi-fi
  - просмотр текущих и сохранённых значений.
##   
