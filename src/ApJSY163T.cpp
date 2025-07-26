#include "ApJSY163T.h"

/*
  ApJSY163T.h - ApJSY163T Implementation file
  For instructions, go to https://github.com/AntonioPrevitali/ApJSY163T
  Created by Antonio Previtali, 26/07/2025.
  
  This library is free software; you can redistribute it and/or
  modify it under the terms of the Gnu general public license version 3
*/

ApJSY163T::ApJSY163T(void)
{
 Voltage = 0;
 Current = 0;
 Power = 0;
 Import = 0;
 Export = 0;
 PowFact = 0;
 Frequency = 0;
}


void ApJSY163T::UseSerial(Stream *xserial)
{
   _serial = xserial;
}


uint16_t ApJSY163T::calculateCrc(byte* data, size_t size)
{
    uint16_t _crc;
    _crc = 0xFFFF;
    for (size_t i = 0; i < size; i++) {
        _crc ^= data[i];
        for (byte j = 0; j < 8; j++) {
            if (_crc & 0x0001)
                _crc = (_crc >> 1) ^ 0xA001; //< fixed polynomial
            else
                _crc = _crc >> 1;
        }
    }
    return _crc >> 8 | _crc << 8;
}


uint16_t ApJSY163T::bytesToUInt16(byte* regData)
{
    uint16_t result = 0;
    for (int8_t i = 1; i >= 0; i--) {
        ((uint8_t*)&result)[i] = regData[1 - i];
    }
    return result;
}


uint32_t ApJSY163T::bytesToUInt32(byte* regData)
{
    uint32_t result = 0;
    for (int8_t i = 3; i >= 0; i--) {
        ((uint8_t*)&result)[i] = regData[3 - i];
    }
    return result;
}




boolean ApJSY163T::okTranza(void)
{
  uint8_t xreq[] = {0x01, 0x03, 0x00, 0x48, 0x00, 0x0A, 0x45, 0xDB};;  // legge tutti e 10 i registri.
  while (_serial->available()) _buffer[0] = _serial->read(); // pulisce eventuali skifezze nel buffer rx.
  _serial->write(xreq, 8);
  _serial->flush();
  if (!waitResponse()) return false;   // la risposta va tutta nel buffer e sono ben 21 caratteri
                                       // conviene fare cosi, fare letture di registri mirate (esempio power e direzione)
                                       // alla fine si impiega piu tempo...  
  return true;
}



boolean ApJSY163T::okReadMeter(void)
{
  byte*   pDat;   // un pointer sul buffer.
  uint32_t xdir;

  if (!okTranza()) return false;

  // conversione in float dei valori (dura circa 300 microsecondi le istruzioni sequenti)
  pDat = _buffer+3;
  Voltage = bytesToUInt16(pDat);
  Voltage = Voltage / 100;
  pDat += 2;
  Current = bytesToUInt16(pDat);
  Current = Current / 100;
  pDat += 2;
  Power = bytesToUInt16(pDat);
  pDat += 2;
  Import = bytesToUInt32(pDat);
  Import = Import / 3200;
  pDat += 4;
  PowFact = bytesToUInt16(pDat);
  PowFact = PowFact / 1000;
  pDat += 2;
  Export = bytesToUInt32(pDat);
  Export = Export / 3200;
  pDat += 4;
  xdir = bytesToUInt16(pDat);
  pDat += 2;
  // campo frequenza
  Frequency = bytesToUInt16(pDat);
  Frequency = Frequency / 100;       // qui 100 
  // ed ora il segno per la potenza.
  if (xdir == 1) Power = - Power;

  return true;
}




boolean ApJSY163T::waitResponse(void)
{
  unsigned long startTimeMs = millis();
  unsigned long startTimeus;
  uint16_t ilcrc;
  uint8_t numBytes = 0;
  while (!_serial->available()) {
    if (millis() - startTimeMs >= 100) {  // di solito servono circa 30ms prima che arrivi il primo carattere risposta..
      return false;                       // a sicurezza metto 100 ms che vedi anche sotto...
    }
  }
  do {
      if (_serial->available()) {
        _buffer[numBytes] = _serial->read();
        numBytes++;
      }
  } while (millis() - startTimeMs <= 120 && numBytes < 25);  // di solito per arrivare qui ed avere tutta la risposta servono circa 80 ms
                                                             // circa 30 ms prima che arrivi il primi carattere e 50 ms di comunicazione
                                                             // sto largo e metto 120 ms.

  if (numBytes < 25)
  {
    // non sono arrivati tutti i caratteri della risposta quindi non va bene ritorna false.
    return false;
  }
  // ok vediamo se il crc è giusto
  ilcrc = calculateCrc(_buffer,23);   // servono circa 160 microsecondi per calcolare questo crc
  if (_buffer[23] != highByte(ilcrc) || _buffer[24] != lowByte(ilcrc) )
  {
     return false;  // crc errato !
  }
  // vediamo anche se questi sono giusti !
  if (_buffer[0] != 1 || _buffer[1] != 3 || _buffer[2] != 20) return false;
  return true; // ok è giusto !
}

