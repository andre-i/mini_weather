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
    void showManual(const char* language);
    void showFile(char answ[]);
    // write
    bool writeToFile(char req[]);
    bool write(String fileName, String toWrite);
    // apend to file many string
    int append(char* toAppend);
    bool isAppend = false;
    File toAppend;
    // execution mode(debug or work)
    String setDebug(String res);
    void executeCommand(char answ[100]);
    //
    // set content to PARAMS FILE
    //
    struct params {
      String only_sta = "false";
      String ap_network = "";
      String sta_ssid = "";
      String sta_passwd = "";
      String ap_ssid = "";
      String ap_passwd = "";
      String ap_ip = "";
      String ts_api_key = "";
      String ds18b20_mode = "false";
      String isDebug = "false";
    };
    //        for bulk set params
    struct params newParams;
    void fillStartParameters(); // start bulk write process
    bool isSetParam = false;
    char* parName;
    void setParameter(char* value);
    void showSummary();
    void writeToPropFile( struct params par);
    //
 //   struct params getParamsFromFile();
    //         wifi ap and sta mode
 //   String writeWifiProps(String props);
 //   bool checkAndFillWifiProps(String props, struct params *par);
    // IP address in AP mode
  //  String writeApModeIpAddr(char *addr);
  //  String writeDebugMode(String isDebug);
  //  String writeTsApiKey(String tsApiKey);
    // ru manual
    const char * help_ru = "\n\t HELP_RU \n\
  ЗАМЕЧАНИЕ: все команды вводятся без кавычек и латинскими символами\n\
1) Режим вывода сообщений(log) \n\t'y' - включено  \n\t'n' - отключено\n\
2) Режим отладки\
    \n\t'debug y' (y - отладка включена, n - отключено)\
     \n\tпри включенной отладке включается вывод отладочных сообщений\n\tопрос датчиков идёт каждые 5 секунд,\n\tзапись значений в файл ежеминутно\n\
3) Показать текущее временя для чипа \n\t'time'\n\
4) Файловая система: r_(чтение) w_(перезапись) a_(добавление строки к файлу) nl_(добавить новую строку к файлу)\
    \n\t'r_/full/path/with/fileName.ext'   чтение содержимого файла\
    \n\t'w_/full/path/with/fName.ext=бла-бла-бла'   запись строки бла-бла-бла в файл, старый файл с таким именем перезапишется\
          \n\t\tВНИМАНИЕ!!! Если после знака равно ничего нет, то файл будет стёрт\
    \n\t'a_/full/path/with/fName.ext добавить текст в конец файла\
    \n\t\t Для перехода на новую строку вводить '%%'\
    \n\t\t Для выхода из режима записи в файл ввести '##$'\n\
5) Информация о файловой системе чипа\n\t'si'\n\
6) Уcтановить время (формат ввода - год/месяц/день/часы/минуты)\
    \n\t'date 2018/sep/06/11/23'  установит для чипа время 11:23 6 сентября 2018 года \
     \n\t  год, день, часы, минуты вводятся в виде чисел, если число меньше 10 \
      \n\t  то перед ним ставится ноль. месяц ввести по трём первым буквам:\n\
    январь jan   | февраль feb |  март mar  | апрель apr \n\
      май may    |   июнь jun  |  июль jul  |август aug \n\
    сентябрь sep | октябрь oct | ноябрь nov | декабрь dec \n\
7) Посмотреть текущие настройки приложения \n\t'current' \n\
8) Перезапуск модуля \n\t'restart'\n\
 \n\t\t УСТАНОВКА ПАРАМЕТРОВ ЗАПУСКА\
\n\t'fillParam'\t  после ввода этой команды будут вводиться запросы\
\n\t  на заполнение отдельных полей они вступят в силу после перезагрузки\
 \n\tесли устраивает параметр по умолчанию вместо значения ОБЯЗАТЕЛЬНО надо вводить пробел(ы).Пустые строки не позволены.\
 \n Назначение отдельных  параметров\
    \n  а) работа только в режиме WiFi клиента\
      \n\t  ONLY_STA - при true чип работает только как клиент\
      \n\t  AP_NETWORK_ADDRESS - параметр нужен для работы в режиме клиента\n  так-как wifi не очень надёжен - его надо переодически проверять\
\n проверка сводится к запросу сетевого интерфейса точки доступа, если соединение неудачное надо перезапустить WiFi\
\n если данный параметр неопределён проверки WiFi не происходит и считается что всё работает нормально\
\n если определён, то каждый час происходит проверка соединения с веб интерфейсом точки доступа в случае неудачного соединения WiFi перезапустится.\
    \n  б) параметры Wi-Fi для клиента и точки доступа \
      \n\t\tssid - имя сети\
      \n\t\tpasswd - пароль\
      \n\t\tчасть ap(точка доступа) \
      \n\t\tчасть sta(клиент)\
    \n  в) адрес чипа в режиме точки доступа (ip4v address) по умолчанию '192.168.0.1'\
      \n\t набор чисел и точек должен соответствовать стандарту 4 бит IP адресов \
    \n  г) ключ для пересылки данных датчиков на thingspeak. Работет только в режиме клиента(STA), если доступен интернет\
    \n  д) работает ли датчик DS18B20 ? если true, то датчик включен. \
        \n\tПоказания датчика в этом случае не идут по сети, а передаются в последовательный порт\
        \n\tчто можно использовать для проверки показаний температуры других датчиков\
    \n  е) режим отладки при старте системы(по умолчанию отключено)\n\t 'true'  включено (при false отключено) \
        \n\t\t\t при true - опрос датчиков каждые 5 секунд, \
                   \n\t\t\t   запись в файл каждую минуту,\
                   \n\t\t\t   идёт вывод сообщений в последовательный порт(COM)\
        \n\t\t\t при false всё работает штатно вывода в 'serial(COM)' не будет\
      \n Замечание: не введённые параметры при старте модуля берутся из прописанных в прошивке(props.h).\
\n\n\t\t  ФОРМАТ ФАЙЛА С НАЧАЛЬНЫМИ ПАРАМЕТРАМИ\
\n\t# - коментарий\
\n\t# ONLY_STA - будет ли работать только как клиент\
\n\tos далее значение\
\n\t# AP_NETWORK_ADDRESS\
\n\tan далее адрес сетевого интерфейса точки доступа\
\n\t#AP\
\n\tas далее имя сети\
\n\tap далее пароль\
\n\tip далее адрес точки доступа\
\n\t#STA\
\n\tss далее имя сети\
\n\tsp далее пароль\
\n\t#debug mode\
\n\tdm далее значение\
\n\t#thingspeak write key\
\n\tts далее ключ\
\nПримечание: данный файл можно самостоятельно создать и заполнить(см. пункт 4)\n\
При таком заполнении файла необходимо строго следовать формату иначе\n\
нормальной работы не гарантируется.\n\
Лучше устанавливать эти значения по команде 'fillParam' или заполнять поля по отдельности.(см выше)\n"; 
    // en manual
    const char *help_en = " Console command:\n\
      1) LOG output : \t'y' - ON  \t'n' - OFF \n\
      2) DEBUG mode : 'debug mode' - mode may be y(yes) or n(no) \n       in debug mode sensors request time - 5 sec, write values time - 1 minute\n\
      3) Check to work with SPIFFS :\n\t'sp' - print whether available SPIFFS\n\
      4) SPIFFS : \n\tread : r_/full/path/with/fileName.ext  where:\
          \n\t  r_  - is signal on print file(or directory) content \n\t  /full/path/with/fileName.ext - fullFile name\
          \n\t write           : after 'equals' sign be char sequense for write\
              \n\t\tWARNING: if after equals sign to be nothing, then file be REMOVED !\n\
          \n\t append          : a_/full/path/with/fName.ext 'equals' \n\t\tfor new line enter '%%' \n\t\tfor exit from append entert '##$'\n\
      5) See SPIFFS info : \n\t'si' - print info about SPIFFS of module\n\
      6) Get date-time on module : 'time' \n\
      6) Set date/time - enter it in format:'year/month/day/hour/minute' \n\tWARNING - date must be strongly follow format, else date be break\n\
        \tyear - 4 numbers\n\t\tmonth - 3 chars( first must be Upper Case)\n\t\tday, hour and minute consist from 2 numbers, if value less than 10 firs must be 0(null)\n\
     7) Show current properties of application : 'current'\n\
     8) Restart module : 'restart'\n\
           \n\tProperties file for set start parameter values(wifi, debug, log, thingspeak\n\
     9) Set all start parameters in one command - 'fillParam'\n Whitespace value mean default value. Empty values not permitted(it must be whitespace)\
          \n NOTE in props file first simbols in string have meaning\
          \n\ta)'#' - it is comment \n\t\tb)'ss' - STA ssid, \n\t\t'sp' - STA password, \n\t\t'as' - AP ssid, \n\t\t\'os' - whether only STA mode, \n\t\t'an' - address of access point network interface, for check suspend WiFi whether \
          \n\t\t'ap' - AP password, \n\t\t'ip' - IP address for AP mode \n\t'dm' -debug mode(may be true or false)\
          \n\t\t'ts' - thingspeak key\
          \n\t\t'ds' - whether work DS18B20 sensor (if true - in work, some other - not work\
          \n\t\tc) record format first simbol(s)+white space+value\
          \n\tnew values will be apply on reboot, empty values will replaced on default";  

};


#endif
