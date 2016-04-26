/*******************************************************************************
 * Atmega328p firmware updater over TFTP protocol.
 * 
 * Copyright (C) 2015-2016 Silinexx LLC, Lviv, Ukraine.
 * 
 * These coded instructions, statements, and computer programs
 * (here and after SOFTWARE) are copyrighted by Silinexx LLC
 * and published as open-source software. This meant that you can use
 * and/or modify it only for personal purposes. Any comertial usage
 * and/or publishing of this SOFTWARE in any form, in whole or in part,
 * should not being done without the specific, prior written permission
 * of Silinexx LLC.
 * 
 * This program is distributed in the hope that it will be useful but
 * WITHOUT ANY WARRANTY, even without warranty of
 * FITNESS FOR A PARTICULAR PURPOSE.
 ******************************************************************************/

/******************************************************************************
 * Author: Andrian Yablonskyy
 * Date: 2015-07-22
 * Email: andrian.yablonskyy@silinexx.com
 * File: common.c
 ******************************************************************************/

#include "common.h"

#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/wdt.h>
#include <avr/eeprom.h>

#ifndef sbi
#define sbi( sfr, bit ) ( _SFR_BYTE( sfr ) |= _BV( bit ) )
#endif

const uint8_t MYMAC[ 6 ] = {0x00, 0x11, 0x22, 0x33, 0x44, 0x55};

// Define a static IP address for tftp flash
// 192.168.1.10 => 10 << 24 | 1 << 16 | 168 << 8 | 192 => 0x0a01a8c0
const uint32_t MYIP = 0x0A01A8C0;

void
app_start( )
{
    wdt_disable( );
    cli( );

    // LED_Y_ON();
    MCUCR = 0;
    MCUCR = ( 1 << IVCE );

    __asm__ volatile( "jmp 0x0000" );
}
