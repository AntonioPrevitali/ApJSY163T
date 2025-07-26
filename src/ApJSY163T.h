#ifndef ApJSY163T_h
#define ApJSY163T_h

/*
  ApJSY163T.h - ApJSY163T include file
  For instructions, go to https://github.com/AntonioPrevitali/ApJSY163T
  Created by Antonio Previtali, 26/07/2025.
  
  This library is free software; you can redistribute it and/or
  modify it under the terms of the Gnu general public license version 3
*/

#include "Arduino.h"


class ApJSY163T {
  public:
    ApJSY163T(void);

    void UseSerial(Stream *xserial);

    boolean okReadMeter(void);

    float   Voltage;    // Voltage
    float   Current;    // current
    float   Power;      // active power, negativa se potenza inviata in rete.
    float   Import;     // Kwh  Positive active energy
    float   Export;     // kwh  Negative active energy
    float   PowFact;    // First channel power factor
    float   Frequency;   // Frequency


  private:
    boolean  waitResponse(void);
    uint16_t calculateCrc(byte* data, size_t size);
    uint16_t bytesToUInt16(byte* regData);
    uint32_t bytesToUInt32(byte* regData);

    boolean  okTranza(void);

    Stream *_serial;

    uint8_t _buffer[25];

};


#endif
