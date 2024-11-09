#include <Arduino.h>

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

void setup() {
  Serial.begin(115200);
  pinMode(pin_rpm, INPUT);
  attachInterrupt(pin_rpm, funcao_ISR, RISING);  
}

void loop() {
  data_rpm();
  Serial.println(rpm);
  delay(100);
}

