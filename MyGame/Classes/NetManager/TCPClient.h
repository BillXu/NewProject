/*
 * \file: TCPClient.h
 * \brief: Created by hushouguo at 13:30:47 Mar 14 2012
 */
 
#ifndef __TCP_CLIENT_H__
#define __TCP_CLIENT_H__
#include "Network.h"
#include "Connection.h"
struct TCPClient : public Connection
{
    network_address				svraddr;
    bool						connected;
    TCPClient() : Connection(0), connected(false) {}
    bool						connect(uint32_t addr, uint16_t port, uint32_t timeout);
    bool						connect(const char* addr, uint16_t port, uint32_t timeout);
};
#endif
