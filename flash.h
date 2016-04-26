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
 * File: flash.h
 ******************************************************************************/

#ifndef __flash_h__
#define __flash_h__

#include <avr/pgmspace.h>

#define FW_PAGE PGM_P
#define FW_FLASH_PAGE_SIZE SPM_PAGESIZE
#define FW_START_ADDR BOOT_START_ADDR
#define FW_UNDEFINED_VALUE 0xFF

// Validate uploaded image
uint8_t fw_is_image_valid( uint8_t* base );

// Write page into controller memory
void fw_write_page( uint32_t page, uint8_t* data );

// Clean up first program page
void fw_clean_first_page( void );

#endif  // __flash_h__
