/*
 * \file: TCPClient.cpp
 * \brief: Created by hushouguo at 13:30:50 Mar 14 2012
 */
#include <unistd.h>
#include <netdb.h>
#include "TCPClient.h"
#include "cocos2d.h"
#include <arpa/inet.h>
USING_NS_CC;

	bool TCPClient::connect(uint32_t addr, uint16_t port, uint32_t timeout)
	{
		struct sockaddr_in serveraddr;

		this->connected = false;

		this->fd = socket(AF_INET, SOCK_STREAM, 0);
		if (this->fd < 0)
		{
			return false;
		}

		memset(&serveraddr, 0, sizeof(serveraddr));
		serveraddr.sin_family = AF_INET;
		/*inet_aton(s_addr, &(localaddr.sin_addr));*/
		serveraddr.sin_addr.s_addr = addr;
		serveraddr.sin_port = port;

        setblocking(this->fd, false);
		if (::connect(this->fd, (sockaddr*)&serveraddr, sizeof(struct sockaddr_in)) < 0)
		{
            fd_set fdread;
            fd_set fdwrite;
            timeval tv;

            FD_ZERO(&fdread);
            FD_ZERO(&fdwrite);
            FD_SET(this->fd, &fdread);
            FD_SET(this->fd, &fdwrite);

            tv.tv_sec = timeout;
            tv.tv_usec = 0;

            int retval = select(this->fd+1, &fdread, &fdwrite, NULL, &tv);
            if (retval < 0)
            {
                //printf("select operate failure:%d,%s\n", errno, strerror(errno));
                close(this->fd);
                return false;
            }
            else if (retval == 0) 
            {
                //printf("connect timeout, unreachable server %s:%d\n", inet_ntoa(*(struct in_addr*)&addr), ntohs(port));
                close(this->fd);
                return false;
            }
            else
            {
                if (FD_ISSET(this->fd, &fdwrite))
                {
                    if (FD_ISSET(this->fd, &fdread))
                    {
                        //TODO: NOT SURE, as a failure case
                        //printf("connect exception, unreachable server %s:%d\n", inet_ntoa(*(struct in_addr*)&addr), ntohs(port));
                        close(this->fd);
                        return false;
                    }
                    else
                    {
                        //CCLOG("connect success\n");
                    }
                }
                else
                {
                    //printf("connect failure, unreachable server %s:%d\n", inet_ntoa(*(struct in_addr*)&addr), ntohs(port));
                    close(this->fd);
                    return false;
                }
            }

            /*
			fd_set fdwrite;
			timeval tv;

			FD_ZERO(&fdwrite);
			FD_SET(this->fd, &fdwrite);

			tv.tv_sec = timeout;
			tv.tv_usec = 0;

			int retval = select(this->fd+1, NULL, &fdwrite, NULL, &tv);
			if (retval < 0)
			{
				close(this->fd);
				return false;
			}
			else if (retval == 0) 
			{
				close(this->fd);
				return false;
			}
            */
		}

		this->svraddr.ip = addr;
		this->svraddr.port = port;

		return this->connected = true;
	}
	bool TCPClient::connect(const char* addr, uint16_t port, uint32_t timeout)
	{
		return this->connect(inet_addr(addr), htons(port), timeout);

    }
