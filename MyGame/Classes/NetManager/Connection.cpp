/*
 * \file: Connection.cpp
 * \brief: Created by hushouguo at 15:17:29 Jul 18 2012
 */
#include "cocos2d.h"
#include "Connection.h"
#include <unistd.h>
#include <sys/socket.h>
USING_NS_CC;
	SocketBuffer::SocketBuffer()
	{
		this->data = NULL;
		this->size = 0;
		this->current = 0;
	}
	SocketBuffer::SocketBuffer(size_t reserved)
	{
		this->data = (char*)malloc(reserved);
		this->size = reserved;
		this->current = 0;
	}
	SocketBuffer::SocketBuffer(char* d, size_t len)
	{
		this->data = (char*)malloc(len);
		memcpy(this->data, d, len);
		this->size = len;
		this->current = len;
	}
	SocketBuffer::~SocketBuffer()
	{
		clear();
	}
	void SocketBuffer::resize(size_t newsize)
	{
		if (this->size < newsize)
		{
			//按4字节对齐
			newsize += 8;
			newsize &= ~0x07;

			void* p = realloc(this->data, newsize);
			assert(p != NULL);
			this->data = (char*)p;
			this->size = newsize;
		}
	}
	void SocketBuffer::clear()
	{
		if (this->data)
		{
			free(this->data);
		}
		this->data = NULL;
		this->size = 0;
		this->current = 0;
	}
	//------------------------------------------------------------------------------------
	Connection::Connection(SOCKET fd)
	{
		this->fd = fd;
		this->recv_bytes = 0;
		this->send_bytes = 0;

		this->active = true;
	
        if (fd > 0)
        {
		    setblocking(fd, false);
        }
	}
	Connection::~Connection()
	{
		close(this->fd);
	}
	void Connection::disable()
	{
		this->active = false;
	}
	static nullmsg* handle_socket_buffer(SocketBuffer* socketBuffer)
	{
		if (socketBuffer->current < PROTOCOL_LENGTH_BYTES) 
		{ 
			return NULL;
		}

		uint32_t msgSize = *(uint16_t*)&socketBuffer->data[0];

		if (socketBuffer->size < msgSize)
		{
			if (msgSize > MAX_NETWORK_PACKAGE_SIZE)
			{
				//printf("exceed package size limit:%u\n", msgSize);
				return NULL;
			}

			socketBuffer->resize(msgSize);

			return NULL;
		}

		if (socketBuffer->current < msgSize) 
		{ 
			return NULL;
		}

		//TODO: 处理压缩和加密 

        nullmsg* newmsg = (nullmsg*)malloc(msgSize);
		memcpy(newmsg, &socketBuffer->data[0], msgSize);

		if (socketBuffer->current > msgSize)
		{
			//碎包
			memmove(&socketBuffer->data[0], &socketBuffer->data[msgSize], socketBuffer->current - msgSize);
		}

		socketBuffer->current -= msgSize;

		return newmsg;
	}
    nullmsg* Connection::receive()
    {
		//粘包
        nullmsg* newmsg = handle_socket_buffer(&this->recv_buffer);
        if (newmsg) { return newmsg; }
	
		while(1)
		{
			int size = this->recv_buffer.size - this->recv_buffer.current;
			assert(size >= 0);
			if (size == 0)
			{
				size = 960;
				this->recv_buffer.resize(size); 
			}

			ssize_t n = TEMP_FAILURE_RETRY(recv(this->fd, &this->recv_buffer.data[this->recv_buffer.current], size, 0));
		
			if (n == 0) // The return value will be 0 when the peer has performed an orderly shutdown. 
            {
                //error_log("Connection lost:%d,%s\n", errno, strerror(errno));
                this->disable();
                return NULL; 
            }
			else if (n == SOCKET_ERROR)
			{
				if (interrupted()) { continue; }
				if (wouldblock())
				{
					/* on ET mode, MUST recv EAGAIN return */
					return NULL;
				}
                //CCLOG("Connection error:%d,%s\n", errno, strerror(errno));
                this->disable();
				return NULL;
			}

			this->recv_bytes += n;
			this->recv_buffer.current += n;

            newmsg = handle_socket_buffer(&this->recv_buffer);
            if (newmsg) { return newmsg; }
		}

		return NULL;
    }
	static ssize_t net_send(Connection* c, const void* d, size_t size)
	{
		size_t sendsize = 0;
	
		while(size > sendsize)
		{
#ifdef WIN32
			ssize_t n = TEMP_FAILURE_RETRY(send(c->fd, (const char*)d + sendsize, size - sendsize, 0));
#else
			ssize_t n = TEMP_FAILURE_RETRY(send(c->fd, (uint8_t*)d + sendsize, size - sendsize, 0));
#endif
	
			if (n == 0) { return SOCKET_ERROR; }
			if (n == SOCKET_ERROR)
			{
				if (interrupted()) { continue; }
				if (wouldblock())
				{
					/* on ET mode, MUST send EAGAIN return */
					return sendsize;
				}
				return SOCKET_ERROR;
			}
		
			sendsize += n;
			c->send_bytes += n;
		}

		return sendsize;
	}
	void Connection::sendMsg(nullmsg* msg, size_t msgsize)
	{
	    if (!this->active) { return; }

		printf("send %d\n",msg->cmd);
	    msg->size = msgsize;
	    if (this->send_buffer.current > 0)
	    {
		    this->send_buffer.resize(this->send_buffer.current + msg->size);
		    memcpy(&this->send_buffer.data[this->send_buffer.current], msg, msg->size);
		    this->send_buffer.current += msg->size;

		    ssize_t n = net_send(this, &this->send_buffer.data[0], this->send_buffer.current);
		    if (n == SOCKET_ERROR)
		    {
			    this->disable();
			    return;
		    }

		    size_t size = (size_t)n;
		    if (this->send_buffer.current > size && size > 0)
		    {
			    //todo: 检查发送缓冲大小如果超过阀值则剔除这个链接
			    memmove(&this->send_buffer.data[0], &this->send_buffer.data[size], this->send_buffer.current - size);
		    }

		    this->send_buffer.current -= size;
	    }
	    else
	    {
		    ssize_t n = net_send(this, msg, msg->size);
		    if (n == SOCKET_ERROR)
		    {
			    this->disable();
			    return;
		    }

		    size_t size = (size_t)n;
		    if (msg->size > size)
		    {
			    this->send_buffer.resize(this->send_buffer.current + msg->size - size);
			    memcpy(&this->send_buffer.data[this->send_buffer.current], (uint8_t*)msg + size, msg->size - size);
			    this->send_buffer.current += (msg->size - size);
		    }
	    }
	}
	void Connection::sendout()
	{
		assert(this->send_buffer.current >= 0 && this->send_buffer.size >= this->send_buffer.current);

	    while(this->send_buffer.current > 0)
	    {
			printf("send %lu",send_buffer.current);
		    ssize_t n = net_send(this, &this->send_buffer.data[0], this->send_buffer.current);
		    if (n == SOCKET_ERROR)
		    {
			    this->disable();
			    break;
		    }

		    size_t size = (size_t)n;

		    if (size == 0)
		    {
			    /* would block */
			    break;
		    }

		    if (this->send_buffer.current > size)
		    {
			    memmove(&this->send_buffer.data[0], &this->send_buffer.data[size], this->send_buffer.current - size);
		    }

		    this->send_buffer.current -= size;
	    }
	}
