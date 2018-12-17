#ifndef _SERIAL_OS_
#define _SERIAL_OS_

#include <Arduino.h>

extern String serialInput;
extern bool serialOsActivated;

void printSyntaxHelp();
void processSerialCmd();

#endif