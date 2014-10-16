//
//  NetWorkManager.cpp
//  God
//
//  Created by Xu BILL on 12-10-30.
//
//
#include "NetWorkManager.h"
#include "RakPeerInterface.h"
#include "LogManager.h"
#include "MessageIdentifiers.h"
#include "MessageDefine.h"
int CNetWorkMgr::s_nCurrentDataSize = 0 ;
void CNetMessageDelegate::SetPriority( unsigned int nPriority )
{
	if ( nPriority == GetPriority() )
		return ;
	m_nPriority = nPriority ;
}

CNetWorkMgr::CNetWorkMgr()
{
    m_pNetPeer = NULL ;
	m_nMaxConnectTo = 0;
	m_nConnectedTo = 0 ;
}

CNetWorkMgr::~CNetWorkMgr()
{
	if ( m_pNetPeer )
	{
		RakNet::RakPeerInterface::DestroyInstance(m_pNetPeer) ;
		m_pNetPeer = NULL ;
	}
	RemoveAllDelegate() ;
}

void CNetWorkMgr::ShutDown()
{
    if ( m_pNetPeer )
    {
        m_pNetPeer->Shutdown(1);
        RakNet::RakPeerInterface::DestroyInstance(m_pNetPeer) ;
        m_pNetPeer = NULL ;
    }
}

//CNetWorkMgr* CNetWorkMgr::SharedNetWorkMgr()
//{
//    static CNetWorkMgr s_gNetWork ;
//    return &s_gNetWork ;
//}

void CNetWorkMgr::SetupNetwork( int nIntendServerCount )
{
	if ( !m_pNetPeer )
	{
		m_nMaxConnectTo = nIntendServerCount ;
		m_pNetPeer = RakNet::RakPeerInterface::GetInstance() ;
		RakNet::SocketDescriptor sDesc (0,0) ;
		m_pNetPeer->Startup(nIntendServerCount, &sDesc, 1);
		m_pNetPeer->SetMaximumIncomingConnections(0);
	}
}

bool CNetWorkMgr::ConnectToServer(const char *pSeverIP, unsigned short nPort , const char* pPassword)
{
	assert(m_pNetPeer && "Pls SetupNetwork() first! " );
    if ( !m_pNetPeer )
	{
		CLogMgr::SharedLogMgr()->ErrorLog("m_pNetPeer is null , please setup network first ");
		return false ;
	}

	assert(m_nConnectedTo < m_nMaxConnectTo && "no more slot for new coming server" );
	if ( m_nMaxConnectTo <= m_nConnectedTo )
	{
		CLogMgr::SharedLogMgr()->ErrorLog("no more slot for new coming server, so can not connected to the server: %s , port: %d",pSeverIP, nPort );
		return false ;
	}

	unsigned short nPasswordLen = 0;
	if ( pPassword && strcmp(pPassword,"0"))
	{
		nPasswordLen = strlen(pPassword) ;
	}
	else
	{
		pPassword = NULL ;
	}
	RakNet::ConnectionAttemptResult cReslt = m_pNetPeer->Connect(pSeverIP, nPort, pPassword ,nPasswordLen ) ;
    switch (cReslt)
    {
        case RakNet::CONNECTION_ATTEMPT_STARTED:
        {
            return true ;
        }
            break;
        case RakNet::INVALID_PARAMETER:
        {
            CLogMgr::SharedLogMgr()->ErrorLog("Connect to ServerFail :INVALID_PARAMETER ");
        }
            break ;
        case RakNet::CANNOT_RESOLVE_DOMAIN_NAME:
        {
            CLogMgr::SharedLogMgr()->ErrorLog("Connect to ServerFail :CANNOT_RESOLVE_DOMAIN_NAME ");
        }
            break ;
        case RakNet::ALREADY_CONNECTED_TO_ENDPOINT:
        {
            CLogMgr::SharedLogMgr()->ErrorLog("Connect to ServerFail :ALREADY_CONNECTED_TO_ENDPOINT ");
        }
            break ;
        case RakNet::CONNECTION_ATTEMPT_ALREADY_IN_PROGRESS:
        {
            CLogMgr::SharedLogMgr()->ErrorLog("Connect to ServerFail :CONNECTION_ATTEMPT_ALREADY_IN_PROGRESS ");
        }
            break ;
        case RakNet::SECURITY_INITIALIZATION_FAILED:
        {
            CLogMgr::SharedLogMgr()->ErrorLog("Connect to ServerFail :SECURITY_INITIALIZATION_FAILED ");
        }
            break ;
        default:
            break;
    }
    return false ;
}

void CNetWorkMgr::ReciveMessage()
{
    ProcessDelegateAddRemove();
	if ( m_pNetPeer == NULL )
		return ;
	RakNet::Packet* packet = NULL ;
	while ( (packet = m_pNetPeer->Receive()))
    {
        ProcessDelegateAddRemove();
        unsigned char nMessageID = packet->data[0] ;
        s_nCurrentDataSize = packet->length ;
        switch (nMessageID)
        {
            case ID_DISCONNECTION_NOTIFICATION:
            case ID_CONNECTION_LOST:
            {
				--m_nConnectedTo;
				EnumDeleagte(this, (lpfunc)(&CNetWorkMgr::OnLostServer),packet) ;
            }
                break ;
            case ID_CONNECTION_REQUEST_ACCEPTED:
            {
				++m_nConnectedTo;
				m_nCurrentServer = packet->guid ;
				CLogMgr::SharedLogMgr()->PrintLog("Connected To Server ");
				EnumDeleagte(this, (lpfunc)(&CNetWorkMgr::OnConnectSateChanged),packet) ;
            }
                break ;
            case ID_CONNECTION_ATTEMPT_FAILED:
            {
                CLogMgr::SharedLogMgr()->ErrorLog(" Cann't Connect Server ");
                EnumDeleagte(this, (lpfunc)(&CNetWorkMgr::OnConnectSateChanged),packet) ;
            }
                break ;
            case ID_NO_FREE_INCOMING_CONNECTIONS:
            {
                CLogMgr::SharedLogMgr()->ErrorLog("Server is full and busy !");
                EnumDeleagte(this, (lpfunc)(&CNetWorkMgr::OnConnectSateChanged),packet) ;
            }
                break ;
            case ID_CONNECTION_BANNED:
            {
                CLogMgr::SharedLogMgr()->ErrorLog("BANNED By targeted Server");
                EnumDeleagte(this, (lpfunc)(&CNetWorkMgr::OnConnectSateChanged),packet) ;
            }
                break ;
            case ID_INVALID_PASSWORD:
            {
                CLogMgr::SharedLogMgr()->ErrorLog("network password error!");
            }
                break;
            default:
				{
					if ( nMessageID >= ID_USER_PACKET_ENUM )
					{
//                        stMsg* pmsg = (stMsg*)packet->data ;
//                        if (MSG_PLAYER_CONTINUE_LOGIN == pmsg->usMsgType) {
//                            printf("MSG_PLAYER_CONTINUE_LOGIN");
//                        }
						EnumDeleagte(this, (lpfunc)(&CNetWorkMgr::OnReciveLogicMessage),packet) ;
					}
				}
                break;
        }
        
        m_pNetPeer->DeallocatePacket(packet) ;
    }
}

bool CNetWorkMgr::SendMsg(const char *pbuffer, int iSize)
{
    if ( m_pNetPeer == NULL )
        return false ;
    return (bool)m_pNetPeer->Send(pbuffer, iSize, IMMEDIATE_PRIORITY, RELIABLE_ORDERED, 0, m_nCurrentServer, false) ;
}

bool CNetWorkMgr::SendMsg( const char* pbuffer , int iSize,RakNet::RakNetGUID& nServerNetUID )
{
	if ( m_pNetPeer == NULL )
		return false ;
	return (bool)m_pNetPeer->Send(pbuffer, iSize, IMMEDIATE_PRIORITY, RELIABLE_ORDERED, 0, nServerNetUID, false) ;
}

void CNetWorkMgr::AddMessageDelegate(CNetMessageDelegate *pDelegate, unsigned short nPrio )
{
	if ( !pDelegate)
		return ;
	pDelegate->SetNetWorkMgr(this) ;
	pDelegate->SetPriority(nPrio) ;
    m_vWillAddDelegate.push_back(pDelegate) ;
    printf("\n加入了一个delegate.");
}

void CNetWorkMgr::AddMessageDelegate(CNetMessageDelegate *pDelegate )
{
    m_vWillAddDelegate.push_back(pDelegate) ;
}

void CNetWorkMgr::RemoveAllDelegate()
{
    m_vAllDelegate.clear() ;
}

void CNetWorkMgr::RemoveMessageDelegate(CNetMessageDelegate *pDelegate)
{
    if ( pDelegate )
    {
        m_vWillRemoveDelegate.push_back(pDelegate) ;
        printf("\n移除了一个delegate.");
    }
}

bool CNetWorkMgr::OnLostServer( CNetMessageDelegate* pDeleate,void* pData )
{
    return pDeleate->OnLostSever((RakNet::Packet*)pData) ;
}

bool CNetWorkMgr::OnReciveLogicMessage( CNetMessageDelegate* pDeleate,void* pData )
{
    return pDeleate->OnMessage((RakNet::Packet*)pData);
}

void CNetWorkMgr::EnumDeleagte( CNetWorkMgr* pTarget, lpfunc pFunc, void* pData )
{
    LIST_DELEGATE::iterator iter = m_vAllDelegate.begin() ;
    for ( ; iter != m_vAllDelegate.end(); ++iter )
    {
        if ( (pTarget->*pFunc)(*iter,pData))
            return ;
    }
}

void CNetWorkMgr::DisconnectServer( RakNet::RakNetGUID& nServerNetUID )
{
    //if ( IsConnected() )
    {
        m_pNetPeer->CloseConnection(nServerNetUID, true) ;
    }
}

bool CNetWorkMgr::OnConnectSateChanged( CNetMessageDelegate* pDeleate,void* pData )
{
	RakNet::Packet* packet = (RakNet::Packet*)pData ;
    unsigned char nMessageID = packet->data[0];
    CNetMessageDelegate::eConnectState eSate = CNetMessageDelegate::eConnect_Accepted;
    switch (nMessageID)
    {
        case ID_CONNECTION_REQUEST_ACCEPTED:
        {
            eSate = CNetMessageDelegate::eConnect_Accepted;
        }
            break ;
        case ID_CONNECTION_ATTEMPT_FAILED:
        {
            eSate = CNetMessageDelegate::eConnect_Failed;
        }
            break ;
        case ID_NO_FREE_INCOMING_CONNECTIONS:
        {
            eSate = CNetMessageDelegate::eConnect_SeverFull;
        }
            break ;
        case ID_CONNECTION_BANNED:
        {
            eSate = CNetMessageDelegate::eConnect_Banned;
        }
            break ;
        default:
            return true ;
    }
	return pDeleate->OnConnectStateChanged(eSate,packet) ;
}

void CNetWorkMgr::ProcessDelegateAddRemove()
{
    if ( m_vWillRemoveDelegate.size() > 0 )
    {
        LIST_DELEGATE::iterator iter = m_vWillRemoveDelegate.begin();
        for ( ; iter != m_vWillRemoveDelegate.end() ; ++iter )
        {
            LIST_DELEGATE::iterator iterRemove = m_vAllDelegate.begin() ;
            for ( ; iterRemove != m_vAllDelegate.end() ; ++iterRemove )
            {
                if ( *iterRemove == *iter )
                {
                    m_vAllDelegate.erase(iterRemove) ;
                    break ;
                }
            }
        }
        m_vWillRemoveDelegate.clear();
    }
    
    if ( m_vWillAddDelegate.size() > 0 )
    {
        LIST_DELEGATE::iterator iter = m_vWillAddDelegate.begin();
        CNetMessageDelegate* pDelegate = NULL ;
        for ( ; iter != m_vWillAddDelegate.end() ; ++iter )
        {
            pDelegate = *iter ;
            if ( !pDelegate)
                continue;
            pDelegate->SetNetWorkMgr(this) ;
            LIST_DELEGATE::iterator iter = m_vAllDelegate.begin();
            CNetMessageDelegate* pDelegateIter = NULL ;
            bool bInsert = false ;
            for ( ; iter != m_vAllDelegate.end(); ++iter)
            {
                pDelegateIter = *iter ;
                if ( pDelegateIter->GetPriority() > pDelegate->GetPriority() )
                {
                    m_vAllDelegate.insert(iter,pDelegate);
                    bInsert = true ;
                    break ;
                }
            }
            
            if ( bInsert == false )
            {
                m_vAllDelegate.push_back(pDelegate) ;
            }
        }
        m_vWillAddDelegate.clear();
    }
}

