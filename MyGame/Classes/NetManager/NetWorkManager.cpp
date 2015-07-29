//
//  NetWorkManager.cpp
//  God
//
//  Created by Xu BILL on 12-10-30.
//
//
#include "NetWorkManager.h"
//#include "LogManager.h"
//#include "MessageDefine.h"
#include "cocos2d.h"
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
        m_pNetPeer->Shutdown();
		delete m_pNetPeer;
		m_pNetPeer = NULL ;
	}
	RemoveAllDelegate() ;
}

void CNetWorkMgr::ShutDown()
{
    if ( m_pNetPeer )
    {
        m_pNetPeer->Shutdown();
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
		m_pNetPeer =  new CClientNetwork ;
        m_pNetPeer->Init();
	}
}

bool CNetWorkMgr::ConnectToServer(const char *pSeverIP, unsigned short nPort , const char* pPassword)
{
	assert(m_pNetPeer && "Pls SetupNetwork() first! " );
    if ( !m_pNetPeer )
	{
		//CLogMgr::SharedLogMgr()->ErrorLog("m_pNetPeer is null , please setup network first ");
		return false ;
	}

	assert(m_nConnectedTo < m_nMaxConnectTo && "no more slot for new coming server" );
	if ( m_nMaxConnectTo <= m_nConnectedTo )
	{
		//CLogMgr::SharedLogMgr()->ErrorLog("no more slot for new coming server, so can not connected to the server: %s , port: %d",pSeverIP, nPort );
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

	return  m_pNetPeer->ConnectToServer(pSeverIP, nPort) ;
}

void CNetWorkMgr::ReciveMessage()
{
    ProcessDelegateAddRemove();
	if ( m_pNetPeer == NULL )
		return ;
	
    Packet* NetPacket = NULL;
    while ( ( NetPacket = m_pNetPeer->GetNetPacket() ))
    {
        if ( _PACKET_TYPE_CONNECTED == NetPacket->cPacketType )
        {
//            printf("_PACKET_TYPE_CONNECTED == NetPacket->cPacketType\n");
            EnumDeleagte(this, (lpfunc)(&CNetWorkMgr::OnConnectSateChanged),NetPacket) ;
        }
        else if ( _PACKET_TYPE_DISCONNECTED == NetPacket->cPacketType )
        {
            EnumDeleagte(this, (lpfunc)(&CNetWorkMgr::OnLostServer),NetPacket) ;
        }
        else if ( _PACKET_TYPE_CONNECT_FAILED == NetPacket->cPacketType )
        {
            EnumDeleagte(this, (lpfunc)(&CNetWorkMgr::OnConnectSateChanged),NetPacket) ;
        }
        else if ( _PACKET_TYPE_MSG == NetPacket->cPacketType )
        {
            ProcessDelegateAddRemove();
            s_nCurrentDataSize = NetPacket->nLen ;
            EnumDeleagte(this, (lpfunc)(&CNetWorkMgr::OnReciveLogicMessage),NetPacket) ;
        }
     
        delete NetPacket;
    }
}

void CNetWorkMgr::ReciveOneMessage()
{
    ProcessDelegateAddRemove();
    if ( m_pNetPeer == NULL )
        return ;
    
    Packet* NetPacket = NetPacket = m_pNetPeer->GetNetPacket();
    if (NetPacket == nullptr )
    {
        return ;
    }
    
    if ( _PACKET_TYPE_CONNECTED == NetPacket->cPacketType )
    {
        //            printf("_PACKET_TYPE_CONNECTED == NetPacket->cPacketType\n");
        EnumDeleagte(this, (lpfunc)(&CNetWorkMgr::OnConnectSateChanged),NetPacket) ;
    }
    else if ( _PACKET_TYPE_DISCONNECTED == NetPacket->cPacketType )
    {
        EnumDeleagte(this, (lpfunc)(&CNetWorkMgr::OnLostServer),NetPacket) ;
    }
    else if ( _PACKET_TYPE_CONNECT_FAILED == NetPacket->cPacketType )
    {
        EnumDeleagte(this, (lpfunc)(&CNetWorkMgr::OnConnectSateChanged),NetPacket) ;
    }
    else if ( _PACKET_TYPE_MSG == NetPacket->cPacketType )
    {
        ProcessDelegateAddRemove();
        s_nCurrentDataSize = NetPacket->nLen ;
        EnumDeleagte(this, (lpfunc)(&CNetWorkMgr::OnReciveLogicMessage),NetPacket) ;
    }
    
    delete NetPacket;
}

bool CNetWorkMgr::SendMsg(const char *pbuffer, int iSize)
{
    if ( m_pNetPeer == NULL )
        return false ;
	m_pNetPeer->SendMsg(pbuffer, iSize);
    return true ;
}

void CNetWorkMgr::AddMessageDelegate(CNetMessageDelegate *pDelegate, unsigned short nPrio )
{
	if ( !pDelegate)
		return ;
	pDelegate->SetNetWorkMgr(this) ;
	pDelegate->SetPriority(nPrio) ;
    m_vWillAddDelegate.push_back(pDelegate) ;
//    printf("\n加入了一个delegate.");
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
//        printf("\n移除了一个delegate.");
    }
}

bool CNetWorkMgr::OnLostServer( CNetMessageDelegate* pDeleate,void* pData )
{
    return pDeleate->OnLostSever() ;
}

bool CNetWorkMgr::OnReciveLogicMessage( CNetMessageDelegate* pDeleate,void* pData )
{
    Packet* packet = (Packet*)pData ;
    stMsg* pmsg = (stMsg*)packet->Data();
    return pDeleate->OnMessage(pmsg);
}

void CNetWorkMgr::EnumDeleagte( CNetWorkMgr* pTarget, lpfunc pFunc, void* pData )
{
    //printf("have accpeted state EnumDeleagte delegate cnt %lu \n",m_vAllDelegate.size());
    LIST_DELEGATE::iterator iter = m_vAllDelegate.begin() ;
    for ( ; iter != m_vAllDelegate.end(); ++iter )
    {
        if ( (pTarget->*pFunc)(*iter,pData))
            return ;
    }
}

bool CNetWorkMgr::OnConnectSateChanged( CNetMessageDelegate* pDeleate,void* pData )
{
	Packet* packet = (Packet*)pData ;
    //unsigned char nMessageID = packet->data[0];
    CNetMessageDelegate::eConnectState eSate = CNetMessageDelegate::eConnect_Accepted;
     switch (packet->cPacketType)
     {
         case _PACKET_TYPE_CONNECTED:
         {
             eSate = CNetMessageDelegate::eConnect_Accepted;
//             printf("have accpeted state OnConnectSateChanged\n");
         }
             break ;
         case _PACKET_TYPE_CONNECT_FAILED:
         {
            eSate = CNetMessageDelegate::eConnect_Failed;
         }
             break ;
         default:
             return true ;
     }
	return pDeleate->OnConnectStateChanged(eSate) ;
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
			if ( IsAlreadyAdded(pDelegate) )
			{
				//CLogMgr::SharedLogMgr()->PrintLog("duplicate add network delegate") ;
				continue;
			}
            pDelegate->SetNetWorkMgr(this) ;
            LIST_DELEGATE::iterator iter = m_vAllDelegate.begin();
            CNetMessageDelegate* pDelegateIter = NULL ;
            bool bInsert = false ;
            for ( ; iter != m_vAllDelegate.end(); ++iter)
            {
                pDelegateIter = *iter ;
                if ( pDelegateIter->GetPriority() <= pDelegate->GetPriority() )
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

bool CNetWorkMgr::IsAlreadyAdded(CNetMessageDelegate*pDeleate)
{
	LIST_DELEGATE::iterator iter = m_vAllDelegate.begin();
	for ( ;iter != m_vAllDelegate.end(); ++iter )
	{
		if ( *iter == pDeleate )
		{
			return true ;
		}
	}
	return false ;
}

