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
    // set content to PARAMS FILE
    struct params {
      String ap_ssid = "";
      String ap_passwd = "";
      String sta_ssid = "";
      String sta_passwd = "";
      String ap_ip = "";
      String isDebug = "false";
    };
    struct params getParamsFromFile();
    void writeToPropFile( struct params par);
    // wifi ap and sta mode
    String writeWifiProps(String props);
    bool fillPropsFromString(String props, struct params *par);
    // IP address in AP mode
    String writeApModeIpAddr(char *addr);
    // DEBUG MODE
    String writeDebugMode(String isDebug);
    // ru manual
  const char * help_ru = "\t HELP_RU \
  при нажатии на любой символ кроме 'y' или 'n' - показать доступные команды\n\
  ЗАМЕЧАНИЕ: все команды вводятся без кавычек и латинскими символами\n\
1) Режим отладки - 'y' - включено  \t'n' - отключено\n\
2) Файловая система: r_(чтение) w_(перезапись) a_(добавление строки к файлу) nl_(добавить новую строку к файлу) \n\
    'r_/full/path/with/fileName.ext' - чтение содержимого файла\n\
    'w_/full/path/with/fName.ext=бла-бла-бла' ( запись строки 'бла-бла-бла' в файл, старый файл с таким именем сотрётся)\n\
    'a_/full/path/with/fName.ext=текст' - добавить слово 'текст' в конец файла, перевода на новую строку нет)\n\
     ВНИМАНИЕ - если в первом примере после знака равно ничего нет, то файл будет стёрт\n\
3) Информация о файловой системе чипа - 'si'\n\
4) Уcтановить время - 'date год/месяц/число/час/минуты'\n\
      год, чисо, часы, минуты вводятся в виде чисел, если число меньше 10 \n\
      то перед ним ставится ноль. месяц ввести по трём первым буквам:\n\
    январь jan | февраль feb | март mar | апрель apr\n\
    май may | июнь jun | июль jul |август aug\n\
    сентябрь sep | октябрь oct | ноябрь nov | декабрь dec\n\
    пример: 'date 2018/sep/23/11/23'\n\
5) Установка параметров запуска\n\
    параметры Wi-Fi - 'wifi ap[ssid passwd] sta[ssid passwd]'\n\
      ssid - имя сети\n\
      passwd - пароль\n\
      часть ap(точка доступа) - обязательная\n\
      часть sta(клиент) - можно опустить\n\
    адрес чипа в режиме точки доступа - 'apip 192.168.0.1'\n\
      набор чисел и точек должен соответствовать стандарту 4 бит IP адресов \n\
    режим вывода отладочных сообщений в последовательный порт(serial)\n\
      'debug режим' (режим может быть true или false)\n\
      при false вывода не будет\n\
 формат файла с параметрами запуска следующий:\n\
     # - коментарий\n\
     #AP\n\
     as далее имя сети\n\
     ap далее пароль\n\
     ip далее адрес точки доступа\n\
     #STA\n\
     ss далее имя сети\n \
     sp далее пароль\n\
     #debug mode\n\
     dm далее значение"; 
    // en manual
    const char *help_en = " Console command:\n\
      1) DEBUG : \t\"y\" - ON  \t\"n\" - OFF \n\
      2) Check to work with SPIFFS :\n\t\"sp\" - print whether available SPIFFS\n\
      3) SPIFFS : \n\tread : r_/full/path/with/fileName.ext  where:\n\
          \t  r_  - is signal on print file(or directory) content \n\t  /full/path/with/fileName.ext - fullFile name\n\
          \t write           : after \"equals\" sign be char sequense for write\n\
          \t append          : a_/full/path/with/fName.ext=after \"equals\" sign be char sequense for append\n\
          \t append new_line : nl_/full/path/with/fName.ext\n\
              WARNING: if after equals sign to be nothing, then file be REMOVED !\n\
      4) See SPIFFS info : \n\t\"si\" - print info about SPIFFS of chip\n\
      5) Set date/time - enter it in format:\"year/month/day/hour/minute\" \tWARNING - date must be strongly follow format, else date be break\n\
        \tyear - 4 numbers\n\t\tmonth - 3 chars( first must be Upper Case)\n\t\tday, hour and minute consist from 2 numbers, if value less than 10 firs must be 0(null)\n\
      6) Set Wi-Fi properties - \"wifi ap[ssid password] sta[ssid password]\" (double quotes do not put)\n\
          \twhere ssid - wifi name, password - wifi password\n\t sta part can`t mandatory\n\
      7) Set IP address for AP mode Wi-Fi - \"apip 192.168.0.1\"\n\
          \t the set numbers and dot  must be valid IP4V address,the double quotes do not put\n\
      8) Set debug mode(whether show debug messages) - \"debug isDebug\",\n\there isDebug may be true or false,the double quotes do not put\n\
          NOTE in props file first simbols in string have meaning\n\
          \ta)'#' - it is comment \n\tb)\t'ss' - STA ssid, \n\t\t'sp' - STA password, \n\t\t'as' - AP ssid, \n\
          \t\t'ap' - AP password, \n\t\t'ip - IP address for AP mode \n\tc) record format first simbol(s)+white space+value";

};


#endif
