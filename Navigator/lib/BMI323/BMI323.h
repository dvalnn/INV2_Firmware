#ifndef BMI323_H
#define BMI323_H

#include <Arduino.h>
#include <SPI.h>
#include <Adafruit_SPIDevice.h>

// Definições de registradores
#define CHIP_ID_REG 0x00
#define ERR_REG 0x01
#define STATUS_REG 0x02
#define ACC_CONF_REG 0x20
#define GYR_CONF_REG 0x21
#define ACC_DATA_X_REG 0x03
#define ACC_DATA_Y_REG 0x04
#define ACC_DATA_Z_REG 0x05
#define GYR_DATA_X_REG 0x06
#define GYR_DATA_Y_REG 0x07
#define GYR_DATA_Z_REG 0x08
#define TEMP_DATA_REG 0x09
#define CMD_REG 0x7E
#define FEATURE_IO0_REG 0x10
#define FEATURE_IO1_REG 0x11
#define FEATURE_IO2_REG 0x12
#define FEATURE_IO_STATUS_REG 0x14
#define FEATURE_CTRL_REG 0x40

#define SPI_FREQ 10000000 //10 MHz
#define ACC_RANGE_LSB_PER_G 4096.0
#define GYR_RANGE_LSB_PER_DPS 16.384
#define SOFT_RESET_CMD 0xDEAF
#define ACC_CONF_NORMAL_100HZ_8G 0x4028
#define GYR_CONF_NORMAL_100HZ_2000DPS 0x4048

class BMI323 {
public:
    BMI323(uint8_t csPin, SPIClass *spiDevice);
    bool begin();
    bool read(float &ax, float &ay, float &az, float &gx, float &gy, float &gz, float &temp);

private:
    Adafruit_SPIDevice spi;

    bool initBMI323();
    bool initializeFeatureEngine();

    uint8_t readRegister8(uint8_t addr);
    uint16_t readRegister16(uint8_t addr);
    void readRegisterN(uint8_t addr, uint8_t buffer[], uint8_t n);

    void writeRegister8(uint8_t addr, uint8_t data);
    void writeRegister16(uint8_t addr, uint16_t data);

    float convertAccelData(uint16_t raw);
    float convertGyroData(uint16_t raw);
    float convertTempData(uint16_t raw);
};

#endif
