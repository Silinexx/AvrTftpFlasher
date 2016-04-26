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
 * File: flash.c
 ******************************************************************************/

#include "flash.h"

#include <avr/boot.h>
#include <avr/eeprom.h>
#include <avr/interrupt.h>

uint8_t
fw_is_image_valid( uint8_t* base )
{
    // Check that a jump table is present in the first flash sector
    for ( uint8_t i = 0; i < 0x34; i += 4 )
    {
        // For each vector, check it is of the form:
        // 0x0C 0x94 0xWX 0xYZ  ; JMP 0xWXYZ
        if ( base[ i ] != 0x0C )
        {
            return 0;
        }

        if ( base[ i + 1 ] != 0x94 )
        {
            return 0;
        }
    }

    return 1;
}

void
fw_clean_first_page( void )
{
    // Erase first flash page
    boot_page_erase( 0 );

    // Wait until page is written
    boot_spm_busy_wait( );
}

void
fw_write_page( uint32_t page, uint8_t* data )
{
    uint8_t sreg;

    // Disable interrupts.
    sreg = SREG;

    cli( );

    eeprom_busy_wait( );

    boot_page_erase( page );

    // Wait until the memory is erased.
    boot_spm_busy_wait( );

    for ( uint16_t i = 0; i < FW_FLASH_PAGE_SIZE; i += 2 )
    {
        // Set up little-endian word.
        uint16_t value = *data++;
        value += ( *data++ ) << 8;

        // Fill flash page
        boot_page_fill( page + i, value );
    }

    // Store buffer in flash page.
    boot_page_write( page );

    // Wait until the memory is written.
    boot_spm_busy_wait( );

    // Enable ReadWhileWrite
    boot_rww_enable( );

    // Restore interrupts
    SREG = sreg;
}
