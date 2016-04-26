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
 * File: ipstack.h
 ******************************************************************************/

#ifndef __ipstack_h__
#define __ipstack_h__

#include "enc28j60.h"

#include <string.h>

typedef enum
{
    EE_OK = 0,
    EE_NO_DATA,
    EE_ERROR,
    EE_MAC_UNKNOWN,
    EE_ETHER_TYPE_UNKNOWN,
    EE_INVALID_ARP_REQUEST,
    EE_ARP_REQUEST_UNKNOWN,
    EE_INVALID_IP_PKT,
    EE_IP_PROTOCOL_UNKNOWN,
    EE_INVALID_ICMP_PKT,
    EE_ICMP_TYPE_UNKNOWN,
    EE_INVALID_UDP_PKT,
    EE_INVALID_DST_PORT,
    EE_ERR_MAX
} EthError_t;

// Enable ICMP support
#define WITH_ICMP

// Define TCP/IP packet TTL
#define IP_PACKET_TTL 64

// Endianes conversion macros
#define htons( a ) ( ( ( ( a ) >> 8 ) & 0xff ) | ( ( ( a ) << 8 ) & 0xff00 ) )
#define ntohs( a ) htons( a )
#define htonl( a )                                                                             \
    ( ( ( ( a ) >> 24 ) & 0xff ) | ( ( ( a ) >> 8 ) & 0xff00 ) | ( ( ( a ) << 8 ) & 0xff0000 ) \
      | ( ( ( a ) << 24 ) & 0xff000000 ) )
#define ntohl( a ) htonl( a )

// =================================================================================================
// Ethernet frame data types
// =================================================================================================

// Define ARP frame type
#define ETH_TYPE_ARP htons( 0x0806 )

// Define IP frame type
#define ETH_TYPE_IP htons( 0x0800 )

// DAta type to handle ethernet frame
typedef struct
{
    uint8_t to_addr[ 6 ];    // Destination MAC address
    uint8_t from_addr[ 6 ];  // Source MAC address
    uint16_t type;           // Frame type
    uint8_t data[];          // Pointer to frame data
} eth_frame_t;

// =================================================================================================
// ARP frame data types
// =================================================================================================

#define ARP_HW_TYPE_ETH htons( 0x0001 )
#define ARP_PROTO_TYPE_IP htons( 0x0800 )
#define ARP_TYPE_REQUEST htons( 1 )
#define ARP_TYPE_RESPONSE htons( 2 )

typedef struct
{
    uint16_t hw_type;
    uint16_t proto_type;
    uint8_t hw_addr_len;
    uint8_t proto_addr_len;
    uint16_t type;
    uint8_t mac_addr_from[ 6 ];
    uint32_t ip_addr_from;
    uint8_t mac_addr_to[ 6 ];
    uint32_t ip_addr_to;
} arp_message_t;

// =================================================================================================
// IP frame data types
// =================================================================================================
#define IP_PROTOCOL_ICMP 1
#define IP_PROTOCOL_TCP 6
#define IP_PROTOCOL_UDP 17

typedef struct
{
    uint8_t ver_head_len;
    uint8_t tos;
    uint16_t total_len;
    uint16_t fragment_id;
    uint16_t flags_framgent_offset;
    uint8_t ttl;
    uint8_t protocol;
    uint16_t cksum;
    uint32_t from_addr;
    uint32_t to_addr;
    uint8_t data[];
} ip_packet_t;

// =================================================================================================
// ICMP frame data types
// =================================================================================================
#define ICMP_TYPE_ECHO_RQ 8
#define ICMP_TYPE_ECHO_RPLY 0

typedef struct
{
    uint8_t type;
    uint8_t code;
    uint16_t cksum;
    uint16_t id;
    uint16_t seq;
    uint8_t data[];
} icmp_echo_packet_t;

// =================================================================================================
// UDP frame data types
// =================================================================================================
typedef struct udp_packet
{
    uint16_t from_port;
    uint16_t to_port;
    uint16_t len;
    uint16_t cksum;
    uint8_t data[];
} udp_packet_t;

typedef enum
{
    BDT_UNDEFINED = -1,
    BDT_ETH = 0,
    BDT_ARP,
    BDT_IP,
    BDT_ICMP,
    BDT_UDP,
    
    BDT_MAX
} buffer_data_type_t;

typedef struct
{
    buffer_data_type_t type;
    void* data;
    void* buffer;
} buffer_t;

EthError_t udp_reply( eth_frame_t* frame, uint16_t len );
EthError_t eth_reply( eth_frame_t* frame, uint16_t len );
EthError_t ip_reply( eth_frame_t* frame, uint16_t len );
uint16_t ip_cksum( uint32_t sum, uint8_t* buf, uint16_t len );

// =================================================================================================
// LAN
// =================================================================================================
EthError_t eth_poll( buffer_t* data );

#endif  //__ipstack_h__
