#include "BMI323.h"

BMI323::BMI323(uint8_t csPin) : spiSettings(SPI_FREQ, MSBFIRST, SPI_MODE3), cs(csPin) {}

bool BMI323::begin() {
    pinMode(cs, OUTPUT);
    digitalWrite(cs, HIGH);
    SPI.begin();
    return initBMI323();
}

bool BMI323::initBMI323() {
    // Reset
    writeRegister16(CMD_REG, SOFT_RESET_CMD);
    delay(50);

    // Verifica CHIP ID
    if (readRegister16(CHIP_ID_REG) != 0x323) return false;

    // Configura acelerômetro e giroscópio
    writeRegister16(ACC_CONF_REG, ACC_CONF_NORMAL_100HZ_8G);
    writeRegister16(GYR_CONF_REG, GYR_CONF_NORMAL_100HZ_2000DPS);

    // Inicializa engine de features
    return initializeFeatureEngine();
}

bool BMI323::initializeFeatureEngine() {
    writeRegister16(FEATURE_CTRL_REG, 0x0001);
    delay(10);
    if (readRegister16(FEATURE_IO_STATUS_REG) & 0x01) return true;
    return false;
}

uint16_t BMI323::readRegister16(uint8_t reg) {
    uint16_t val;
    SPI.beginTransaction(spiSettings);
    digitalWrite(cs, LOW);
    SPI.transfer(reg | 0x80); // Read command
    val = (SPI.transfer(0x00) << 8) | SPI.transfer(0x00);
    digitalWrite(cs, HIGH);
    SPI.endTransaction();
    return val;
}

void BMI323::writeRegister16(uint8_t reg, uint16_t data) {
    SPI.beginTransaction(spiSettings);
    digitalWrite(cs, LOW);
    SPI.transfer(reg & 0x7F); // Write command
    SPI.transfer(data >> 8);
    SPI.transfer(data & 0xFF);
    digitalWrite(cs, HIGH);
    SPI.endTransaction();
}

float BMI323::convertAccelData(uint16_t raw) {
    return (int16_t)raw / ACC_RANGE_LSB_PER_G;
}

float BMI323::convertGyroData(uint16_t raw) {
    return (int16_t)raw / GYR_RANGE_LSB_PER_DPS;
}

float BMI323::convertTempData(uint16_t raw) {
    return (int16_t)raw * 0.0039 + 25.0;
}

bool BMI323::read(float &ax, float &ay, float &az, float &gx, float &gy, float &gz, float &temp) {
    ax = convertAccelData(readRegister16(ACC_DATA_X_REG));
    ay = convertAccelData(readRegister16(ACC_DATA_Y_REG));
    az = convertAccelData(readRegister16(ACC_DATA_Z_REG));
    gx = convertGyroData(readRegister16(GYR_DATA_X_REG));
    gy = convertGyroData(readRegister16(GYR_DATA_Y_REG));
    gz = convertGyroData(readRegister16(GYR_DATA_Z_REG));
    temp = convertTempData(readRegister16(TEMP_DATA_REG));
    return true;
}

