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

#define ALIVE 0xF0
#define DERECHA 0xA0
#define IZQUIERDA 0xA1
#define ABAJO 0xA2
#define ARRIBA 0xA3
#define LANZAMIENTO 0xD1

#define RESET 0
#define SAVE 1
#define STOP 2

void Respuesta();
void LeerBotones();
void PutByteIntx(uint8_t byte);
void PutHeaderIntx();
void ChangeVariable();
void Shot();
void Add(uint16_t cant);
void SaveSec(uint8_t mode);
void PlaySec();

uint8_t rxBuff[32], txBuff[32], indexWriteTX, indexReadTX, indexReadRX, indexWriteRX;
uint8_t stateRead, botones, botonesAnterior, checksumRX, checksumTX, estado;
uint16_t lenghtPL, lenghtPLSaved;
unsigned long tiempo, uTDebounce, uTRebote, uTBtn, uTDato, uTPlay, uTLED;
uint16_t timeMemory[20];
uint8_t indexTimeMemoryWrite, indexTimeMemoryRead, indexSecMemoryWrite, indexSecMemoryRead, play, stop, secMemory[20], cantSec;

void LeerBotones(){
  if (digitalRead(SW1) || digitalRead(SW2) || digitalRead(SW3) || digitalRead(SW4)){
    botones = 0x00;
    if( digitalRead(SW1))
      botones |= 0x01;
    if( digitalRead(SW2))
      botones |= 0x02;
    if( digitalRead(SW3))
      botones |= 0x04;
    if( digitalRead(SW4))
      botones |= 0x08;
    tiempo = millis();
    if( (tiempo - uTDebounce) >= 30){
      if (botones ^ botonesAnterior){
        botonesAnterior = botones;
        if (botonesAnterior & 0x01 ){
          ChangeVariable();
        }
        if (botonesAnterior & 0x02 ){
          Add(1);
        }
        if (botonesAnterior & 0x04 ){
          Add(-1);
        }
        if (botonesAnterior & 0x08 ){
          Shot();
        }
        uTBtn = millis();
      } else {
          uTDebounce = millis();
          if ((uTDebounce - uTBtn) > 500){
            if (botonesAnterior & 0x02){
              Add(10);
            }
            if (botonesAnterior & 0x04){
              Add(-10);
            }
            if (botonesAnterior & 0x01){
              play = 0x01;
            }
          }
        }
      }
    } else {
      uTDebounce = millis();
      botonesAnterior = 0x00;
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
      stop = 0x01;
    break;
    case SAVE:
      if (indexTimeMemoryWrite == 0){
        secMemory[indexSecMemoryWrite] |= estado;
        indexSecMemoryWrite++;
        timeMemory[indexTimeMemoryWrite] = 0;
        indexTimeMemoryWrite++;
      } else {
        secMemory[indexSecMemoryWrite] |= estado;
        indexSecMemoryWrite++;
        tiempo = millis();
        timeMemory[indexTimeMemoryWrite] = (tiempo - uTRebote);
        indexTimeMemoryWrite++;
      }
    break;
    case STOP:
      stop = 0x00;
      cantSec = indexSecMemoryWrite;
      indexTimeMemoryRead = 0x00;
      indexSecMemoryRead = 0x00;
      play = 0x00;
    break;
  }
}

void PlaySec(){
  tiempo = millis();
  if((tiempo - uTPlay) >= timeMemory[indexTimeMemoryRead]){
    digitalWrite(LED1, secMemory[indexSecMemoryRead] & 0x01);
    digitalWrite(LED2, secMemory[indexSecMemoryRead] & 0x02);
    digitalWrite(LED3, secMemory[indexSecMemoryRead] & 0x04);
    digitalWrite(LED4, secMemory[indexSecMemoryRead] & 0x08);
    indexTimeMemoryRead++;
    indexSecMemoryRead++;
    if(indexTimeMemoryRead == cantSec){
      indexTimeMemoryRead = 0;
      indexSecMemoryRead = 0;
      play = 0x00;
    }
    uTPlay = millis();
    uTLED = uTPlay;
  }
}

void Respuesta(){
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
    case LANZAMIENTO:
      if (rxBuff[(indexReadRX - (lenghtPLSaved - 1))] == 0x00){
  //      digitalWrite(LED_BUILTIN,LOW);
        estado = 0x00;
        SaveSec(STOP);
      }
      if (rxBuff[(indexReadRX - (lenghtPLSaved - 1))] == 0x01){
//        digitalWrite(LED_BUILTIN,HIGH);
        SaveSec(RESET);
      }
    break;
    case DERECHA:
      estado = 0x01;
      SaveSec(SAVE);
    break;
    case IZQUIERDA:
      estado = 0x02;
      SaveSec(SAVE);
    break;
    case ABAJO:
      estado = 0x04;
      SaveSec(SAVE);
    break;
    case ARRIBA:
      estado = 0x08;
      SaveSec(SAVE);
    break;
  }
  uTRebote = millis();
  uTLED = uTRebote;
  digitalWrite(LED1,estado & 0x01);
  digitalWrite(LED2,estado & 0x02);
  digitalWrite(LED3,estado & 0x04);
  digitalWrite(LED4,estado & 0x08);
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

  stateRead = ESPERANDOE0;
  Serial.begin(9600);

}

void loop() {
  LeerBotones();
  if (play == 0x01){
    PlaySec();
  }
  while (Serial.available()){
    rxBuff[indexWriteRX++] = Serial.read();
    indexWriteRX &= (32 - 1);
  }
  if (indexReadRX != indexWriteRX){
    uTDato = millis();
    switch(stateRead){
      case ESPERANDOE0:
        if( rxBuff[indexReadRX++] == 0xE0 ){
          stateRead = ESPERANDO0E;
        }
      break;
        case ESPERANDO0E:
        if( rxBuff[indexReadRX++] == 0x0E ){
          stateRead = ESPERANDOLB;
        } else {
          stateRead = ESPERANDOE0;
        }
      break;
      case ESPERANDOLB:
        lenghtPL = rxBuff[indexReadRX];
        lenghtPLSaved = (rxBuff[indexReadRX] - 1);
        checksumRX = 0xE0 + 0x0E + rxBuff[indexReadRX];
        stateRead = ESPERANDOHB;
        indexReadRX++;
      break;
      case ESPERANDOHB:
        stateRead = ESPERANDO3A;
        lenghtPL = lenghtPL + 256 * rxBuff[indexReadRX];
        checksumRX = checksumRX + rxBuff[indexReadRX];
        indexReadRX++;
      break;
      case ESPERANDO3A:
        if (rxBuff[indexReadRX++] == 0x3A){
          checksumRX = checksumRX + 0x3A;
          stateRead = ESPERANDOPL;
        }
      break;
      case ESPERANDOPL:
        if (lenghtPL > 1){
          checksumRX = checksumRX + rxBuff[indexReadRX];
        }
        lenghtPL--;
        if (lenghtPL == 0){
          stateRead = ESPERANDOE0;
          if (checksumRX == rxBuff[indexReadRX]){
            Respuesta();
          }
        }
        indexReadRX++;
      break;
      default:
        stateRead = ESPERANDOE0;
    }
    indexReadRX &= (32 - 1);
  }
  tiempo = millis();
  if (tiempo - uTLED >= 800){
    digitalWrite(LED1,LOW);
    digitalWrite(LED2,LOW);
    digitalWrite(LED3,LOW);
    digitalWrite(LED4,LOW);
  }
  if (tiempo - uTDato > 15){
    stateRead =  ESPERANDOE0;
  }
  if ((tiempo - uTRebote > 8000) && (stop = 0x01)){
    SaveSec(STOP);
  }
  if(indexWriteTX != indexReadTX){
    if(Serial.availableForWrite()){
      Serial.write(txBuff[indexReadTX++]);
      indexReadTX &= (32 - 1);
    }
  }
}
