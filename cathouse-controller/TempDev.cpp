#include "TempDev.h"
#include "Config.h"
#include "Util.h"
#include "Stats.h"
#include "EEStaticConfig.h"
#include "EEJsonConfig.h"
#include "WeightDev.h"

int temperatureDeviceCount = 0;

DeviceAddress *tempDevAddress; // DeviceAddress defined as uint8_t[8]
char **tempDevHexAddress;

float *temperatures = NULL; // current temp
#define TEMPERATURE_INTERVAL_MS 5000
unsigned long lastTemperatureRead;

// each sensor history for 48hour each 1 min record data
uint16_t temperatureHistorySize = 0; // computed
float **temperatureHistory = NULL;
uint16_t temperatureHistoryOff = 0;
uint16_t temperatureHistoryFillCnt = 0;

unsigned long lastTemperatureHistoryRecord;
uint16_t temperatureHistoryIntervalSec = 5 * 60; // computed

BitArray *catInThereHistory;

OneWire tempOneWire(ONEWIRE_PIN);
DallasTemperature DS18B20(&tempOneWire);

void setupTemperatureDevices()
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

        auto threshold = FREERAM_THRESHOLD_MIN_BYTES;
        auto ramsize = freeMemorySum() - threshold - ADCWEIGHT_HISTORY_BACKLOG_KB * 1024 - 3 * 1024; // 3 kb diff for wifi
        temperatureHistorySize = ramsize / temperatureDeviceCount / sizeof(float);

        auto backloghr = TEMPERATURE_HISTORY_BACKLOG_HOURS;
        temperatureHistoryIntervalSec = backloghr * 60 * 60 / temperatureHistorySize;

        for (int i = 0; i < temperatureDeviceCount; ++i)
            temperatureHistory[i] = (float *)malloc(sizeof(float) * temperatureHistorySize);

        lastTemperatureHistoryRecord = millis();

        catInThereHistory = new BitArray(temperatureHistorySize);
    }
    readTemperatures();
}

float tbottom, twood, tambient, textern;
bool tbottom_assigned = false;
bool twood_assigned = false;
bool tambient_assigned = false;
bool textern_assigned = false;

void readTemperatures()
{
    DS18B20.requestTemperatures();

    tbottom_assigned = twood_assigned = tambient_assigned = textern_assigned = false;
    for (int i = 0; i < temperatureDeviceCount; ++i)
    {
        auto id = (char *)tempDevAddress[i];
        auto temp = DS18B20.getTempC(tempDevAddress[i]);        

        if (strncmp(id, eeStaticConfig.tbottomId, DS18B20_ID_STRLENMAX) == 0)
        {
            tbottom = temp;
            tbottom_assigned = true;
        }
        else if (strncmp(id, eeStaticConfig.twoodId, DS18B20_ID_STRLENMAX) == 0)
        {
            twood = temp;
            twood_assigned = true;
        }
        else if (strncmp(id, eeStaticConfig.tambientId, DS18B20_ID_STRLENMAX) == 0)
        {
            tambient = temp;
            tambient_assigned = true;
        }
        else if (strncmp(id, eeStaticConfig.texternId, DS18B20_ID_STRLENMAX) == 0)
        {
            textern = temp;
            textern_assigned = true;
        }

        //Serial.printf("temperature sensor [%d] = %f\n", i, temp);
        //Serial.printf("tbottom [%f] ; twood [%f] ; tambient [%f] ; textern [%f]\n", tbottom, twood, tambient, textern);
        temperatures[i] = temp;
    }

    lastTemperatureRead = millis();
}

void manageTemp()
{
    {
        auto delta = timeDiff(lastTemperatureRead, millis());
        if (delta >= UPDATE_TEMPERATURE_INTERVAL_MS)
            readTemperatures();
    }

    if (temperatureHistory != NULL &&
        (timeDiff(lastTemperatureHistoryRecord, millis()) > 1000UL * temperatureHistoryIntervalSec))
    {
        if (temperatureHistoryFillCnt < temperatureHistorySize)
            ++temperatureHistoryFillCnt;

        if (temperatureHistoryOff == temperatureHistorySize)
            temperatureHistoryOff = 0;

        for (int i = 0; i < temperatureDeviceCount; ++i)
            temperatureHistory[i][temperatureHistoryOff] = temperatures[i];

        catInThereHistory->Set(temperatureHistoryOff, catInThere);

        ++temperatureHistoryOff;
        lastTemperatureHistoryRecord = millis();
    }
}