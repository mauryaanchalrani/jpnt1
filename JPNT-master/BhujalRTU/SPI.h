/*
 * Copyright (c) 2010 by Cristian Maglie <c.maglie@bug.st>
 * SPI Master library for arduino.
 *
 * This file is free software; you can redistribute it and/or modify
 * it under the terms of either the GNU General Public License version 2
 * or the GNU Lesser General Public License version 2.1, both as
 * published by the Free Software Foundation.
 */

#ifndef _SPI_H_INCLUDED
#define _SPI_H_INCLUDED

#include <stdio.h>
//#include <Arduino.h>
#include <avr/pgmspace.h>

#define SPI_CLOCK_DIV4 0x00
#define SPI_CLOCK_DIV16 0x01
#define SPI_CLOCK_DIV64 0x02
#define SPI_CLOCK_DIV128 0x03
#define SPI_CLOCK_DIV2 0x04
#define SPI_CLOCK_DIV8 0x05
#define SPI_CLOCK_DIV32 0x06
//#define SPI_CLOCK_DIV64 0x07

#define SPI_MODE0 0x00
#define SPI_MODE1 0x04
#define SPI_MODE2 0x08
#define SPI_MODE3 0x0C

#define SPI_MODE_MASK 0x0C  // CPOL = bit 3, CPHA = bit 2 on SPCR
#define SPI_CLOCK_MASK 0x03  // SPR1 = bit 1, SPR0 = bit 0 on SPCR
#define SPI_2XCLOCK_MASK 0x01  // SPI2X = bit 0 on SPSR

// class SPIClass {
// public:
unsigned char SPI_transfer(unsigned char _data);

// SPI Configuration methods

void SPI_attachInterrupt();
void SPI_detachInterrupt(); // Default

   void SPI_begin(); // Default
	void SPI_start();
   void SPI_end();

   void SPI_setBitOrder(void);
   void SPI_setDataMode(uint8_t);
   void SPI_setClockDivider(uint8_t);
#if defined(__arm__) && defined(CORE_TEENSY)
   void SPI_setMOSI(uint8_t pin) __attribute__((always_inline)) { SPCR.setMOSI(pin); }
   void SPI_setMISO(uint8_t pin) __attribute__((always_inline)) { SPCR.setMISO(pin); }
   void SPI_setSCK(uint8_t pin) __attribute__((always_inline)) { SPCR.setSCK(pin); }
#endif
//};

//extern SPIClass SPI;

#endif
