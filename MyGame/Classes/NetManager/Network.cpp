/*
 * \file: network.c
 * \brief: Created by hushouguo at 17:15:56 Dec 03 2011
 */

#include <errno.h>
#include <sys/stat.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <sys/stat.h>
#include <fcntl.h>
#include "Network.h"

	bool setblocking(SOCKET fd, bool block)
	{
	#ifdef WIN32
		unsigned long b = block ? 0 : 1;
		if (ioctlsocket(fd, FIONBIO, &b) != 0)
        {
            return false;
        }
	#else
		int opts = fcntl(fd, F_GETFL);

		if (opts < 0)
		{
			return false;
		}
		if (block)
		{
			opts &= ~(O_NONBLOCK);
		}
		else
		{
			opts |= O_NONBLOCK;
		}
		if (fcntl(fd, F_SETFL, opts) < 0)
		{
			return false;
		}
	#endif
		return true;
	}
	bool interrupted()
	{
	#ifdef WIN32
		return WSAGetLastError() == WSAEINTR;
	#else
		return errno == EINTR;
	#endif	
	}
	#if 0
	static bool nobuffers()
	{
	#ifdef WIN32
		int error = WSAGetLastError();
		return error == WSAENOBUFS || error == WSAEFAULT;
	#else
		return errno == ENOBUFS;
	#endif	
	}
	#endif
	bool wouldblock()
	{
	#ifdef WIN32
		int error = WSAGetLastError();
		return error == WSAEWOULDBLOCK || error == WSA_IO_PENDING || error == ERROR_IO_PENDING;
	#else
		return errno == EAGAIN || errno == EWOULDBLOCK;
	#endif	
	}
	bool connectionlost()
	{
	#ifdef WIN32
		int error = WSAGetLastError();
		return error == WSAECONNRESET || error == WSAESHUTDOWN || error == WSAENOTCONN || error == WSAECONNABORTED;
	#else
		return errno == ECONNRESET || errno == ENOTCONN || errno == ESHUTDOWN || errno == ECONNABORTED || errno == EPIPE;
	#endif	
	}
#ifdef WIN32
    static WSAInitializer __wsa_initializer;
#endif
