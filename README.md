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
 + On press *Forecast* button be show the forecast by 10 days(see image)/ For you place need get by yahoo.com and replace in */mini_weather/espSensors/data/index.htm* file script *<script src='https://query.yahooapis.com/v1/public/yql?q=select%20*%20from%20weather.forecast%20where%20woeid%20%3D%202003841%20&format=json&callback=yahooCallbackFunction&env=store%3A%2F%2Fdatatables.org%2Falltableswithkeys'></script>* on script appropriate for your place.

| view | forecast |
| --------- | --------- |
|![Common view ](https://github.com/andre-i/mini_weather/blob/master/pict/view.png)|![Forecast view](https://github.com/andre-i/mini_weather/blob/master/pict/forecast.png)|

Used components:
  1. Hardware
      + esp12 module
      + sensors - BMP280, DHT11 or DHT22,(optional -dallas DS18B20)
      + power source(3.3v 500mA)
      + bredboard or pcb
      + connectors
      + resistors : 10kOm , 4.7kOm, 330Om
      + capacitor : 100uF, 100nF
      + green led, red led
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
 
 ### Check
 
 After install on start chip open serial monitor. If in output have string with Wi-Fi parameters it all good. Wait 5 minutes and connect to wifi. On success connect open browser(it may be mozilla, chrome or opera). in address bar enter *http://weather.local*. In result you have see page as here.
 
 Note: address may be other if you replace value *#define HOST "weather"* from props.h file. You may write address as digits *http://192.168.0.1* or other, if IP address replaced.
____
## Краткое описание
  Это небольшая метеостанция работает на чипе ESP8266. К нему подключены два датчика DHT11(DHT22) и BMP280. Поскольку каждый из них ещё измеряет температуру имеется возможность измерять температуру внутри помещения и на улице, давление и влажность(в помещении т.к. DHT11 работает только при положительных температурах). Соответственно DHT находится в помещении, а BMP выносится на улицу.
  
  Каждые 5 минут производится измерение температуры, давления и влажности, за каждый час производится запись средних значений в память чипа. Одновременно чип работает в wifi сети для того, чтобы можно было увидеть показания датчиков. Текущие значения отображаются в виде показаний измерительных приборов. Записанные в память значения отображаются в виде графиков. По желанию для каждого датчика можно построить график за час, день, месяц, год если они входят в период измерений.
  
  Принцип работы можно посмотреть в файле Help.htm, который находится в директории */espSensors/data*. Если разметка не мешает можно читать прямо так. Если мешает тогда сохраните его на компьютер и откройте в браузере.
  
### Возможности

  - измерение температуры, давления влажности каждые 5 минут
  - запись в память чипа средних значений каждый час
  - отправка данных измерений на сайт [thingspeak.com](http://thingspeak.com)(необязательно)
  - передача результатов измерений по wi-fi для просмотра
  - просмотр текущих и сохранённых в модуле значений
  - при нажатии на кнопку прогноз будет показан прогноз от yahoo для тульской области. Чтобы заменить  местность прогноза надо на сервисах yahoo погоды найти скрипт для определения прогноза для данной местности и заменить скрипт *<script src='https://query.yahooapis.com/v1/public/yql?q=select%20*%20from%20weather.forecast%20where%20woeid%20%3D%202003841%20&format=json&callback=yahooCallbackFunction&env=store%3A%2F%2Fdatatables.org%2Falltableswithkeys'></script>* на полученный в файле */mini-weather/espSensors/data/index.htm*(он находится почти в самом конце файла).
  
### Что требуется
  
  1. Компоненты
      + модуль ESP12 или ESP12e
      + датчики BMP280, DHT11 или DHT22. Дополнительно можно подключать DS18B20
      + USB-TTL конвертер с напряжением  *3.3В* 
      + источник на *3.3В*
      + резисторы с номиналами 10кОм 3шт, 4,7кОм, 330Ом 2шт , конденсаторы электролитический на 100мкФ x 6.3В и любой на 100пФ
      + светодиод красный, светодиод зелёный. Два светодиода можно заменить на один двухцветный. Аноды такого светодиода сажаются на пины(см схему), а катод через резистор 330Ом подключается к земле.
      + кнопочный переключатель 2шт.
      + соединительные провода, плата
   2. Программы
      + Java не менее 7 версии
      + ArduinoIDE
			
Важно: __при напряжени больше 3.3В модуль может сгореть!!!__

### Как сделать.

+ собрать схему
+ установить Java и ArduinoIDE.
+ В ArduinoIDE 
  + Установить поддержку для ESP8266
  + Установить *ESP8266 Scetch Data Upload*
  + Догрузить необходимые библиотеки:adafruit bme280, adafruit sensor, dht, wire, one wire, dallas themperature
  + Скачать или клонировать этот репозиторий в любую удобную директорию на своём компьютере
  + Перейти в скачанную папку 
  + Перед компиляцией просмотрите файл *espSensors/props.h* - в нём начальные настройки для сенсоров, вай-фай, thingspeak. Ниже привожу какие строки можно изменить для своих параметров:
  
    | строка | описание |
    |-----|----|
    |#define DHT_TYPE DHT11| если датчик DHT22 заменить 11 на 22|
    |#define DHT_THEMP_SHIFT 0 | если выяснится ,что датчик DHT откалиброван неверно можно сдвинуть его показания в меньшую(число со знаком минус) или большую сторону. Для этого заменить 0 на своё число|
    |#define BMP_THEMP_SHIFT 0|сдвиг для датчика BMP аналогично DHT|
    |#define HOST "weather"|доменное имя для передачи данных|
    
    Ниже описаны настройки сети. Их лучше изменить используя 'serial monitor' из ArduinoIDE, но можно и в файле настроек
    
    |__настройки wi-fi__| если необходимо - заменить на своё |
    |----|----|
    |#define STA_SSID_DEF  "mywifi"|имя сети, к которй надо подключиться |
    |#define STA_PASSWD_DEF "mypasswd"|пароль для подключения к работающей сети |
    |#define AP_SSID_DEF "espWeather"|имя создаваемой чипом сети(режим точки доступа)|
    |#define AP_PASSWD_DEF  "espWeather"|пароль для точки доступа|
    |#define AP_IP_ADDR  "192.168.0.1"|адрес точки доступа(с него передаются данные в режиме точки доступа)|
    Остальные настройки лучше не трогать!
    
  + В ArduinoIDE зайти в меню  скетч и выполнить *проверить/скомпилировать*
  + Подать напряжение на схему. Во время запуска держать кнопку Flash нажатой, после можно отпустить.
  + В ArduinoIDE зайти в меню  скетч и выполнить *загрузить*
  + Перезапустить с нажатой Flash
  + В ArduinoIDE зайти в меню иструменты и запустить *ESP8266 Scetch Data Upload* 
  + После окончания загрузки запустить *serial monitor* с параметрам *no line ending* и скоростью 115200
  + Нажать кнопку Reset на плате(или на чём там собрана схема) и посмотреть вывод программы. Если всё нормально будет напечатан режим работы wi-fi и другая информация.**Обратить особое внимание на последнюю строку**
  + В консольной справке указано как пожно настроить стартовые параметры и посмотреть текущие настройки
  
### Проверка установки
Первоначально смотрим что выводит чип в последовательный порт. Если вывод пошёл и нет ошибок(см пункт выше) можно проверить работу чипа по wifi. Если чип запустился как точка доступа то у него будет адрес из начальных настроек. Далее проверяем наличие wi-fi сети с такими параметрами и пробуем подключиться к ней. Если подключились ждём 5 минут, открываем браузер и пробуем пойти по адресу *http://192.168.0.1* ( набор цифр может быть другим - смотрите в начальных настройках)


В случае подключения к существующей сети надо в выводе искать строку похожую на:

*WiFi work as STA connected to wifiSSID  IP address: 0.0.0.0*

где значения wifiSSID и 0.0.0.0 будут под вашу сеть.Далее порядок действий аналогичный проверке точки доступа. В этом случае подключаемся к указанной сети и прописываем адрес из строки.

В любом случае порядок действий одинаковый:
+ Подключение к wi-fi
+ Ждём 5 минут
+ набираем адрес в браузере можно в виде цифр, можно с использованием доменных имён: *http:// weather.local*. Если в props.h меняли значение параметра *#define HOST "weather"* то ставить вместо *weather* то, что прописано.
+ Смотрим результат в браузере. Если всё нормально, то получим приведённую выше html страницу со своими значениями датчиков.

### Замечания
+ Сразу после включения модуль работает только по последовательному порту. Ответить на запросы браузера он сможет только через 5 минут. Это связано с заполнением полей показаний датчиков. Проверяются они раз в 5 минут , соответственно и первые результаты появятся только через 5 минут.
+ Посмотреть показания за последний час можно не ранее чем через 65 минут после включения.
+ Выбирать период начинают с года далее уточняем месяц и число.
+ **Модуль**. Взят модуль ESP12 из-за памяти 4Мб в остальных модулях она меньше и , соответственно  меньший период наблюдений. В остальном без разницы какой модуль брать.
+ **Браузеры**. 
	+ Так как отображение значений датчиков идёт в виде SVG графики, то для просмотра соответственно надо выбирать браузер с поддержкой SVG графики. Мозила, хром, опера нормально отображают страницы, остальные не проверял.
	+ Модуль по wi-fi ответит на запрос как в чистом IP(цифры и точки) так и на DNS запрос(в виде имён). По умолчанию в режиме точки доступа запрос будет *http://weather.local*  или *http://192.168.0.1*. В режиме клиента доменный запрос не изменится а IP адрес надо взять из вывода в последовательный порт. Иногда при нормально работающем модуле DNS запросы не обрабатываются, а запросы к IP адресу работают всегда.
	+ Помимо значений датчиков в браузере можно посмотреть:
		+ содержимое текстового файла *http://weather.local/readFile?fName=/путь/и/имя/файла*
			<br>Замечание: файл настроек так посмотреть не удастся
		+ содержимое лога *http://weather.local/log*
		+ некоторые из текущих настроек *http://weather.local/getProps*
		+ в запросах  доменную часть(weather.local) можно заменять на IP(числовое значение)
+ Подключаться к модулю по последовательному порту можно не только в инструменте *serial monitor* из ArduinoIDE, но и любой другой программой работающей с последовательным портом, например putty или USB-serial для андроида.
+ **Некоторые настройки**. Для настройки используется последовательный порт путём отправки разных команд. Для этого надо подключиться к модулю(как подключиться смотри выше). После того, как соединение установлено шлём модулю *ru* и читаем как и что делается. Ниже некоторые настройки и возможности работы через последовательный порт.*Все изменения вступают в силу после перезагрузки.*
	+ *Установить время*. При записи значений в память необходимо знать текущее время. В режиме подключения к существующей сети посылается запрос на yandex и из ответа получаем время. В режиме точки доступа таким способом время не определить, его **обязательно** надо установить, иначе показания датчиков не будут записываться.
	+ *Thingspeak* . Если модуль имеет постоянное подключение к сети интернет можно отправлять данные на сервис Thingspeak и обращаться к нему для просмотра показаний через интернет. Там ещё много чего можно сделать. Кто не знает спросите гугла или яндекса. Для этого надо зарегистрироваться на сервисе, создать канал, настройить визуализацию. После скопировать ключ для записи в данный канал(write api key) и прописать его в настройках модуля, и модуль будет отсылать данные на Thingspeak. 
	+ *Датчик DS18B20* . Дополнительно можно сравнить показания температуры, подключив DS18B20. Датчик подключается к тем же пинам, что и DHT. Чтобы включить его в работу надо прописать соответствующий параметр. Значение температуры датчик передаёт в последовательный порт. Подключаемся к порту и шлём *y*(игрек а не букву у) чтобы включить отображение вывода. Перед отключением датчика **обязательно** изменяем параметр в настройках.
	+ *Файловая система* . Можно читать содержимое как отдельных файлов, так и директорий. Можно создавать файлы, стирать,  перезаписывать и дополнять.
	+ Можно проверить текущее время и настройки модуля
	
_____
*PS: Как настраивать ArduinoIDE, работать с сервисом thingspeak не привожу. Смотрите в интернете.*
