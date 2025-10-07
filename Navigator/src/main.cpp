#include <Arduino.h>
#include <BME280Spi.h>
#include <BMI323.h>
#include <FreeRTOS.h>
#include <SPI.h>
#include <SparkFun_u-blox_GNSS_Arduino_Library.h>

// --- SPI pins ---
#define SPI_MISO 8
#define SPI_MOSI 11
#define SPI_SCK 10

#define BME280_CS_PIN 9
#define LIS2MD_CS_PIN 13
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

// Sensors and bus
BME280Spi::Settings bmeSettings(BME280_CS_PIN);
BME280Spi bme(bmeSettings);
BMI323 bmi(BMI323_CS_PIN, &SPI1);
SFE_UBLOX_GNSS myGNSS;

// Task handles
TaskHandle_t TaskBMEHandle;
TaskHandle_t TaskBMIHandle;
TaskHandle_t TaskGPSHandle;

void setupSPI() {
  SPI1.setSCK(SPI_SCK);
  SPI1.setTX(SPI_MOSI);
  SPI1.setRX(SPI_MISO);
  SPI1.begin();

  pinMode(BME280_CS_PIN, OUTPUT);
  pinMode(LIS2MD_CS_PIN, OUTPUT);
  pinMode(BMI323_CS_PIN, OUTPUT);
  digitalWrite(BME280_CS_PIN, HIGH);
  digitalWrite(LIS2MD_CS_PIN, HIGH);
  digitalWrite(BMI323_CS_PIN, HIGH);

  delay(100);
}

void TaskBMI(void *pvParameters) {
  (void)pvParameters;

  if (!bmi.begin()) {
    Serial.println("❌ BMI323 not found!");
  } else {
    Serial.println("✅ BMI323 initialized.");
  }

  for (;;) {
    // Aqui iriam leituras periódicas do BME323
    // Exemplo: leitura simples a cada 2.5 segundos
    Serial.println("Hello BMI Task");
    vTaskDelay(pdMS_TO_TICKS(2500));
  }
}

// Task para leitura dos sensores BME280 e BMI323
void TaskBME(void *pvParameters) {
  (void)pvParameters;

  // Inicialização dos sensores
  if (!bme.begin()) {
    Serial.println("❌ BME280 not found!");
  } else {
    Serial.println("✅ BME280 initialized.");
  }

  for (;;) {
    // Aqui iriam leituras periódicas do BME280
    // Exemplo: leitura simples a cada 3 segundos
    Serial.println("Hello BME Task");
    vTaskDelay(pdMS_TO_TICKS(3000));
  }
}

// Task para leitura do GPS
void TaskGPS(void *pvParameters) {
  // Inicialização do GPS
  Serial2.setTX(GNSS_TX);
  Serial2.setRX(GNSS_RX);

  bool connected = false;
  while (!connected) {
    Serial.println("GNSS: trying 38400 baud");
    Serial2.begin(UART1_BAUDRATE_HIGH);
    if (myGNSS.begin(Serial2)) {
      connected = true;
      break;
    }

    vTaskDelay(pdMS_TO_TICKS(100));

    Serial.println("GNSS: trying 9600 baud");
    Serial2.begin(UART1_BAUDRATE_LOW);

    if (myGNSS.begin(Serial2)) {
      Serial.println("GNSS: connected at 9600 baud, switching to 38400");

      myGNSS.setSerialRate(UART1_BAUDRATE_HIGH);
      vTaskDelay(pdMS_TO_TICKS(100));
      connected = true;
    } else {
      vTaskDelay(pdMS_TO_TICKS(2000));
    }
  }

  Serial.println("✅ GNSS serial connected");

  myGNSS.setUART1Output(COM_TYPE_UBX);
  myGNSS.saveConfiguration();

  for (;;) {

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

    vTaskDelay(pdMS_TO_TICKS(1000));
  }
}

void setup() {
  Serial.begin(UART0_BAUDRATE);

  while (!Serial) {
    tight_loop_contents();
  }

  Serial.println("Initializing sensors and GNSS...");

  setupSPI();

  // Criação das tasks --> Numero de prio maior == maior prioridade
  xTaskCreate(TaskGPS, "GPSTask", 2048, NULL, 1, &TaskGPSHandle);
  xTaskCreate(TaskBMI, "BMITask", 2048, NULL, 2, &TaskBMIHandle);
  xTaskCreate(TaskBME, "BMETask", 2048, NULL, 3, &TaskBMEHandle);

  // Inicia o scheduler FreeRTOS
  vTaskStartScheduler();
}

void loop() {
  // Deixar vazio, o FreeRTOS gerencia as tasks
}
