#include "TempDev.h"
#include "Config.h"
#include "Util.h"

int temperatureDeviceCount = 0;

DeviceAddress *tempDevAddress; // DeviceAddress defined as uint8_t[8]
char **tempDevHexAddress;

#define TEMPERATURE_INTERVAL_MS 5000
unsigned long lastTemperatureRead;
float *temperatures = NULL; // current temp

unsigned long lastTemperatureHistoryRecord;
uint8_t temperatureHistoryFillCnt = 0;
// each sensor history for 48hour each 1 min record data

uint16_t TEMPERATURE_HISTORY_SIZE = 0; // computed
float **temperatureHistory = NULL;
uint16_t temperatureHistoryOff = 0;
uint16_t TEMPERATURE_HISTORY_INTERVAL_SEC = 5 * 60; // computed

OneWire tempOneWire(ONEWIRE_PIN);
DallasTemperature DS18B20(&tempOneWire);

void SetupTemperatureDevices()
{
    DS18B20.begin();
    temperatureDeviceCount = DS18B20.getDeviceCount();
    Serial.printf("temperature device count = %d\n", temperatureDeviceCount);
    if (temperatureDeviceCount > 0)
    {
        temperatures = new float[temperatureDeviceCount];
        tempDevAddress = new DeviceAddress[temperatureDeviceCount];
        tempDevHexAddress = (char **)malloc(sizeof(char *) * temperatureDeviceCount);

        for (int i = 0; i < temperatureDeviceCount; ++i)
        {
            tempDevHexAddress[i] = (char *)malloc(sizeof(char) * (TEMPERATURE_ADDRESS_BYTES * 2 + 1));
            DS18B20.getAddress(tempDevAddress[i], i);
            sprintf(tempDevHexAddress[i], "%02x%02x%02x%02x%02x%02x%02x%02x",
                    tempDevAddress[i][0],
                    tempDevAddress[i][1],
                    tempDevAddress[i][2],
                    tempDevAddress[i][3],
                    tempDevAddress[i][4],
                    tempDevAddress[i][5],
                    tempDevAddress[i][6],
                    tempDevAddress[i][7]);

            Serial.printf("sensor [%d] address = %s\n", i, tempDevHexAddress[i]);

            DS18B20.setResolution(12);
        }

        temperatureHistory = (float **)malloc(sizeof(float *) * temperatureDeviceCount);

        auto ramsize = FreeMemorySum() - TEMPERATURE_HISTORY_FREERAM_THRESHOLD - 3*1024; // 3 kb diff for wifi
        TEMPERATURE_HISTORY_SIZE = ramsize / temperatureDeviceCount / sizeof(float);

        TEMPERATURE_HISTORY_INTERVAL_SEC = TEMPERATURE_HISTORY_BACKLOG_HOURS * 60 * 60 / TEMPERATURE_HISTORY_SIZE;

        for (int i = 0; i < temperatureDeviceCount; ++i)
            temperatureHistory[i] = (float *)malloc(sizeof(float) * TEMPERATURE_HISTORY_SIZE);
    }
    ReadTemperatures();
}

void ReadTemperatures()
{
    DS18B20.requestTemperatures();
    for (int i = 0; i < temperatureDeviceCount; ++i)
    {
        auto temp = DS18B20.getTempC(tempDevAddress[i]);
        //Serial.printf("temperature sensor [%d] = %f\n", i, temp);
        temperatures[i] = temp;
    }

    lastTemperatureRead = millis();
}

void manageTemp()
{
    if (TimeDiff(lastTemperatureRead, millis()) > TEMPERATURE_INTERVAL_MS)    
        ReadTemperatures();    

    if (temperatureHistory != NULL &&
        (TimeDiff(lastTemperatureHistoryRecord, millis()) > 1000UL * TEMPERATURE_HISTORY_INTERVAL_SEC))
    {
        if (temperatureHistoryFillCnt < TEMPERATURE_HISTORY_SIZE)
            ++temperatureHistoryFillCnt;

        if (temperatureHistoryOff == TEMPERATURE_HISTORY_SIZE)
            temperatureHistoryOff = 0;

        for (int i = 0; i < temperatureDeviceCount; ++i)                    
            temperatureHistory[i][temperatureHistoryOff] = temperatures[i];
        
        ++temperatureHistoryOff;
        lastTemperatureHistoryRecord = millis();        
    }    
}