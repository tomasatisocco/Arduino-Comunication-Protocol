#include <Arduino.h>

typedef union{
  struct{
    uint8_t b0: 1;
    uint8_t b1: 1;
    uint8_t b2: 1;
    uint8_t b3: 1;
    uint8_t b4: 1;
    uint8_t b5: 1;
    uint8_t b6: 1;
    uint8_t b7: 1;
  }bit;
  uint8_t byte;
}_flag;

#define   LED1  2
#define   LED2  3
#define   LED3  4
#define   LED4  5

#define   SW1   6
#define   SW2   7
#define   SW3   8
#define   SW4   9

#define   WAITINGE0   0
#define   WAITING0E   1
#define   WAITINGLB   2
#define   WAITINGHB   3
#define   WAITING3A   4
#define   WAITINGPL   5

#define   ALIVE   0xF0
#define   RIGHT   0xA0
#define   LEFT    0xA1
#define   LOWER   0xA2
#define   UPPER   0xA3
#define   SHOOT   0xD1

#define   RESET  0
#define   SAVE   1
#define   STOP   2

#define   STOPAVAILABLE  flag1.bit.b0
#define   PLAYAVAILABLE  flag1.bit.b1

void Return();
void ReadBtn();
void PutByteIntx(uint8_t byte);
void PutHeaderIntx();
void ChangeVariable();
void Shot();
void Add(uint16_t cant);
void SaveSec(uint8_t mode);
void PlaySec();

uint8_t rxBuff[32], txBuff[32], indexWriteTX, indexReadTX, indexReadRX, indexWriteRX;
uint8_t stateRead, buttons, lastButtons, checksumRX, checksumTX, state;
uint16_t lenghtPL, lenghtPLSaved, timeMemory[20];
unsigned long time, lastTimeDebounce, lastTimeRebound, lastTimeBtn, lastTimeData, lastTimePlay, lastTimeLED;
uint8_t indexTimeMemoryWrite, indexTimeMemoryRead, indexSecMemoryWrite, indexSecMemoryRead, secMemory[20], cantSec;

_flag flag1;

void ReadBtn(){
  if (digitalRead(SW1) || digitalRead(SW2) || digitalRead(SW3) || digitalRead(SW4)){
    buttons = 0x00;
    if( digitalRead(SW1))
      buttons |= 0x01;
    if( digitalRead(SW2))
      buttons |= 0x02;
    if( digitalRead(SW3))
      buttons |= 0x04;
    if( digitalRead(SW4))
      buttons |= 0x08;
    time = millis();
    if( (time - lastTimeDebounce) >= 30){
      if (buttons ^ lastButtons){
        lastButtons = buttons;
        if (lastButtons & 0x01 ){
          ChangeVariable();
        }
        if (lastButtons & 0x02 ){
          Add(1);
        }
        if (lastButtons & 0x04 ){
          Add(-1);
        }
        if (lastButtons & 0x08 ){
          Shot();
        }
        lastTimeBtn = millis();
      } else {
          lastTimeDebounce = millis();
          if ((lastTimeDebounce - lastTimeBtn) > 500){
            if (lastButtons & 0x02){
              Add(10);
            }
            if (lastButtons & 0x04){
              Add(-10);
            }
            if (lastButtons & 0x01){
              PLAYAVAILABLE = 0x01;
            }
          }
        }
      }
    } else {
      lastTimeDebounce = millis();
      lastButtons = 0x00;
   }
}

void Add(uint16_t cant){
  PutHeaderIntx();
  PutByteIntx(0x04);
  PutByteIntx(0x00);
  PutByteIntx(0x3A);
  PutByteIntx(0xD0);
  PutByteIntx(cant & 0x00FF);
  PutByteIntx(cant >> 8);
  PutByteIntx(checksumTX);
}

void ChangeVariable(){
  PutHeaderIntx();
  PutByteIntx(0x02);
  PutByteIntx(0x00);
  PutByteIntx(0x3A);
  PutByteIntx(0xD2);
  PutByteIntx(checksumTX);
}

void Shot(){
  PutHeaderIntx();
  PutByteIntx(0x02);
  PutByteIntx(0x00);
  PutByteIntx(0x3A);
  PutByteIntx(0xD1);
  PutByteIntx(checksumTX);
}

void PutByteIntx(uint8_t byte){
  txBuff[indexWriteTX++] = byte;
  indexWriteTX &= (32 - 1);
  checksumTX += byte;
}

void PutHeaderIntx(){
  txBuff[indexWriteTX++] = 0xE0;
  indexWriteTX &= (32 - 1);
  txBuff[indexWriteTX++] = 0x0E;
  indexWriteTX &= (32 - 1);
  checksumTX = 0x0E + 0xE0;
}

void SaveSec(uint8_t mode){
  switch (mode) {
    case RESET:
      for (indexSecMemoryWrite = 0; indexSecMemoryWrite >= 11; indexSecMemoryWrite++){
        secMemory[indexSecMemoryWrite] = 0x00;
        timeMemory[indexSecMemoryWrite] = 0x00;
      }
      indexTimeMemoryWrite = 0;
      indexSecMemoryWrite = 0;
      STOPAVAILABLE = 0x01;
    break;
    case SAVE:
      if (indexTimeMemoryWrite == 0){
        secMemory[indexSecMemoryWrite++] |= state;
        timeMemory[indexTimeMemoryWrite++] = 0;
      } else {
        secMemory[indexSecMemoryWrite++] |= state;
        time = millis();
        timeMemory[indexTimeMemoryWrite++] = (time - lastTimeRebound);
      }
    break;
    case STOP:
      STOPAVAILABLE = 0x00;
      cantSec = indexSecMemoryWrite;
      indexTimeMemoryRead = 0x00;
      indexSecMemoryRead = 0x00;
      PLAYAVAILABLE = 0x00;
    break;
  }
}

void PlaySec(){
  time = millis();
  if((time - lastTimePlay) >= timeMemory[indexTimeMemoryRead]){
    digitalWrite(LED1, secMemory[indexSecMemoryRead] & 0x01);
    digitalWrite(LED2, secMemory[indexSecMemoryRead] & 0x02);
    digitalWrite(LED3, secMemory[indexSecMemoryRead] & 0x04);
    digitalWrite(LED4, secMemory[indexSecMemoryRead] & 0x08);
    indexTimeMemoryRead++;
    indexSecMemoryRead++;
    if(indexTimeMemoryRead == cantSec){
      indexTimeMemoryRead = 0;
      indexSecMemoryRead = 0;
      PLAYAVAILABLE = 0x00;
    }
    lastTimePlay = millis();
    lastTimeLED = lastTimePlay;
  }
}

void Return(){
  switch (rxBuff[(indexReadRX - lenghtPLSaved)]){
    case ALIVE:
      PutHeaderIntx();
      PutByteIntx(0x03);
      PutByteIntx(0x00);
      PutByteIntx(0x3A);
      PutByteIntx(0xF0);
      PutByteIntx(0x0D);
      PutByteIntx(checksumTX);
    break;
    case SHOOT:
      if (rxBuff[(indexReadRX - (lenghtPLSaved - 1))] == 0x00){
        digitalWrite(LED_BUILTIN,LOW);
        state = 0x00;
        SaveSec(STOP);
      }
      if (rxBuff[(indexReadRX - (lenghtPLSaved - 1))] == 0x01){
        digitalWrite(LED_BUILTIN,HIGH);
        SaveSec(RESET);
      }
    break;
    case RIGHT:
      state = 0x01;
      SaveSec(SAVE);
    break;
    case LEFT:
      state = 0x02;
      SaveSec(SAVE);
    break;
    case LOWER:
      state = 0x04;
      SaveSec(SAVE);
    break;
    case UPPER:
      state = 0x08;
      SaveSec(SAVE);
    break;
  }
  lastTimeRebound = millis();
  lastTimeLED = lastTimeRebound;
  digitalWrite(LED1,state & 0x01);
  digitalWrite(LED2,state & 0x02);
  digitalWrite(LED3,state & 0x04);
  digitalWrite(LED4,state & 0x08);
}

void setup() {

  pinMode(LED1, OUTPUT);
  pinMode(LED2, OUTPUT);
  pinMode(LED3, OUTPUT);
  pinMode(LED4, OUTPUT);

  pinMode(SW1, INPUT);
  pinMode(SW2, INPUT);
  pinMode(SW3, INPUT);
  pinMode(SW4, INPUT);

  stateRead = WAITINGE0;
  Serial.begin(9600);

}

void loop() {
  ReadBtn();
  if (PLAYAVAILABLE){
    PlaySec();
  }
  while (Serial.available()){
    rxBuff[indexWriteRX++] = Serial.read();
    indexWriteRX &= (32 - 1);
  }
  if (indexReadRX != indexWriteRX){
    lastTimeData = millis();
    switch(stateRead){
      case WAITINGE0:
        if( rxBuff[indexReadRX++] == 0xE0 ){
          stateRead = WAITING0E;
        }
      break;
        case WAITING0E:
        if( rxBuff[indexReadRX++] == 0x0E ){
          stateRead = WAITINGLB;
        } else {
          stateRead = WAITINGE0;
        }
      break;
      case WAITINGLB:
        lenghtPL = rxBuff[indexReadRX];
        lenghtPLSaved = (rxBuff[indexReadRX] - 1);
        checksumRX = 0xE0 + 0x0E + rxBuff[indexReadRX];
        stateRead = WAITINGHB;
        indexReadRX++;
      break;
      case WAITINGHB:
        stateRead = WAITING3A;
        lenghtPL = lenghtPL + 256 * rxBuff[indexReadRX];
        checksumRX = checksumRX + rxBuff[indexReadRX];
        indexReadRX++;
      break;
      case WAITING3A:
        if (rxBuff[indexReadRX++] == 0x3A){
          checksumRX = checksumRX + 0x3A;
          stateRead = WAITINGPL;
        }
      break;
      case WAITINGPL:
        if (lenghtPL > 1){
          checksumRX = checksumRX + rxBuff[indexReadRX];
        }
        lenghtPL--;
        if (lenghtPL == 0){
          stateRead = WAITINGE0;
          if (checksumRX == rxBuff[indexReadRX]){
            Return();
          }
        }
        indexReadRX++;
      break;
      default:
        stateRead = WAITINGE0;
    }
    indexReadRX &= (32 - 1);
  }
  time = millis();
  if (time - lastTimeLED >= 800){
    digitalWrite(LED1,LOW);
    digitalWrite(LED2,LOW);
    digitalWrite(LED3,LOW);
    digitalWrite(LED4,LOW);
  }
  if (time - lastTimeData > 15){
    stateRead =  WAITINGE0;
  }
  if ((time - lastTimeRebound > 8000) && (STOPAVAILABLE)){
    SaveSec(STOP);
    digitalWrite(LED_BUILTIN,LOW);
  }
  if(indexWriteTX != indexReadTX){
    if(Serial.availableForWrite()){
      Serial.write(txBuff[indexReadTX++]);
      indexReadTX &= (32 - 1);
    }
  }
}
