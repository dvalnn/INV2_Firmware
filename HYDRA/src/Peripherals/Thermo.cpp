#include "Peripherals/Thermo.h"
#include "Peripherals/IO_Map.h"

struct thermo_callback {
    thermo_data_callback callback;
    void *user_data;
};

static struct thermo_callback thermo_cb = {nullptr, nullptr};

#define MAKE_THERMO(n)                                                         \
    static Adafruit_MAX31856 maxthermo##n = Adafruit_MAX31856(                 \
        SPI_MISO_PIN, SPI_MOSI_PIN, THERMO##n##_CS_PIN, SPI_SCK_PIN);

#define MAKE_THERMO_ISR(n)                                                     \
    void thermo_##n##_isr(void) {                                              \
        if (!thermo_cb.callback)                                               \
            return;                                                            \
        /* NOTE: maybe the conversionComplete check can be skipped. Test. */   \
        if (maxthermo##n.conversionComplete()) {                               \
            const float temp = maxthermo##n.readThermocoupleTemperature();     \
            thermo_cb.callback(n, temp, thermo_cb.user_data);                  \
        }                                                                      \
    }

#define SETUP_ISR_IF_RDY(n, rdy)                                               \
    if (rdy) {                                                                 \
        pinMode(THERMO##n##_DRDY_PIN, INPUT);                                  \
        attachInterrupt(digitalPinToInterrupt(THERMO##n##_DRDY_PIN),           \
                        thermo_##n##_isr, FALLING);                            \
    }

MAKE_THERMO(1)
MAKE_THERMO(2)
MAKE_THERMO(3)

MAKE_THERMO_ISR(1)
MAKE_THERMO_ISR(2)
MAKE_THERMO_ISR(3)

int thermo_setup(void) {
    bool thermo1_rdy, thermo2_rdy, thermo3_rdy = false;

    if (!maxthermo1.begin()) {
        Serial.println("Failed to initialize thermo1!");
        thermo1_rdy = true;
    }
    if (!maxthermo2.begin()) {
        Serial.println("Failed to initialize thermo2!");
        thermo2_rdy = true;
    }
    if (!maxthermo3.begin()) {
        thermo3_rdy = true;
        Serial.println("Failed to initialize thermo3!");
    }

    if (!thermo1_rdy && !thermo2_rdy && !thermo3_rdy) {
        Serial.println("All thermocouples failed to initialize!");
        return -1; // All thermocouples failed to initialize
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

    SETUP_ISR_IF_RDY(1, thermo1_rdy);
    SETUP_ISR_IF_RDY(2, thermo2_rdy);
    SETUP_ISR_IF_RDY(3, thermo3_rdy);

    Serial.println("MAX31856 thermocouples initialized!");
    return 0; // Successful initialization
}

void set_thermo_callback(thermo_data_callback callback,
                                void *user_data) {
    thermo_cb.callback = callback;
    thermo_cb.user_data = user_data;
}
