#include "ServerNetwork.h"
#include "LogManager.h"
#include "MessageIdentifiers.h"
void CServerNetworkDelegate::SetPriority(unsigned int nPriority )
{
	if ( nPriority == m_nPriority )
		return ;
	m_nPriority = nPriority ;
	CServerNetwork::SharedNetwork()->RemoveDelegate(this);
	CServerNetwork::SharedNetwork()->AddDelegate(this,nPriority);
}

CServerNetwork* CServerNetwork::SharedNetwork()
{
	static CServerNetwork g_sNetwork ;
	return &g_sNetwork ;
}

CServerNetwork::CServerNetwork()
{
	m_pNetPeer = NULL;
	m_vAllDelegates.clear() ;
}

CServerNetwork::~CServerNetwork()
{
	ShutDown();
}

bool CServerNetwork::StartupNetwork( unsigned short nPort , int nMaxInComming ,const char* pIncomingPassword )
{
	assert(m_pNetPeer == NULL && nMaxInComming > 0 && "m_pNetPeer Must NULL" ) ;
	if ( m_pNetPeer )
	{
		ShutDown();
	}

	m_pNetPeer = RakNet::RakPeerInterface::GetInstance();
	RakNet::SocketDescriptor socketDes( nPort,0);
	RakNet::StartupResult StartResult = m_pNetPeer->Startup(nMaxInComming,&socketDes,1) ;
	m_pNetPeer->SetMaximumIncomingConnections(nMaxInComming);
	if ( StartResult != RakNet::RAKNET_STARTED )
	{
		CLogMgr::SharedLogMgr()->ErrorLog( "Can not Start ServerNetwork , Error ID = %d",StartResult );
		return false ;
	}
	if ( pIncomingPassword && strcmp(pIncomingPassword,"0") )
	{
		m_pNetPeer->SetIncomingPassword(pIncomingPassword,strlen(pIncomingPassword)) ;
	}
	return  true;
}

void CServerNetwork::ShutDown()
{
	if ( m_pNetPeer == NULL)
		return ;
	m_vAllDelegates.clear() ;
	m_pNetPeer->Shutdown(2);
	RakNet::RakPeerInterface::DestroyInstance(m_pNetPeer);
	m_pNetPeer = NULL ;
}

void CServerNetwork::RecieveMsg()
{
	if ( m_pNetPeer == NULL )
	{
		return ;
	}

	RakNet::Packet* packet = NULL ;
	while ( packet = m_pNetPeer->Receive() )
	{
		switch( packet->data[0] )
		{
		case ID_NEW_INCOMING_CONNECTION:
			{
				// new perr connected ;
				EnumDelegate(&CServerNetwork::OnNewPeerConnected,packet);
			}
			break; 
		case ID_DISCONNECTION_NOTIFICATION:
		case ID_CONNECTION_LOST:
			{
				// a client peer connection lost ;
				EnumDelegate(&CServerNetwork::OnPeerDisconnected,packet);
			}
			break; 
		default:
			{
				if ( packet->data[0] >= ID_USER_PACKET_ENUM )
				{
					// a logic message ;
					// tell delegates ;
					EnumDelegate(&CServerNetwork::OnLogicMessage,packet);
				}
			}
			break;
		}
		m_pNetPeer->DeallocatePacket(packet);
	}
}

void CServerNetwork::SendMsg(const char* pData , int nLength , RakNet::RakNetGUID& nSendToOrExcpet ,bool bBroadcast )
{
	if ( !m_pNetPeer )
		return ;
	m_pNetPeer->Send(pData,nLength,IMMEDIATE_PRIORITY,RELIABLE_ORDERED,0,nSendToOrExcpet,bBroadcast);
}

void CServerNetwork::ClosePeerConnection(RakNet::RakNetGUID& nPeerToClose)
{
	if ( !m_pNetPeer )
		return ;
	m_pNetPeer->CloseConnection(nPeerToClose,true);
	// tell delegate ;
	RakNet::Packet packet ;
	packet.guid = nPeerToClose ;
	packet.data = NULL ;
	EnumDelegate(&CServerNetwork::OnPeerDisconnected,&packet);
}

void CServerNetwork::AddDelegate(CServerNetworkDelegate* pDelegate , unsigned int nPriority /* = 0 */ )
{
	LIST_DELEGATE::iterator iter = m_vAllDelegates.begin() ;
	for ( ; iter != m_vAllDelegates.end(); ++iter )
	{
		if ( (*iter)->GetPriority() <= nPriority )
		{
			m_vAllDelegates.insert(iter,pDelegate);
			return ;
		}
	}
	m_vAllDelegates.push_back(pDelegate);
}

void CServerNetwork::RemoveDelegate(CServerNetworkDelegate* pDelegate )
{
	LIST_DELEGATE::iterator iter = m_vAllDelegates.begin();
	for ( ; iter != m_vAllDelegates.end(); ++iter )
	{
		if ( (*iter) == pDelegate )
		{
			m_vAllDelegates.erase(iter);
			return ;
		}
	}
}

bool CServerNetwork::OnNewPeerConnected(CServerNetworkDelegate* pDelegate, RakNet::Packet* pData )
{
	pDelegate->OnNewPeerConnected(pData->guid,pData);
	return true ;
}

bool CServerNetwork::OnPeerDisconnected(CServerNetworkDelegate* pDelegate, RakNet::Packet* pData )
{
	pDelegate->OnPeerDisconnected(pData->guid,pData);
	return true ;
}

bool CServerNetwork::OnLogicMessage(CServerNetworkDelegate* pDelegate, RakNet::Packet* pData)
{
	return pDelegate->OnMessage(pData);
}

void CServerNetwork::EnumDelegate( lpFunc pFunc, RakNet::Packet* pData )
{
	LIST_DELEGATE::iterator iter = m_vAllDelegates.begin();
	for ( ; iter != m_vAllDelegates.end(); ++iter )
	{
		if ((this->*pFunc)(*iter,pData))
		{
			return ;
		}
	}
}
