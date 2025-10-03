#include "Thermo.h"
#include "IO_Map.h"

Adafruit_MAX31856 maxthermo1 = Adafruit_MAX31856(SPI_MISO, SPI_MOSI, THERMO1_CS, SPI_SCK);
Adafruit_MAX31856 maxthermo2 = Adafruit_MAX31856(SPI_MISO, SPI_MOSI, THERMO2_CS, SPI_SCK);
Adafruit_MAX31856 maxthermo3 = Adafruit_MAX31856(SPI_MISO, SPI_MOSI, THERMO3_CS, SPI_SCK);

int thermo_setup(void) {
    bool thermo1_config, thermo2_config = false;
    if (!maxthermo1.begin()) {
        Serial.println("Failed to initialize thermo1!");
        thermo1_config = true;
    }
    if (!maxthermo2.begin()) {
        Serial.println("Failed to initialize thermo2!");
        thermo2_config = true;
    }
    if (!maxthermo3.begin()) {
        Serial.println("Failed to initialize thermo3!");
        if (thermo1_config && thermo2_config)
            return -1;  // Exit if all thermocouples fail
    }

    maxthermo1.setThermocoupleType(MAX31856_TCTYPE_K);
    maxthermo2.setThermocoupleType(MAX31856_TCTYPE_K);
    maxthermo3.setThermocoupleType(MAX31856_TCTYPE_K);

    maxthermo1.setConversionMode(MAX31856_CONTINUOUS);
    maxthermo2.setConversionMode(MAX31856_CONTINUOUS);
    maxthermo3.setConversionMode(MAX31856_CONTINUOUS);

    maxthermo1.setAveragingSamples(16);
    maxthermo2.setAveragingSamples(16);
    maxthermo3.setAveragingSamples(16);

    maxthermo1.setNoiseFilter(MAX31856_NOISE_FILTER_50HZ);
    maxthermo2.setNoiseFilter(MAX31856_NOISE_FILTER_50HZ);
    maxthermo3.setNoiseFilter(MAX31856_NOISE_FILTER_50HZ);

    Serial.println("MAX31856 thermocouples initialized!");
    return 0;  // Successful initialization
}
