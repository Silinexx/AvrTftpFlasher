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
 * File: ipstack.c
 ******************************************************************************/

#include "ipstack.h"

#include "common.h"

uint8_t net_buf[ ENC28J60_MAXFRAME ];

EthError_t
udp_filter( eth_frame_t* frame, uint16_t len, buffer_t* buffer )
{
    uint16_t data_offset = sizeof( udp_packet_t );

    buffer->type = BDT_UDP;
    buffer->data = &frame[ data_offset ];

    if ( len >= data_offset )
    {
        return EE_OK;
    }

    return EE_INVALID_UDP_PKT;
}

EthError_t
udp_reply( eth_frame_t* frame, uint16_t len )
{
    ip_packet_t* ip = (void*)( frame->data );
    udp_packet_t* udp = (void*)( ip->data );
    uint16_t temp;

    len += sizeof( udp_packet_t );

    temp = udp->from_port;
    udp->from_port = udp->to_port;
    udp->to_port = temp;

    udp->len = htons( len );

    udp->cksum = 0;
    udp->cksum = ip_cksum( len + IP_PROTOCOL_UDP, (uint8_t*)udp - 8, len + 8 );

    return ip_reply( frame, len );
}

/*
 * ICMP
 */
#ifdef WITH_ICMP

EthError_t
icmp_filter( eth_frame_t* frame, uint16_t len, buffer_t* buffer )
{
    ip_packet_t* packet = (void*)frame->data;
    icmp_echo_packet_t* icmp = (void*)packet->data;
    uint16_t data_offset = sizeof( icmp_echo_packet_t );

    buffer->type = BDT_ICMP;
    buffer->data = &frame[ data_offset ];

    if ( len >= data_offset )
    {
        if ( icmp->type == ICMP_TYPE_ECHO_RQ )
        {
            icmp->type = ICMP_TYPE_ECHO_RPLY;
            icmp->cksum += 8;  // update cksum

            return ip_reply( frame, len );
        }

        return EE_ICMP_TYPE_UNKNOWN;
    }

    return EE_INVALID_ICMP_PKT;
}

#endif

/*
 * IP
 */
uint16_t
ip_cksum( uint32_t sum, uint8_t* buf, size_t len )
{
    while ( len >= 2 )
    {
        sum += ( (uint16_t)*buf << 8 ) | *( buf + 1 );
        buf += 2;
        len -= 2;
    }

    if ( len )
        sum += (uint16_t)*buf << 8;

    while ( sum >> 16 )
        sum = ( sum & 0xffff ) + ( sum >> 16 );

    return ~htons( (uint16_t)sum );
}

EthError_t
ip_reply( eth_frame_t* frame, uint16_t len )
{
    ip_packet_t* packet = (void*)( frame->data );

    packet->total_len = htons( len + sizeof( ip_packet_t ) );
    packet->fragment_id = 0;
    packet->flags_framgent_offset = 0;
    packet->ttl = IP_PACKET_TTL;
    packet->cksum = 0;
    packet->to_addr = packet->from_addr;
    packet->from_addr = MYIP;
    packet->cksum = ip_cksum( 0, (void*)packet, sizeof( ip_packet_t ) );

    return eth_reply( (void*)frame, len + sizeof( ip_packet_t ) );
}

EthError_t
ip_filter( eth_frame_t* frame, uint16_t len, buffer_t* buffer )
{
    ip_packet_t* packet = (void*)( frame->data );
    uint16_t data_offset = sizeof( ip_packet_t );

    buffer->type = BDT_IP;
    buffer->data = &frame[ data_offset ];

    // if(len >= sizeof(ip_packet_t))
    //{
    if ( ( packet->ver_head_len == 0x45 ) && ( packet->to_addr == MYIP ) )
    {
        len = ntohs( packet->total_len ) - data_offset;

        switch ( packet->protocol )
        {
#ifdef WITH_ICMP
        case IP_PROTOCOL_ICMP:
            return icmp_filter( frame, len, buffer );
#endif

        case IP_PROTOCOL_UDP:
            return udp_filter( frame, len, buffer );
        }

        return EE_IP_PROTOCOL_UNKNOWN;
    }

    //}

    return EE_INVALID_IP_PKT;
}

/*
 * ARP
 */
EthError_t
arp_filter( eth_frame_t* frame, uint16_t len, buffer_t* buffer )
{
    arp_message_t* msg = (void*)( frame->data );
    uint16_t data_offset = sizeof( arp_message_t );

    buffer->type = BDT_ARP;
    buffer->data = &frame[ data_offset ];

    if ( len >= data_offset )
    {
        if ( ( msg->hw_type == ARP_HW_TYPE_ETH ) && ( msg->proto_type == ARP_PROTO_TYPE_IP ) )
        {
            if ( ( msg->type == ARP_TYPE_REQUEST ) && ( msg->ip_addr_to == MYIP ) )
            {
                uint32_t ip = msg->ip_addr_to;
                msg->type = ARP_TYPE_RESPONSE;
                memcpy( msg->mac_addr_to, msg->mac_addr_from, 6 );
                memcpy( msg->mac_addr_from, MYMAC, 6 );
                msg->ip_addr_to = msg->ip_addr_from;
                msg->ip_addr_from = ip;

                return eth_reply( frame, data_offset );
            }

            return EE_ARP_REQUEST_UNKNOWN;
        }

        return EE_INVALID_ARP_REQUEST;
    }

    return EE_ERROR;
}

/*
 * Ethernet
 */
EthError_t
eth_reply( eth_frame_t* frame, uint16_t len )
{
    memcpy( frame->to_addr, frame->from_addr, 6 );
    memcpy( frame->from_addr, MYMAC, 6 );

    enc28j60_send_packet( (void*)frame, len + sizeof( eth_frame_t ) );
    return EE_OK;
}

EthError_t
eth_filter( eth_frame_t* frame, uint16_t len, buffer_t* buffer )
{
    uint16_t data_offset = sizeof( eth_frame_t );
    buffer->type = BDT_ETH;
    buffer->data = frame;

    if ( len >= data_offset )
    {
        switch ( frame->type )
        {
        case ETH_TYPE_ARP:
            return arp_filter( frame, len - data_offset, buffer );

        case ETH_TYPE_IP:
            return ip_filter( frame, len - data_offset, buffer );
        }
    }

    return EE_ETHER_TYPE_UNKNOWN;
}

EthError_t
eth_poll( buffer_t* buffer )
{
    EthError_t err = EE_NO_DATA;
    uint16_t len = 0;
    eth_frame_t* frame = (void*)net_buf;

    while ( ( len = enc28j60_recv_packet( net_buf, sizeof( net_buf ) ) ) )
    {
        buffer->buffer = frame;
        err = eth_filter( frame, len, buffer );
        if ( err > EE_ERROR )
        {
            break;
        }
    }

    return err;
}
