#include <Arduino.h>

//ibrares for SD card
#include "FS.h"
#include "SD.h"
#include "SPI.h"

int32_t pin_rpm = 36;
int rpm = 0;
int pulse_count = 0;


void IRAM_ATTR funcao_ISR(){
  pulse_count++;
}

int data_rpm() {
  rpm = pulse_count * 60 / 2; // 2 pulses per rotation
  pulse_count = 0;
  return rpm;
}

// Function to write file in sd card
void writeFile(fs::FS &fs, const char * path, const char * message){
  Serial.printf("Writing file: %s\n", path);

  File file = fs.open(path, FILE_WRITE);
  if(!file){
    Serial.println("Failed to open file for writing");
    return;
  }
  if(file.print(message)){
    Serial.println("File written");
  } else {
    Serial.println("Write failed");
  }
  file.close();
}

void setup() {
  Serial.begin(115200);
  pinMode(pin_rpm, INPUT);
  attachInterrupt(pin_rpm, funcao_ISR, RISING);

  if(!SD.begin(5)){
    Serial.println("Card Mount Failed");
    return;
  }
  uint8_t cardType = SD.cardType();

  if(cardType == CARD_NONE){
    Serial.println("No SD card attached");
    return;
  }

  Serial.print("SD Card Type: ");
  if(cardType == CARD_MMC){
    Serial.println("MMC");
  } else if(cardType == CARD_SD){
    Serial.println("SDSC");
  } else if(cardType == CARD_SDHC){
    Serial.println("SDHC");
  } else {
    Serial.println("UNKNOWN");
  }

  uint64_t cardSize = SD.cardSize() / (1024 * 1024);
  Serial.printf("SD Card Size: %lluMB\n", cardSize);

}

void loop() {
  data_rpm();
  Serial.print("RPM: ");
  Serial.println(rpm);

  writeFile(SD, "/data_log.csv", "Hello\n"); // Write file in sd card

  delay(100);
}

