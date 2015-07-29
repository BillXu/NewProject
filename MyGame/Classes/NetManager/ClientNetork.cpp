//
//  ClientNetork.cpp
//  MyPoker
//
//  Created by Xu BILL on 15/1/30.
//
//
//#include "Common/Common.h"
#include <stdio.h>
#include <stdlib.h>

#include "Network.h"
#include <list>
#include <unistd.h>
#include "ClientNetork.h"
#include <arpa/inet.h>
#include <netdb.h>
#include "TCPClient.h"
#include "Thread.h"
#include "protocol.h"
#include <sys/_select.h>
//#include "NetManager/NetManager.h"
#define NET_CONNECT_TIMEOUT				8

#define DEF_HEARTBEAT_INTERVAL          2

#define NET_RETRY_TIMES					1
#define NET_SELECT_TIMEOUT				DEF_HEARTBEAT_INTERVAL
#define NET_LOGIN_TIMEOUT               30
//#define NET_RETRY_INTERVAL				8

#define ACTIVE_CONNECTION(c)		(c && c->connected && c->active)
#define CLIENT_HEATBET_INTERVAL 5
#define HEATBET_TIME_OUT 18
CClientNetwork::CClientNetwork()
{
    m_pTCPConnect = NULL ;
    m_bUserShutDown = false ;
    m_nNextSendHeatBet = (time_t)-1 ;
    m_nHeatBetTimeOut = (time_t)-1 ;
    m_nNetState = _NET_STATE_DISCONNECT ;
}

CClientNetwork::~CClientNetwork()
{
    Shutdown();
}

void CClientNetwork::Init()
{
    m_nNetState = _NET_STATE_DISCONNECT ;
    m_bUserShutDown = false ;
    pthread_mutex_init(&m_lockerNetState, NULL);
    runThread();
}

bool CClientNetwork::ConnectToServer(const char* ip , unsigned short nPort)
{
    CAuotoLock t (&m_lockerNetState);
    if ( m_nNetState != _NET_STATE_DISCONNECT )
    {
        printf("Error: already connecting %d \n", m_nNetState );
        return false ;
    }
    
    const char* pConnectIP = Domain_lookup(ip);
    if ( pConnectIP == NULL )
    {
        pConnectIP = ip ;
    }
    m_strIP = pConnectIP ;
    m_nPort = nPort ;
    m_nNetState = _NET_STATE_CONNETING ;
    if ( m_bUserShutDown )
    {
        m_bUserShutDown = false ;
        this->runThread();
    }
    
    printf("request connect\n");
    return true ;
}

//bool CClientNetwork::DoConnectToServer()
//{
//    printf("tt conneting to server  %d \n",m_nNetState);
//    protocol::nullmsg* msg = new protocol::nullmsg(0) ;
//    msg->size = 0 ; // specail event ;
//    msg->cmd = _PACKET_TYPE_CONNECT_FAILED;
//    if (m_pTCPConnect) { delete m_pTCPConnect; m_pTCPConnect = NULL; }
//    
//    m_pTCPConnect = new Game::TCPClient;
//    
//    
//    if (!m_pTCPConnect->connect( m_strIP.c_str(),m_nPort, NET_CONNECT_TIMEOUT))
//    {
//        delete m_pTCPConnect ;
//        m_pTCPConnect = NULL ;
//        printf("connected failed no no \n");
//        m_tMsgQueue.push_back(msg);
//        return false ;
//    }
//    
//    
//    if (!ACTIVE_CONNECTION(m_pTCPConnect))
//    {
//        delete m_pTCPConnect ;
//        m_pTCPConnect = NULL ;
//        printf("connected failed no no 222 \n");
//        m_tMsgQueue.push_back(msg);
//        return false ;
//    }
//    
////    protocol::nullmsg* msg = new protocol::nullmsg(0) ;
////    msg->size = 0 ; // specail event ;
////    msg->cmd = _PACKET_TYPE_CONNECT_FAILED;
//    msg->cmd = _PACKET_TYPE_CONNECTED;
//    m_nNextSendHeatBet = time(NULL) + CLIENT_HEATBET_INTERVAL ;
//    printf("connected haha\n");
//    m_nHeatBetTimeOut = time(NULL) + HEATBET_TIME_OUT ;
//    m_tMsgQueue.push_back(msg);
//    return true ;
//}

bool CClientNetwork::DoConnectToServer()
{
    printf("tt conneting to server  %d \n",m_nNetState);
    
    if (m_pTCPConnect) { delete m_pTCPConnect; m_pTCPConnect = NULL; }
    
    m_pTCPConnect = new TCPClient;
    
    
    if (!m_pTCPConnect->connect( m_strIP.c_str(),m_nPort, NET_CONNECT_TIMEOUT))
    {
        delete m_pTCPConnect ;
        m_pTCPConnect = NULL ;
        printf("connected failed no no \n");
        //m_tMsgQueue.push_back(msg);
        return false ;
    }
    
    
    if (!ACTIVE_CONNECTION(m_pTCPConnect))
    {
        delete m_pTCPConnect ;
        m_pTCPConnect = NULL ;
        printf("connected failed no no 222 \n");
        //m_tMsgQueue.push_back(msg);
        return false ;
    }
    
    nullmsg* msg = new nullmsg(0) ;
    msg->size = 0 ; // specail event ;
    msg->cmd = _PACKET_TYPE_CONNECT_FAILED;
    msg->cmd = _PACKET_TYPE_CONNECTED;
    m_nNextSendHeatBet = time(NULL) + CLIENT_HEATBET_INTERVAL ;
    printf("connected haha\n");
    m_nHeatBetTimeOut = time(NULL) + HEATBET_TIME_OUT ;
    m_tMsgQueue.push_back(msg);
    return true ;
}


const char* CClientNetwork::Domain_lookup(const char* domain)
{
    static char __address_buf[32];
    struct hostent* p = gethostbyname(domain);
    if (!p)
    {
        printf("lookup domain:%s failure\n", domain);
        return NULL;
    }
    
    switch (p->h_addrtype)
    {
        case AF_INET:
        {
            char** pp = p->h_addr_list;
            for (; *pp; ++pp)
            {
                return inet_ntop(p->h_addrtype, *pp, __address_buf, sizeof(__address_buf));
            }
        }
            break;
        case AF_INET6:
            printf("NOT SUPPORT IPV6 ADDRESS FORMAT\n");
            break;
        default:
            printf("UNKNOWN ADDRESS FORMAT\n");
            break;
    }
    
    return NULL;
}


void CClientNetwork::Shutdown()
{
    if ( m_bUserShutDown )
    {
        return ;
    }
    printf("shut down net \n");
    stopThread();
    CAuotoLock t (&m_lockerNetState);
    m_nNetState = _NET_STATE_DISCONNECT ;
    m_bUserShutDown = true ;
    m_nNextSendHeatBet = (time_t)-1 ;
}

void CClientNetwork::SendMsg( const char* pBuffer, size_t msgsize)
{
    bool bC = false ;
    {
       // CAuotoLock t (&m_lockerNetState);
        bC = (m_nNetState == _NET_STATE_CONNECTED) ;
    }
    
    if ( bC == false || msgsize >= (4*1024 -2) )
    {
        return ;
    }
    
    if (ACTIVE_CONNECTION(m_pTCPConnect))
    {
        char pMaxBuffer[4*1024] = {0};
        uint16_t nSize = msgsize + sizeof(uint16_t) ;
        memcpy(pMaxBuffer + sizeof(uint16_t),pBuffer,msgsize );
        m_pTCPConnect->sendMsg((nullmsg*)pMaxBuffer, nSize );
    }
}

Packet* CClientNetwork::GetNetPacket()
{
    nullmsg* msg = m_tMsgQueue.pop_front();
    if ( msg )
    {
        Packet* p = new Packet ;
        p->cPacketType = _PACKET_TYPE_MSG ;
        if ( msg->size == 0 )  // specail event ;
        {
            p->cPacketType = msg->cmd ;
            p->InteralData = NULL ;
            p->nLen = 0 ;
        }
        else
        {
            p->nLen = msg->size - sizeof(unsigned short);
            p->InteralData = msg ;
        }
        return p ;
    }
    return NULL ;
}

void CClientNetwork::Run()
{
    printf("first urnning %d \n",m_nNetState);
    while( m_bUserShutDown == false )
    {
        ProcessNetwork();
        SLEEP_MS(20);
         
    }
    
    if ( m_pTCPConnect )
    {
        delete m_pTCPConnect ;
        m_pTCPConnect = NULL ;
        printf("delete tcp client\n");
    }
}

void CClientNetwork::ProcessNetwork()
{
    CAuotoLock t (&m_lockerNetState);
    
    if ( _NET_STATE_CONNETING == m_nNetState )
    {
        if ( DoConnectToServer() )
        {
            m_nNetState = _NET_STATE_CONNECTED ;
        }
        return ;
    }
    
    if ( _NET_STATE_DISCONNECT == m_nNetState )
    {
        return ;
    }
    
    if (!ACTIVE_CONNECTION(m_pTCPConnect) || time(NULL) >= m_nHeatBetTimeOut )
    {
        printf("lost server connection\n");
        nullmsg* msg = new nullmsg(0) ;
        msg->size = 0 ; // specail event ;
        msg->cmd = _PACKET_TYPE_DISCONNECTED;
        m_tMsgQueue.push_back(msg);
        m_nNetState = _NET_STATE_DISCONNECT ;
        return;
    }
    
    fd_set fdread;
    timeval tv;
    FD_ZERO(&fdread);
    FD_SET(m_pTCPConnect->fd, &fdread);
    
    tv.tv_sec = NET_SELECT_TIMEOUT;
    tv.tv_usec = 0;
    
    int retval = select(m_pTCPConnect->fd+1, &fdread, NULL, NULL, &tv);
    if (retval < 0)
    {
        printf("lost server connection  2 \n");
        
        nullmsg* msg = new nullmsg(0) ;
        msg->size = 0 ; // specail event ;
        msg->cmd = _PACKET_TYPE_DISCONNECTED;
        m_tMsgQueue.push_back(msg);
        m_nNetState = _NET_STATE_DISCONNECT ;
        return;
    }
    
    Heartbeat();
    if (retval == 0) { return; }
    
    if (FD_ISSET(m_pTCPConnect->fd, &fdread))
    {
        for (; ACTIVE_CONNECTION(m_pTCPConnect); )
        {
            nullmsg* msg = m_pTCPConnect->receive();
            if (msg)
            {
                if ( msg->size == sizeof(unsigned short) * 2 && msg->cmd == 0 )
                {
//                    CCLog("recived heat bet \n");
                    m_nHeatBetTimeOut = time(NULL) + HEATBET_TIME_OUT ;
                }
                else
                {
                    m_tMsgQueue.push_back(msg);
                }
            }
            else
            {
                break;
            }
        }
    }

}

void CClientNetwork::Heartbeat()
{
    time_t t = time(NULL);
    if ( t >= m_nNextSendHeatBet )
    {
        unsigned short nHeartBeat = 0 ;
        SendMsg((char*)&nHeartBeat, sizeof(nHeartBeat));
//        CCLog("send heat bet \n");
        m_nNextSendHeatBet = time(NULL) + CLIENT_HEATBET_INTERVAL ;
    }
    
}