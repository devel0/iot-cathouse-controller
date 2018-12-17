# iot-cathouse-controller

## requirements

- [nodemcu](http://nodemcu.com/index_en.html)
- [vscode arduino](https://github.com/devel0/knowledge/blob/master/arduino/vscode-arduino.md)
- [esp8266 board plugin](https://github.com/esp8266/Arduino#installing-with-boards-manager)

## thirdy part libraries

- [ArduinoJsonWriter](https://github.com/maxpowel/ArduinoJsonWriter)
- [JsonStreamingParser](https://github.com/squix78/json-streaming-parser)
- [StringStream](https://gist.github.com/arcao/3252bb6e5e52493f03726ec32e61395c)

## compile

- set your wifi ssid and ssid key in a separate header file

```
mkdir ~/Arduino/libraries/security
echo '#define WIFI_SSID "labwlan"' > ~/Arduino/libraries/security/wifikey.h
echo '#define WIFI_KEY "wifipass"' >> ~/Arduino/libraries/security/wifikey.h
```

- ensure to have `dialout` group permission to access /dev/ttyUSBx devices ( eg. `usermod -a -G dialout user` )
- choose board NodeMCU 1.0

## debug

```
cd cathouse-controller
code .
```
