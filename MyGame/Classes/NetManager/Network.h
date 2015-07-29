/*
 * \file: network.h
 * \brief: Created by hushouguo at 17:15:54 Dec 03 2011
 */
 
#ifndef __NETMANAGER_NETWORK_H__
#define __NETMANAGER_NETWORK_H__

#ifdef _MSC_VER
#pragma warning(disable:4514)
#endif

#ifdef _MSC_VER
#pragma comment(lib, "ws2_32.lib")
#endif

	#ifndef WIN32
//		#define SOCKET int
    typedef int SOCKET;
        #define SOCKET_ERROR -1
		#define INVALID_SOCKET -1
	#endif

	#ifndef SHUT_RD
		#define SHUT_RD 0
	#endif

	#ifndef SHUT_WR
		#define SHUT_WR 1
	#endif

	#ifndef SHUT_RDWR
		#define SHUT_RDWR 2
	#endif

	#ifdef LINUX
	#define ERRNO   errno
	#endif
	#ifdef WIN32
	#define ERRNO   WSAGetLastError()
	#endif

	#ifdef WIN32
	#define close	closesocket
	#endif

	#ifdef WIN32
	typedef long ssize_t;
	#endif

	//#ifdef WIN32
	#define TEMP_FAILURE_RETRY(FUNCTION) FUNCTION
	//#endif

	struct network_address
	{
		uint32_t					ip;
		uint16_t					port;
	};

	bool setblocking(SOCKET fd, bool block);
	bool interrupted();
	bool wouldblock();
	bool connectionlost();

#ifdef WIN32
    class WSAInitializer
    {
        public:
            WSAInitializer() 
            {
                if (WSAStartup(0x101,&m_wsadata)) 
                {
                    exit(-1);
                }
            }
            ~WSAInitializer() 
            {
                WSACleanup();
            }
        private:
            WSADATA m_wsadata;
    };
#endif

#endif // __NETMANAGER_NETWORK_H__
