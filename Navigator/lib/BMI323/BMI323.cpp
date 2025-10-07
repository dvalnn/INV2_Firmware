#include "BMI323.h"

BMI323::BMI323(uint8_t csPin, SPIClass *spiDevice = &SPI) : spi(csPin, SPI_FREQ, MSBFIRST, SPI_MODE3, spiDevice) {}

bool BMI323::begin() {
    return initBMI323();
}

bool BMI323::initBMI323() {
    // Reset
    writeRegister16(CMD_REG, SOFT_RESET_CMD);
    delay(50);

    uint16_t chip_id = readRegister8(CHIP_ID_REG);
    Serial.printf("chip id is %d\n", chip_id);

    // Verifica CHIP ID
    if (chip_id != 0x323) {
        Serial.println("AAAAAAAHHHHHH");
        return false;
    }

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

uint8_t BMI323::readRegister8(uint8_t addr) {
  uint8_t ret = 0;
  readRegisterN(addr, &ret, 1);

  return ret;
}

uint16_t BMI323::readRegister16(uint8_t addr) {
  uint8_t buffer[2] = {0, 0};
  readRegisterN(addr, buffer, 2);

  uint16_t ret = buffer[0];
  ret <<= 8;
  ret |= buffer[1];

  return ret;
}

void BMI323::readRegisterN(uint8_t addr, uint8_t buffer[], uint8_t n){
  addr &= 0x7F; // MSB=0 for read, make sure top bit is not set

  spi.write_then_read(&addr, 1, buffer, n);
}

void BMI323::writeRegister8(uint8_t addr, uint8_t data) {
  addr |= 0x80; // MSB=1 for write, make sure top bit is set

  uint8_t buffer[2] = {addr, data};

  spi.write(buffer, 2);
}

void BMI323::writeRegister16(uint8_t addr, uint16_t data){
  addr |= 0x80; // MSB=1 for write, make sure top bit is set

  uint8_t buffer[3] = {addr, (uint8_t)(data >> 8), (uint8_t)(data & 0xFF)};

  spi.write(buffer, 3);
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

