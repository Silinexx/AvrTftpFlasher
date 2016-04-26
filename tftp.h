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
 * File: tftp.h
 ******************************************************************************/

#ifndef __tftp_h__
#define __tftp_h__

// TFTP Opcode values from RFC 1350
#define TFTP_OPCODE_RRQ 1
#define TFTP_OPCODE_WRQ 2
#define TFTP_OPCODE_DATA 3
#define TFTP_OPCODE_ACK 4
#define TFTP_OPCODE_ERROR 5

#include "ipstack.h"
#include "flash.h"


typedef enum
{
    TFTPE_ACK = 0,
    TFTPE_ACK_FINAL,
    TFTPE_ERROR,
    TFTPE_ACCESS_DENIED,
    TFTPE_NO_MEMORY,
    TFTPE_INVALID_IMG,

    TFTPE_MAX,
} TftpError_t;

typedef struct
{
    uint8_t len;
    FW_PAGE msg;
} TftpReply_t;

uint8_t is_download_started( void );
TftpError_t tftp_process_packet( eth_frame_t* frame );
const TftpReply_t* tftp_response( TftpError_t response, eth_frame_t* frame );

#endif  //__tftp_h__
