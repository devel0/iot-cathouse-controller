# iot-cathouse-controller

## requirements

- [nodemcu](http://nodemcu.com/index_en.html)
- [arduino ide](https://www.arduino.cc/en/Main/Software) and optionally [vscode arduino](https://github.com/devel0/knowledge/blob/master/arduino/vscode-arduino.md)
- [esp8266 board plugin](https://github.com/esp8266/Arduino#installing-with-boards-manager)

## compile

- set your wifi ssid and ssid key in a separate header file and set the path into the sketch

```c
#include "/home/devel0/security/my-wifi-key.h"
```

`my-wifi-key.h` example:

```c
#define WIFI_SSID "labwlan"
#define WIFI_KEY "something"
```

- tune `my-wifi-key.h` path into `c_cpp_properties.json`
- ensure to have `dialout` group permission to access /dev/ttyUSBx devices ( eg. `usermod -a -G dialout user` )
- choose board NodeMCU 1.0
