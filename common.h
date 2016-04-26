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
 * File: common.h
 ******************************************************************************/

#ifndef __common_h__
#define __common_h__

#include <stdint.h>

#define ATTR_NO_RETURN __attribute__( ( noreturn ) )

// TFTP flasher MAC address
extern const uint8_t MYMAC[ 6 ];

// TFTP flasher IP address
extern const uint32_t MYIP;

void app_start( void );

#endif  //__common_h__
