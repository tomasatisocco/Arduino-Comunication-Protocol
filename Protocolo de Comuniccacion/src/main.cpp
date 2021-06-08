#include <Arduino.h>

#define LED1 2
#define LED2 3
#define LED3 4
#define LED4 5

#define SW1 6
#define SW2 7
#define SW3 8
#define SW4 9

uint8_t txbuff, checksum, i, header1 = 0xE0, header2 = 0x0E, estado, rxbuff;
uint16_t lenght = 0x0003;

void setup() {
  pinMode(LED1, OUTPUT);
  pinMode(LED2, OUTPUT);
  pinMode(LED3, OUTPUT);
  pinMode(LED4, OUTPUT);

  pinMode(SW1, INPUT);
  pinMode(SW2, INPUT);
  pinMode(SW3, INPUT);
  pinMode(SW4, INPUT);

  Serial.begin(9600);

}

void loop() {
  if (Serial.available()){
    while(Serial.available()){
      i = 1;
      rxbuff = Serial.read();
    }
    estado ^= 0x01;
    digitalWrite(LED1, estado);
  }
  if (i == 1){
    checksum = header1 + header2 + 0x3A + 0x03 + 0xF0 + 0x0D;
    Serial.write(header1);
    Serial.write(header2);
    Serial.write(0x03);
    Serial.write(0x00);
    Serial.write(0x3A);
    Serial.write(0xF0);
    Serial.write(0x0D);
    Serial.write(checksum);
    i = 0;
  }
}
