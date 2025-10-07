#include <Arduino.h>
// #include <SPI.h>
// #include <BME280Spi.h>
// #include <BMI323.h>
#include <SparkFun_u-blox_GNSS_Arduino_Library.h>

// --- SPI pins you want to use ---
#define SPI_MISO 8
#define SPI_MOSI 11
#define SPI_SCK 10

#define BME280_CS_PIN 9
#define BMI323_CS_PIN 25

// --- UART0 config pins ---
#define UART0_BAUDRATE 9600

#define OBC_TX 16
#define OBC_RX 17
#define OBC_CTS 18
#define OBC_RTS 19

// --- UART1 pins for GNSS ---
#define UART1_BAUDRATE_HIGH 38400
#define UART1_BAUDRATE_LOW 9600

#define GNSS_TX 4
#define GNSS_RX 5

// --- SPI bus & sensors ---
// BME280Spi::Settings bmeSettings(BME280_CS_PIN);
// BME280Spi bme(bmeSettings);
// BMI323 bmi(BMI323_CS_PIN);
SFE_UBLOX_GNSS myGNSS;

long lastTime = 0;

void setup()
{
  Serial.begin(UART0_BAUDRATE);

  // TODO: Remove this for the comms with the OBC as well as all log messages
  while (!Serial) {
    Serial.println("Waiting for serial");
    tight_loop_contents();
  }

  Serial.println("test-test");
  Serial.println("Initializing sensors and GNSS...");
  Serial.println("Test0");

  // --- SPI pin setup (Philhower core syntax) ---
  SPI1.setSCK(SPI_SCK);
  SPI1.setTX(SPI_MOSI);
  SPI1.setRX(SPI_MISO);
  SPI1.begin();

  // --- CS pins ---
  pinMode(BME280_CS_PIN, OUTPUT);
  pinMode(BMI323_CS_PIN, OUTPUT);
  digitalWrite(BME280_CS_PIN, HIGH);
  digitalWrite(BMI323_CS_PIN, HIGH);

  // --- Initialize sensors ---
  // if (!bme.begin())
  //   Serial.println("❌ BME280 not found!");
  // else
  //   Serial.println("✅ BME280 initialized.");

  // if (!bmi.begin())
  //   Serial.println("❌ BMI323 not found!");
  // else
  //   Serial.println("✅ BMI323 initialized.");

  // --- GNSS setup on UART1 (pins 4/5) ---
  // Serial2.setTX(GNSS_TX);
  // Serial.println("Teste8");
  // Serial2.setRX(GNSS_RX);
  // Serial.println("Teste9");

  Serial2.begin(UART1_BAUDRATE_HIGH);

  bool connected = false;
  while (!connected)
  {
    Serial.println("GNSS: trying 38400 baud");
    Serial2.begin(UART1_BAUDRATE_HIGH);
    if (myGNSS.begin(Serial2))
    {
      connected = true;
      break;
    }

    delay(100);
    Serial.println("GNSS: trying 9600 baud");
    Serial2.begin(UART1_BAUDRATE_LOW);
    if (myGNSS.begin(Serial2))
    {
      Serial.println("GNSS: connected at 9600 baud, switching to 38400");
      myGNSS.setSerialRate(UART1_BAUDRATE_HIGH);
      delay(100);
      connected = true;
    }
    else
    {
      delay(2000);
    }
  }

  Serial.println("✅ GNSS serial connected");

  myGNSS.setUART1Output(COM_TYPE_UBX);
  myGNSS.saveConfiguration();
}

void loop()
{
  if (millis() - lastTime > 1000)
  {
    lastTime = millis();

    long latitude = myGNSS.getLatitude();
    long longitude = myGNSS.getLongitude();
    long altitude = myGNSS.getAltitude();
    byte SIV = myGNSS.getSIV();

    Serial.print(F("Lat: "));
    Serial.print(latitude);
    Serial.print(F("  Long: "));
    Serial.print(longitude);
    Serial.print(F("  Alt: "));
    Serial.print(altitude);
    Serial.print(F("mm  SIV: "));
    Serial.println(SIV);
  }
}