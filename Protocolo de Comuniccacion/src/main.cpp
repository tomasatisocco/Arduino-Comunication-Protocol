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

void Respuesta();
void LeerBotones();
void PutByteIntx(uint8_t byte);

uint8_t txbuff, checksum, i, header1 = 0xE0, header2 = 0x0E, estado, rxBuff[32], txBuff[32], indexWrite, indexRead;
uint8_t stateRead, respuesta, botones, botonesAnterior;
uint16_t lenghtPL;
unsigned long tiempo, uTDebounce, uTRebote, uTBtn, uTDato;

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
          txBuff[2] = 0x02;
          txBuff[5] = 0xD2;
          txBuff[6] = 0xFC;
        }
        if (botonesAnterior & 0x02 ){
          txBuff[2] = 0x04;
          txBuff[5] = 0xD0;
          txBuff[6] = 0x01;
          txBuff[7] = 0x00;
          txBuff[8] = 0xFD;
          uTBtn = millis();
        }
        if (botonesAnterior & 0x04 ){
          txBuff[2] = 0x04;
          txBuff[5] = 0xD0;
          txBuff[6] = 0xFF;
          txBuff[7] = 0xFF;
          txBuff[8] = 0xFA;
          uTBtn = millis();
        }
        if (botonesAnterior & 0x08 ){
          txBuff[2] = 0x02;
          txBuff[5] = 0xD1;
          txBuff[6] = 0xFB;
          txBuff[7] = 0x00;
          txBuff[8] = 0x00;
        }
        indexWrite = 0x00;
        while ((indexWrite < 9) && Serial.availableForWrite()){
          Serial.write(txBuff[indexWrite]);
          indexWrite++;
        }
        } else {
          uTDebounce = millis();
          if (((botonesAnterior & 0x02) || (botonesAnterior && 0x04)) && ((uTDebounce - uTBtn) > 500)){
            txBuff[2] = 0x04;
            txBuff[5] = 0xD0;
            if (botonesAnterior & 0x02){
              txBuff[6] = 0x0A;
              txBuff[7] = 0x00;
              txBuff[8] = 0x06;
            }
            if (botonesAnterior & 0x04){
              txBuff[6] = 0xF6;
              txBuff[7] = 0xFF;
              txBuff[8] = 0xF1;
            }
            indexWrite = 0;
            while((indexWrite < 9) && Serial.availableForWrite()){
              Serial.write(txBuff[indexWrite]);
              indexWrite++;
            }
          }
        }
      }
    } else {
      uTDebounce = millis();
      botonesAnterior = 0x00;
    }
}

void PutByteIntx(uint8_t byte){
  txBuff[indexWrite++] = byte;
  indexWrite &= (32 - 1);
}

void Respuesta(){
  switch (rxBuff[0]){
    case ALIVE:
    txBuff[indexWrite++] = 0x03;
    PutByteIntx(0x03);
    PutByteIntx(0x0F);
    txBuff[indexWrite++] = 0x0D;
    txBuff[7] = 0x28;
    txBuff[8] = 0x00;
    respuesta = 0x01;
    break;
    case LANZAMIENTO:
    if (rxBuff[1] == 0x00){
      digitalWrite(LED_BUILTIN,LOW);
      estado = 0x00;
      respuesta = 0x00;
    }
    if (rxBuff[1] == 0x01){
      digitalWrite(LED_BUILTIN,HIGH);
    }
    break;
    case DERECHA:
    estado = 0x01;
    respuesta = 0x00;
    uTRebote = millis();
    break;
    case IZQUIERDA:
    estado = 0x02;
    respuesta = 0x00;
    uTRebote = millis();
    break;
    case ABAJO:
    estado = 0x04;
    respuesta = 0x00;
    uTRebote = millis();
    break;
    case ARRIBA:
    estado = 0x08;
    respuesta = 0x00;
    uTRebote = millis();
    break;
    default:
    respuesta = 0x00;
  }
  if (respuesta == 0x00){
    digitalWrite(LED1,estado & 0x01);
    digitalWrite(LED2,estado & 0x02);
    digitalWrite(LED3,estado & 0x04);
    digitalWrite(LED4,estado & 0x08);
    } else {
    indexWrite = 0x00;
    while ((indexWrite < 9) && Serial.availableForWrite() ){
      Serial.write(txBuff[indexWrite]);
      indexWrite++;
    }
  }
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
  txBuff[0] = 0xE0;
  txBuff[1] = 0x0E;
  txBuff[3] = 0x00;
  txBuff[4] = 0x3A;

  Serial.begin(9600);

}

void loop() {
  LeerBotones();
  if (Serial.available()){
    rxBuff[indexRead] = Serial.read();
    uTDato = millis();
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
          Respuesta();
          }
          indexRead = 0;
        } else {
          indexRead++;
        }
      break;
      default:
      stateRead = ESPERANDOE0;
    }
  }
  tiempo = millis();
  if (tiempo - uTRebote >= 800){
    digitalWrite(LED1,LOW);
    digitalWrite(LED2,LOW);
    digitalWrite(LED3,LOW);
    digitalWrite(LED4,LOW);
  }
  if (tiempo - uTDato > 15){
    stateRead =  ESPERANDOE0;
  }
  if(indexWrite != indexRead){
    if(Serial.availableForWrite()){
      Serial.write(txBuff[indexRead++]);
      indexRead &= (32 - 1);
    }
  }
}
