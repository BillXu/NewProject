#include "Server.h"
#include "MessageDefine.h"
bool CServerApp::Init()
{
	m_pNetWork = new CServerNetworkImp ;
	m_pNetWork->Start(5000);
	return true ;
}

void CServerApp::RunRoop()
{
	ProcessMsg();
}

void CServerApp::ProcessMsg()
{
	INetwork::VEC_PACKET vPacket ;
	if ( !m_pNetWork->GetAllPacket(vPacket) )
	{
		return ;
	}

	INetwork::VEC_PACKET::iterator iter = vPacket.begin();
	for ( ; iter != vPacket.end(); ++iter )
	{
		Packet* packet = *iter ;
		if ( packet->_packetType == _PACKET_TYPE_CONNECTED )
		{
			ConnectInfo* info = (ConnectInfo*)packet->_orgdata ;
			printf("connect id = %d connected a client ip = %s, port = %d \n",packet->_connectID,info->strAddress,info->nPort) ;
			SendMsg(packet->_connectID, "welcome to my server" );
		}
		else if ( _PACKET_TYPE_DISCONNECT == packet->_packetType )
		{
			ConnectInfo* info = (ConnectInfo*)packet->_orgdata ;
			printf("connect id = %d disconnect a client ip = %s, port = %d \n",packet->_connectID,info->strAddress,info->nPort) ;
		}
		else if ( _PACKET_TYPE_MSG == packet->_packetType )
		{
			stMsgText* pText = (stMsgText*)packet->_orgdata ;
			printf("recieved idx = %d, str = %s\n",pText->nIdx,pText->pText) ;
			SendMsg(packet->_connectID," I recived your msg ");
		}
		delete packet;
	}
}

void CServerApp::SendMsg(CONNECT_ID nTarget , const char* p )
{
	static unsigned int idx = 8 ;
	stMsgText msg ;
	msg.nIdx = ++idx ;
	sprintf_s(msg.pText,sizeof(msg.pText),"Server: %s",p) ;
	m_pNetWork->SendMsg((unsigned char*)&msg,sizeof(msg),nTarget);
}