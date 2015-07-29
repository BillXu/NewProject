/*
 * \file: Connection.h
 * \brief: Created by hushouguo at 15:15:14 Jul 18 2012
 */
 
#ifndef __CONNECTION_H__
#define __CONNECTION_H__
#include "protocol.h"
#include "Network.h"
#define NET_SEND_BUFFER_SIZE_MAX    1*MB
	struct SocketBuffer
	{
		char*						data;
		size_t						size;
		size_t						current;
		SocketBuffer();
		SocketBuffer(size_t reserved);
		SocketBuffer(char* d, size_t len);
		~SocketBuffer();
		void						resize(size_t newsize);
		void						clear();
	};

	struct Connection
	{
		SOCKET						fd;
		size_t						recv_bytes;
		size_t						send_bytes;
	
		SocketBuffer				recv_buffer;
		SocketBuffer				send_buffer;

		bool						active;

		void						disable();

		Connection(SOCKET fd);
		~Connection();

        nullmsg*          receive();
		void						sendMsg(nullmsg* msg, size_t msgsize);
		void						sendout();
	};

#endif
