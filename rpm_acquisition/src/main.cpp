#include <Arduino.h>
#include "FS.h"
#include "SD.h"
#include "SPI.h"
#include "RTClib.h" // Include the RTClib for DS1307 RTC

const int pin_rpm = 36;  // Pino de entrada do RPM
const int SD_CS_PIN = 5; // Pino CS (Chip Select) para o cartão SD
int rpm = 0;
int pulse_count = 0;
char buffer[40];

// Create RTC object
RTC_DS1307 rtc;

void IRAM_ATTR funcao_ISR(){
  pulse_count++;
}

int data_rpm() {
  rpm = pulse_count * 60 / 2; // 2 pulsos por rotação
  pulse_count = 0;
  return rpm;
}

void writeFile(fs::FS &fs, const char * path, const char * message){
  Serial.printf("Escrevendo arquivo: %s\n", path);

  File file = fs.open(path, FILE_APPEND);  // FILE_APPEND para não sobrescrever
  if(!file){
    Serial.println("Falha ao abrir o arquivo para escrita");
    return;
  }
  if(file.println(message)){
    Serial.println("Arquivo gravado com sucesso");
  } else {
    Serial.println("Falha na gravação");
  }
  file.close();
}

void setup() {
  Serial.begin(115200);

    // Initialize the RTC
  if (!rtc.begin()) {
    Serial.println("Couldn't find RTC");
    while (1);
  }

  // Check if the RTC is running, if not set the current date and time
  if (!rtc.isrunning()) {
    rtc.adjust(DateTime(F(_DATE), F(TIME_)));  // Set RTC to compile date and time
  }

  pinMode(pin_rpm, INPUT);
  attachInterrupt(pin_rpm, funcao_ISR, RISING);

  // Initialize SPI with defined pins
  SPI.begin(18, 19, 23, SD_CS_PIN);

  // Initialize SD card
  if (!SD.begin(SD_CS_PIN)) {
    Serial.println("Falha ao montar o cartão SD!");
    return;
  }

  uint8_t cardType = SD.cardType();
  if (cardType == CARD_NONE) {
    Serial.println("Nenhum cartão SD encontrado");
    return;
  }

  Serial.print("Tipo de Cartão SD: ");
  if (cardType == CARD_MMC) {
    Serial.println("MMC");
  } else if (cardType == CARD_SD) {
    Serial.println("SDSC");
  } else if (cardType == CARD_SDHC) {
    Serial.println("SDHC");
  } else {
    Serial.println("Desconhecido");
  }

  uint64_t cardSize = SD.cardSize() / (1024 * 1024);
  Serial.printf("Tamanho do Cartão SD: %lluMB\n", cardSize);

// RTC MODULE

  // Create and write header in the file if necessary
  File file = SD.open("/data_log.csv", FILE_READ);
  if (!file) {
    Serial.println("Criando arquivo data_log.csv...");
    writeFile(SD, "/data_log.csv", "RPM,Time");  // Header
  } else {
    file.close();
  }
}

void loop() {
  data_rpm();
  Serial.print("RPM: ");
  Serial.println(rpm);

  // Get the current time from the RTC
  DateTime now = rtc.now();

  // sprintf(buffer, "%d", rpm);
  // writeFile(SD, "/data_log.csv", buffer); // Escreve o valor de RPM

  // sprintf(buffer, "%c", ",");
  // writeFile(SD, "/data_log.csv", buffer); // Escreve o valor de ,

  // sprintf(buffer, "%02d:%02d:%02d", now.hour(),now.minute(),now.second());
  // writeFile(SD, "/data_log.csv", buffer); // Escreve o valor de RPM

// Format the RPM and timestamp (HH:MM:SS) into a single string for logging
  sprintf(buffer, "%d, %02d:%02d:%02d", rpm, now.hour(), now.minute(), now.second());

  // Write the formatted string to the SD card
  writeFile(SD, "/data_log.csv", buffer); // Save RPM with time on a single line

  delay(400);  // Delay to prevent overload

  // Show SD card space info
  Serial.printf("Espaço total: %lluMB\n", SD.totalBytes() / (1024 * 1024));
  Serial.printf("Espaço usado: %lluMB\n", SD.usedBytes() / (1024 * 1024));
}