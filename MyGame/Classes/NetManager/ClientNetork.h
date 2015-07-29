//
//  ClientNetork.h
//  MyPoker
//
//  Created by Xu BILL on 15/1/30.
//
//

#ifndef __MyPoker__ClientNetork__
#define __MyPoker__ClientNetork__

#include <stdio.h>
#include "Thread.h"
#include "MessageQueue.h"
#include <string>
struct TCPClient;
#include "protocol.h"
class CClientNetwork;
struct Packet
{
    unsigned char cPacketType;
#define _PACKET_TYPE_CONNECTED 1 
#define _PACKET_TYPE_DISCONNECTED 2
#define _PACKET_TYPE_CONNECT_FAILED 3
#define _PACKET_TYPE_MSG 5
    unsigned short nLen ;
    char* Data(){ if (InteralData == NULL) return NULL ;  char* pD = (char*)InteralData ; pD = pD + sizeof(unsigned short) ; return pD; }
    ~Packet(){ delete InteralData ; InteralData = NULL ;}
private:
    friend class CClientNetwork ;
    nullmsg* InteralData ;
};

class CAuotoLock
{
public:
    CAuotoLock(pthread_mutex_t* locker ):m_pLocker(locker)
    {
        pthread_mutex_lock(m_pLocker);
    }
    ~CAuotoLock()
    {
        pthread_mutex_unlock( m_pLocker );
    }
protected:
    pthread_mutex_t* m_pLocker ;
};

class CClientNetwork
    :public Misc::Thread
{
public:
    CClientNetwork();
    ~CClientNetwork();
    void Init();
    bool ConnectToServer(const char* ip , unsigned short nPort);
    void Shutdown();
    void SendMsg( const char* pBuffer, size_t msgsize);
    void Run();
    void Heartbeat();
    Packet* GetNetPacket();  // must delete out side ;
protected:
    const char* Domain_lookup(const char* domain);
    bool DoConnectToServer();
    void ProcessNetwork();
protected:
    MessageQueue   m_tMsgQueue;
    TCPClient*			 m_pTCPConnect;
    bool m_bUserShutDown;
    time_t m_nNextSendHeatBet ;
    time_t m_nHeatBetTimeOut ;
    unsigned char m_nNetState ;
    std::string m_strIP ;
    unsigned short m_nPort ;
#define _NET_STATE_DISCONNECT 1
#define _NET_STATE_CONNETING 2
#define _NET_STATE_CONNECTED 3
    pthread_mutex_t						m_lockerNetState;
};

#endif /* defined(__MyPoker__ClientNetork__) */
