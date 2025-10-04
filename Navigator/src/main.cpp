#include <Arduino.h>
#include <SPI.h>
#include <BME280Spi.h>
#include <BMI323.h>

#define SPI_MISO 8
#define SPI_SCK 10
#define SPI_MOSI 11

#define BME280_CS_PIN 9 // GPIO 9
#define BMI323_CS_PIN 25 // GPIO 25

//  ---  Sensor Unit 1  ---


BME280Spi::Settings settings(BME280_CS_PIN); // Default : forced mode, standby time = 1000 ms
                                             //           Oversampling = pressure ×1, temperature ×1, humidity ×1, filter off,
BME280Spi bme(settings);

BMI323 bmi(BMI323_CS_PIN);

// --- GNSS ---
#include <SparkFun_u-blox_GNSS_Arduino_Library.h> //http://librarymanager/All#SparkFun_u-blox_GNSS
SFE_UBLOX_GNSS myGNSS;

#include <Serial.h>
Serial mySerial(10, 11); 

long lastTime = 0; //Simple local timer. Limits amount of I2C traffic to u-blox module.

void setup()
{
    // Iniciar e configurar interface SPI no setup
    SPI.begin(SPI_SCK, SPI_MISO, SPI_MOSI); // SCK, MISO, MOSI, SS

    // Inicializar o sensor LIS2MDL com SPI e pino CS definido
    //LIS2MDLSensor Magneto(&dev_spi, CS_PIN);

    //Magneto.begin();
    //Magneto.Enable();

  Serial.begin(115200);
  while (!Serial); //Wait for user to open terminal
  Serial.println("SparkFun u-blox Example");

  //Assume that the U-Blox GNSS is running at 9600 baud (the default) or at 38400 baud.
  //Loop until we're in sync and then ensure it's at 38400 baud.
  do {
    Serial.println("GNSS: trying 38400 baud");
    mySerial.begin(38400);
    if (myGNSS.begin(mySerial) == true) break;

    delay(100);
    Serial.println("GNSS: trying 9600 baud");
    mySerial.begin(9600);
    if (myGNSS.begin(mySerial) == true) {
        Serial.println("GNSS: connected at 9600 baud, switching to 38400");
        myGNSS.setSerialRate(38400);
        delay(100);
    } else {
        //myGNSS.factoryReset();
        delay(2000); //Wait a bit before trying again to limit the Serial output
    }
  } while(1);
  Serial.println("GNSS serial connected");

  myGNSS.setUART1Output(COM_TYPE_UBX); //Set the UART port to output UBX only
  myGNSS.setI2COutput(COM_TYPE_UBX); //Set the I2C port to output UBX only (turn off NMEA noise)
  myGNSS.saveConfiguration(); //Save the current settings to flash and BBR
}

void loop()
{
  //Query module only every second. Doing it more often will just cause I2C traffic.
  //The module only responds when a new position is available
  if (millis() - lastTime > 1000)
  {
    lastTime = millis(); //Update the timer
    
    long latitude = myGNSS.getLatitude();
    Serial.print(F("Lat: "));
    Serial.print(latitude);

    long longitude = myGNSS.getLongitude();
    Serial.print(F(" Long: "));
    Serial.print(longitude);
    Serial.print(F(" (degrees * 10^-7)"));

    long altitude = myGNSS.getAltitude();
    Serial.print(F(" Alt: "));
    Serial.print(altitude);
    Serial.print(F(" (mm)"));

    byte SIV = myGNSS.getSIV();
    Serial.print(F(" SIV: "));
    Serial.print(SIV);

    Serial.println();
  }
}