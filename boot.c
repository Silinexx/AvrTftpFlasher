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
 * File: boot.c
 ******************************************************************************/

#include "common.h"

#include "boot.h"
#include "tftp.h"
#include "ipstack.h"
#include "enc28j60.h"

#include <avr/io.h>
#include <avr/eeprom.h>
#include <avr/interrupt.h>
#include <avr/pgmspace.h>
#include <avr/power.h>
#include <avr/wdt.h>

static uint8_t app_present = 0;

#define HW_INIT_DELAY_MS 50
#define HW_ENC_PHY_FLAGS 0x476
#define FW_UPDATE_TIMEOUT 80

/***********************************************************
 * Main function                                           *
 **********************************************************/
int
main( void )
{
    uint8_t ch = MCUSR;
    MCUSR = 0;

    WDTCSR |= _BV( WDCE ) | _BV( WDE );
    WDTCSR = 0;

    wdt_enable( WDTO_8S );

    if ( pgm_read_word( (uint8_t*)0x0 ) != 0xFFFF )
    {
        app_present = 1;
    }

    // Check if the WDT was used to reset.
    if ( !( ch & _BV( EXTRF ) ) )
    {
        app_present = 0;
    }

    if ( app_present )
    {
        wdt_disable( );
        app_start( );
    }

    // Wait to be sure that all HW has been started
    _delay_ms( HW_INIT_DELAY_MS * 5 );

    // Disable clock division
    clock_prescale_set( clock_div_1 );

    enc28j60_init( MYMAC );
    enc28j60_write_phy( PHLCON, HW_ENC_PHY_FLAGS );
    //enc28j60_write_phy(PHLCON, 0x476);

    // Wait to be sure that all HW has been started
    _delay_ms( HW_INIT_DELAY_MS );

    sei( );

    buffer_t buffer = { BDT_UNDEFINED, NULL };

    // Wait 10 seconds to upload firmware then try to start App.
    uint8_t timeout = FW_UPDATE_TIMEOUT;
    uint32_t cnt = 0;
    while ( timeout )
    {
        EthError_t result = eth_poll( &buffer );
        if ( EE_OK == result && buffer.type == BDT_UDP )
        {
            eth_frame_t* frame = (eth_frame_t*)( buffer.buffer );
            TftpError_t result = tftp_process_packet( frame );
            const TftpReply_t* reply = tftp_response( result, frame );

            // Send responce
            udp_reply( frame, reply->len );

            // Start app after last packet will be received
            if ( ( result == TFTPE_ACK_FINAL ) )
            {
                break;
            }
            else if ( result == TFTPE_ACK )
            {
                // Prevent reset by timeout during FW update
                timeout = FW_UPDATE_TIMEOUT;
            }
        }

        cnt++;
        // Check 100ms interval (Timers is not applicable on bootloader stage)
        if(cnt >= 7970)
        {
            --timeout;
            cnt = 0;
        }

        // Reset watchdog
        wdt_reset( );
    }

    wdt_disable( );

    // Start app by timeout
    app_start( );

    return 0;
}
