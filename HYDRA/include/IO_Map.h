#ifndef IO_MAP_H
#define IO_MAP_H

//* Actuation Unit
#define SOL_VALVE_1 18
#define SOL_VALVE_2 19
#define SOL_VALVE_3 20
#define QDC_N2O 21
#define QDC_N2 22
#define ST_VALVE_1 23
#define ST_VALVE_2 24
#define CAM_EN 25
#define PWM_SIG 3

//* Sensor Unit
#define THERMO1_DRDY 7
#define THERMO2_DRDY 12
#define THERMO3_DRDY 17
#define AD5593R_RST 16

#define FAULT_STATES 27
#define TEMP_SENSE 26

//* I2C
#define I2C_SDA 14
#define I2C_SCL 15

//* SPI
#define SPI_MISO 8
#define SPI_MOSI 11
#define SPI_SCK 10

#define THERMO1_CS 9
#define THERMO2_CS 13
#define THERMO3_CS 29

//* STATUS
#define RED_STATUS 0
#define GREEN_STATUS 1
#define BUZZER_PWM 2

//* RS-485
#define WRITE_RS 4
#define READ_RS 5
#define ENABLE_RS 6

#endif  // IO_MAP_H