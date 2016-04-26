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
 * File: tftp.c
 ******************************************************************************/

#include "tftp.h"

#include "common.h"
#include "boot.h"
#include "enc28j60.h"

#define TFTP_DATA_SIZE 512

/*
 * TFTP
 */
typedef struct tftp_packet
{
    uint16_t opcode;
    uint16_t block;
    uint8_t data[];
} tftp_packet_t;

const TftpReply_t tftp_reply_map[ TFTPE_MAX ] = {
    {4, "\0\4\0\0"},
    {4, "\0\4\0\0"},
    {10, "\0\5\0\0Error"},
    {18, "\0\5\0\0Access denied"},
    {14, "\0\5\0\3No memory"},
    {18, "\0\5\0\0Invalid image"},
};

uint16_t received_packet_number = 0;
uint8_t download_started = 0;

TftpError_t
tftp_process_packet( eth_frame_t* frame )
{
    ip_packet_t* ip = (void*)( frame->data );
    udp_packet_t* udp = (void*)( ip->data );
    tftp_packet_t* tftp_data = (void*)( udp->data );

    uint16_t packet_length = ntohs( udp->len ) - 12;  // (UDP + TFTP) header length = 12 bytes

    tftp_data->opcode = ntohs( tftp_data->opcode );
    tftp_data->block = ntohs( tftp_data->block );

    switch ( tftp_data->opcode )
    {
    // Handle TFTP Read request
    case TFTP_OPCODE_RRQ:
    default:
        // ReadRequest is not supported.
        return TFTPE_ACCESS_DENIED;

    // Handle TFTP Write request
    case TFTP_OPCODE_WRQ:
        fw_clean_first_page( );

        received_packet_number = 0;

        // Indicate that we ready to receive data
        return TFTPE_ACK;

    case TFTP_OPCODE_DATA:
    {
        received_packet_number = tftp_data->block;

        // Calculate write address for this block inside chip flash memory
        uint16_t write_addr = ( tftp_data->block - 1 ) << 9;

        if ( ( write_addr + packet_length ) > FW_START_ADDR )
        {
            // Indicate memory overflow
            return TFTPE_NO_MEMORY;
        }

        uint8_t* page_buffer = tftp_data->data;  // Start of block data
        uint16_t offset = 0;                // Block offset
        uint16_t buffer_length = packet_length;

        // Round up packet length to a full flash sector size
        while ( buffer_length % FW_FLASH_PAGE_SIZE )
        {
            tftp_data->data[ buffer_length ] = FW_UNDEFINED_VALUE;
            buffer_length++;
        }

        if ( write_addr == 0 )
        {
            // First sector - validate
            if ( !fw_is_image_valid( page_buffer ) )
            {
                return TFTPE_INVALID_IMG;
            }
        }

        for ( offset = 0; offset < buffer_length; offset += FW_FLASH_PAGE_SIZE )
        {
            fw_write_page( write_addr + offset, page_buffer + offset );
        }

        if ( buffer_length < TFTP_DATA_SIZE )
        {
            // Flash is complete
            // Hand over to application
            return TFTPE_ACK_FINAL;
        }

        download_started = 1;

        // Wait next packet
        return TFTPE_ACK;
    }

    case TFTP_OPCODE_ACK:    // Acknowledgement
    case TFTP_OPCODE_ERROR:  // Error signal
        break;
    }

    return TFTPE_ERROR;
}

uint8_t
tftp_is_download_started( void )
{
    return download_started;
}

const TftpReply_t*
tftp_response( TftpError_t response, eth_frame_t* frame )
{
    ip_packet_t* ip = (void*)( frame->data );
    udp_packet_t* udp = (void*)( ip->data );

    const TftpReply_t* reply = &tftp_reply_map[ TFTPE_ERROR ];

    if ( response > TFTPE_MAX )
    {
        response = TFTPE_ERROR;
    }

    reply = &tftp_reply_map[ response ];

    // Copy reply message
    for ( uint8_t n = 0; n < reply->len; n++ )
    {
        udp->data[ n ] = reply->msg[ n ];
    }

    if ( response == TFTPE_ACK || response == TFTPE_ACK_FINAL )
    {
        // Write block num
        udp->data[ 2 ] = received_packet_number >> 8;
        udp->data[ 3 ] = received_packet_number & 0xFF;
    }

    return reply;
}
