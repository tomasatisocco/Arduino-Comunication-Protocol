#include <Arduino.h>

#define LED1 2
#define LED2 3
#define LED3 4
#define LED4 5

#define SW1 6
#define SW2 7
#define SW3 8
#define SW4 9

#define ESPERANDOE0 0
#define ESPERANDO0E 1
#define ESPERANDOLB 2
#define ESPERANDOHB 3
#define ESPERANDO3A 4
#define ESPERANDOPL 5


uint8_t txbuff, checksum, i, header1 = 0xE0, header2 = 0x0E, estado, rxBuff[16], txBuff[16], indexWrite, indexRead;
uint8_t stateRead;
uint16_t lenght = 0x0003, lenghtPL;

void setup() {
  pinMode(LED1, OUTPUT);
  pinMode(LED2, OUTPUT);
  pinMode(LED3, OUTPUT);
  pinMode(LED4, OUTPUT);

  pinMode(SW1, INPUT);
  pinMode(SW2, INPUT);
  pinMode(SW3, INPUT);
  pinMode(SW4, INPUT);

  stateRead = ESPERANDOE0;

  Serial.begin(9600);

}

void loop() {
  if (Serial.available()){
    rxBuff[indexRead] = Serial.read();
    switch(stateRead){
      case ESPERANDOE0:
      if( rxBuff[indexRead] == 0xE0 ){
        stateRead = ESPERANDO0E;
      }
      break;
      case ESPERANDO0E:
      if( rxBuff[indexRead] == 0x0E ){
        stateRead = ESPERANDOLB;
      } else {
        stateRead = ESPERANDOE0;
      }
      break;
      case ESPERANDOLB:
      lenghtPL = rxBuff[indexRead];
      checksum = 0xE0 + 0x0E + rxBuff[indexRead];
      stateRead = ESPERANDOHB;
      break;
      case ESPERANDOHB:
      stateRead = ESPERANDO3A;
      lenghtPL = lenghtPL + 256 * rxBuff[indexRead];
      checksum = checksum + rxBuff[indexRead];
      break;
      case ESPERANDO3A:
      if (rxBuff[indexRead] == 0x3A){
        checksum = checksum + 0x3A;
        stateRead = ESPERANDOPL;
      }
      break;
      case ESPERANDOPL:
      if (lenghtPL > 1){
        checksum = checksum + rxBuff[indexRead];
      }
      lenghtPL--;
      if (lenghtPL == 0){
        stateRead = ESPERANDOE0;
        if (checksum == rxBuff[indexRead]){
          switch (rxBuff[0]){
            case 0xF0:
            Serial.write(header1);
            Serial.write(header2);
            Serial.write(0x03);
            Serial.write(0x00);
            Serial.write(0x3A);
            Serial.write(0xF0);
            Serial.write(0x0D);
            Serial.write(0x28);
            break;
          }
          indexRead = 0;
        }
      }
      indexRead++;
      break;
      default:
      stateRead = ESPERANDOE0;
    }
  }
}
