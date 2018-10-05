# iot-cathouse-controller

## requirements

- [nodemcu](http://nodemcu.com/index_en.html)
- [arduino ide](https://www.arduino.cc/en/Main/Software)
- [esp8266 board plugin](https://github.com/esp8266/Arduino#installing-with-boards-manager)

## compile

- set your wifi ssid and ssid key in a separate header file and set the path into the sketch

```c
#include "/home/devel0/security/wifikey.h"
```

`wifikey.h` example:

```c
#define WIFI_SSID "labwlan"
#define WIFI_KEY "something"
```

- ensure to have `dialout` group permission to access /dev/ttyUSBx devices ( eg. `usermod -a -G dialout user` )
- choose board NodeMCU 1.0
