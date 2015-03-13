#include "client.h"
#include "MessageDefine.h"
//#include <time.h>
#include <time.inl>
bool CClientApp::Init()
{
	m_pNetWork = new CClientNetwork ;
	m_pNetWork->Start();
	//m_nServerID = m_pNetWork->ConnectToServer("203.186.75.136",5000);
	m_nServerID = m_pNetWork->ConnectToServer("127.0.0.1",5000);
	printf("star client\n");
	return true ;
}

void CClientApp::RunRoop()
{
	ProcessMsg();
}

void CClientApp::ProcessMsg()
{
	CClientNetwork::VEC_PACKET vPacket ;
	if ( !m_pNetWork->GetAllPacket(vPacket) )
	{
		return ;
	}

	CClientNetwork::VEC_PACKET::iterator iter = vPacket.begin();
	for ( ; iter != vPacket.end(); ++iter )
	{
		Packet* packet = *iter ;
		if ( packet->_packetType == _PACKET_TYPE_CONNECTED )
		{
			printf("connected server \n") ;
			m_nServerID = packet->_connectID ;
			SendMsg("Client: enter server haha");
		}
		else if ( _PACKET_TYPE_DISCONNECT == packet->_packetType )
		{
			printf( "disconnect a from server\n" ) ;
			m_nServerID = INVALID_CONNECT_ID ;
		}
		else if ( _PACKET_TYPE_MSG == packet->_packetType )
		{
 			stMsgText* pText = (stMsgText*)packet->_orgdata ;
 			printf("recieved idx = %d, str = %s\n",pText->nIdx,pText->pText) ;
 			SendMsg("test string");
			if ( m_nServerID != INVALID_CONNECT_ID && 0 )
			{
				LOGD("player close session");
				m_pNetWork->CloseConnection(m_nServerID);
				m_nServerID = INVALID_CONNECT_ID ;
			}
		}
		delete packet;
	}
}

void CClientApp::SendMsg(const char* p )
{
	static unsigned int idx = 8 ;
	stMsgText msg ;
	msg.nIdx = ++idx ;
	sprintf_s(msg.pText,sizeof(msg.pText),"client: %s",p) ;
	m_pNetWork->SendMsg((unsigned char*)&msg,sizeof(msg),m_nServerID);

	unsigned short nHeartBet = 0 ;
	m_pNetWork->SendMsg((unsigned char*)&nHeartBet,sizeof(nHeartBet),m_nServerID);
}