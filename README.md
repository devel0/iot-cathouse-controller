# iot-cathouse-controller

## requirements

- hardware
  - [billing of materials](doc/cathouse-drawings/bom.xlsx)
  - instructables to build and assembly cathouse parts
  - [nodemcu esp8266](https://www.google.com/search?q=nodemcu+esp8266)  
- software
  - [vscode arduino](https://github.com/devel0/knowledge/blob/master/electronics/vscode-arduino.md)
  - [esp8266 board plugin](https://github.com/esp8266/Arduino#installing-with-boards-manager)

### thirdy part libraries

- [ArduinoJsonWriter](https://github.com/maxpowel/ArduinoJsonWriter)
- [JsonStreamingParser](https://github.com/squix78/json-streaming-parser)
- [StringStream](https://gist.github.com/arcao/3252bb6e5e52493f03726ec32e61395c)

## debug

```
cd cathouse-controller
code .
```

- to build or check compilation `CTRL+SHIFT+R`
- to upload through USB-serial `CTRL+SHIFT+U`
