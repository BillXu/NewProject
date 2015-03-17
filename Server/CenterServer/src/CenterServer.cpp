#include "CenterServer.h"
#include "LogManager.h"
#include "ServerMessageDefine.h"
CCenterServerApp* CCenterServerApp::s_GateServer = NULL ;
CCenterServerApp* CCenterServerApp::SharedCenterServer()
{
	return s_GateServer;
}

CCenterServerApp::CCenterServerApp()
{
	m_pNetwork = NULL ;
	m_vGateInfos = NULL ;
}

CCenterServerApp::~CCenterServerApp()
{
	if ( m_pNetwork )
	{
		m_pNetwork->ShutDown();
		delete m_pNetwork ;
	}
	m_pNetwork = NULL ;
	if ( m_vGateInfos )
	{
		delete[] m_vGateInfos ;
		m_vGateInfos = NULL ;
	}
	s_GateServer = NULL ;
}

bool CCenterServerApp:: Init()
{
	if ( s_GateServer )
	{
		CLogMgr::SharedLogMgr()->ErrorLog("can not have too instance of CenterServerApp") ;
		return false ;
	}
	s_GateServer = this ;
	m_bRunning = true ;
	m_stSvrConfigMgr.LoadFile("../configFile/serverConfig.txt");

	stServerConfig* pSvrConfig = m_stSvrConfigMgr.GetServerConfig(eSvrType_Center); 
	if ( pSvrConfig == NULL )
	{
		CLogMgr::SharedLogMgr()->ErrorLog("can not find center server config so start up failed ") ;
		return  false ;
	}
	m_pNetwork = new CServerNetwork ;
	m_pNetwork->StartupNetwork( pSvrConfig->nPort,20,pSvrConfig->strPassword);
	m_pNetwork->AddDelegate(this);

	for ( uint16_t ndx = 0 ; ndx < eSvrType_Max ; ++ndx )
	{
		m_vTargetServers[ndx] = INVALID_CONNECT_ID ;
	}

	m_uGateCounts = m_stSvrConfigMgr.GetServerConfigCnt(eSvrType_Gate);
	if ( m_uGateCounts == 0 )
	{
		CLogMgr::SharedLogMgr()->ErrorLog("gate server config count = 0 , start up failed ") ;
		return false ;
	}

	m_vGateInfos = new stGateInfo[m_uGateCounts];
	for ( uint16_t nIdx = 0 ; nIdx < m_uGateCounts ; ++nIdx )
	{
		m_vGateInfos[nIdx].Reset();
	}
	CLogMgr::SharedLogMgr()->SystemLog("start center server !");
	return true ;
}

void  CCenterServerApp::RunLoop()
{
	while ( m_bRunning )
	{
		if ( m_pNetwork )
		{
			m_pNetwork->RecieveMsg();
		}
		Sleep(1);
	}

	if ( m_pNetwork )
	{
		m_pNetwork->ShutDown() ;
	}
}

void  CCenterServerApp::Stop()
{
	m_bRunning = false ;
}

bool  CCenterServerApp::OnMessage( Packet* pData )
{
	stMsg* pMsg =(stMsg*)pData->_orgdata ;
	switch ( pMsg->usMsgType )
	{
	case MSG_VERIFY_APNS:
		{
			m_vTargetServers[eSvrType_APNS] = pData->_connectID;
			CLogMgr::SharedLogMgr()->SystemLog("apns server connected ip = %s",m_pNetwork->GetIPInfoByConnectID(pData->_connectID)) ;
		}
		break;
	case MSG_VERIFY_LOGIN:
		{
			m_vTargetServers[eSvrType_Login] = pData->_connectID;
			CLogMgr::SharedLogMgr()->SystemLog("login server connected ip = %s",m_pNetwork->GetIPInfoByConnectID(pData->_connectID)) ;
		}
		break;
	case MSG_VERIFY_LOG:
		{
			m_vTargetServers[eSvrType_Log] = pData->_connectID;
			CLogMgr::SharedLogMgr()->SystemLog("log server connected ip = %s",m_pNetwork->GetIPInfoByConnectID(pData->_connectID)) ;
		}
		break;
	case MSG_VERIFY_DB:
		{
			m_vTargetServers[eSvrType_DB] = pData->_connectID;
			CLogMgr::SharedLogMgr()->SystemLog("DB server connected ip = %s",m_pNetwork->GetIPInfoByConnectID(pData->_connectID)) ;
		}
		break;
	case MSG_VERIFY_DATA:
		{
			m_vTargetServers[eSvrType_Data] = pData->_connectID;
			CLogMgr::SharedLogMgr()->SystemLog("Data server connected ip = %s",m_pNetwork->GetIPInfoByConnectID(pData->_connectID)) ;
		}
		break;
	case MSG_VERIFY_TAXAS:
		{
			m_vTargetServers[eSvrType_Taxas] = pData->_connectID;
			CLogMgr::SharedLogMgr()->SystemLog("Taxas server connected ip = %s",m_pNetwork->GetIPInfoByConnectID(pData->_connectID)) ;
		}
		break;
	case MSG_VERIFY_GATE:
		{			
			stMsgGateServerInfo msg ;
			msg.uMaxGateSvrCount = m_uGateCounts ;
			msg.bIsGateFull = true ;
			for ( uint16_t nIdx = 0 ; nIdx < m_uGateCounts ; ++nIdx )
			{
				if ( m_vGateInfos[nIdx].IsGateWorking() == false )
				{
					msg.bIsGateFull = false ;
					msg.uIdx = nIdx;
					m_vGateInfos->nIdx = nIdx ;
					m_vGateInfos->nNetworkID = pData->_connectID ; 
					CLogMgr::SharedLogMgr()->SystemLog("Gate server started idx = %d connected ip = %s",nIdx,m_pNetwork->GetIPInfoByConnectID(pData->_connectID)) ;
					break;
				}
			}

			if ( msg.bIsGateFull )
			{
				CLogMgr::SharedLogMgr()->ErrorLog("gate is full can not accept more gate ip = %s",m_pNetwork->GetIPInfoByConnectID(pData->_connectID)) ;
			}

			m_pNetwork->SendMsg((char*)&msg,sizeof(msg),pData->_connectID) ;

			if ( msg.bIsGateFull )
			{
				m_pNetwork->ClosePeerConnection(pData->_connectID);
			}
		}
		break;
	case MSG_CONNECT_NEW_CLIENT:
		{
			stMsgNewClientConnected* pC = (stMsgNewClientConnected*)pMsg;
			stGateInfo* pGateInfo = GetGateInfoByNetworkID(pData->_connectID);
			if ( pGateInfo == NULL )
			{
				CLogMgr::SharedLogMgr()->ErrorLog("why no gate , this peer is not gate ip = %s",m_pNetwork->GetIPInfoByConnectID(pData->_connectID)) ;
				return true ;
			}
			pGateInfo->AddSessionID(pC->nNewSessionID) ;
		}
		break;
	case MSG_DISCONNECT_CLIENT:
		{
			stMsgClientDisconnect* pRet = (stMsgClientDisconnect*)pMsg;
			if ( !pRet->bIsForClientReconnect )
			{
				// send disconnect to server ;
				SendClientDisconnectMsg(pRet->nSeesionID) ;
			}

			stGateInfo* pGateInfo = GetGateInfoByNetworkID(pData->_connectID);
			if ( pGateInfo == NULL )
			{
				CLogMgr::SharedLogMgr()->ErrorLog("why no gate , this peer is not gate ip = %s",m_pNetwork->GetIPInfoByConnectID(pData->_connectID)) ;
				return true ;
			}
			pGateInfo->RemoveSessionID(pRet->nSeesionID) ;
		}
		break;
	case MSG_TRANSER_DATA:
		{
			stMsgTransferData* pTransfer = (stMsgTransferData*)pData->_orgdata ;
			stMsg* pReal = (stMsg*)(pData->_orgdata + sizeof(stMsgTransferData)) ;
			if ( pReal->cSysIdentifer == ID_MSG_PORT_CLIENT || pReal->cSysIdentifer == ID_MSG_PORT_GATE )
			{
				stGateInfo* pInfo = GetOwerGateInfoBySessionID(pTransfer->nSessionID) ;
				if ( pInfo && pInfo->IsGateWorking() )
				{
					m_pNetwork->SendMsg(pData->_orgdata,pData->_len,pInfo->nNetworkID ) ;
				}
				else
				{
					CLogMgr::SharedLogMgr()->ErrorLog("send msg = %d to client but nSession id = %d have no gate, inform this session id disconnect ",pReal->usMsgType,pTransfer->nSessionID) ;
					SendClientDisconnectMsg(pTransfer->nSessionID) ;
					if ( pInfo )
					{
						pInfo->RemoveSessionID(pTransfer->nSessionID);
					}
				}
			}
			else
			{
				eServerType svr = GetServerTypeByMsgTarget(pReal->cSysIdentifer);
				if ( eSvrType_Center == svr )
				{
					CLogMgr::SharedLogMgr()->ErrorLog("why msg = %d process here should process above ",pReal->usMsgType ) ;
				}
				else if ( eSvrType_Max == svr )
				{
					CLogMgr::SharedLogMgr()->ErrorLog("unknown msg target = %d",pReal->cSysIdentifer ) ;
				}
				else
				{
					if ( m_vTargetServers[svr] == INVALID_CONNECT_ID )
					{
						CLogMgr::SharedLogMgr()->ErrorLog("sever %s is disconnected",GetServerDescByType(svr)) ;
					}
					else
					{
						m_pNetwork->SendMsg(pData->_orgdata,pData->_len,m_vTargetServers[svr]) ;
					}
				}
			}
		}
		break;
	default:
		{
			CLogMgr::SharedLogMgr()->ErrorLog("unknown msg id = %d , csysIdentifer = %d, ip = %s",pMsg->usMsgType,pMsg->cSysIdentifer,m_pNetwork->GetIPInfoByConnectID(pData->_connectID)) ;
		}
	}
	return true ;
}

void  CCenterServerApp::OnNewPeerConnected( CONNECT_ID nNewPeer, ConnectInfo* IpInfo )
{
	if ( IpInfo )
	{
		CLogMgr::SharedLogMgr()->SystemLog("a peer connected ip = %s , port = %d connect id = %d",IpInfo->strAddress,IpInfo->nPort ,nNewPeer );
	}
	else
	{
		CLogMgr::SharedLogMgr()->SystemLog("a peer connected ip = NULL, connect id = %d", nNewPeer) ;
	}
}

void CCenterServerApp::OnGateDisconnected(CONNECT_ID& nNetworkID )
{
	stGateInfo* pGate = GetGateInfoByNetworkID(nNetworkID);
	if ( pGate == NULL )
	{
		CLogMgr::SharedLogMgr()->ErrorLog("this is not gate , info = NULL ip = %s",m_pNetwork->GetIPInfoByConnectID(nNetworkID)) ;
		return ;
	}

	std::map<uint32_t,uint8_t>::iterator iter = pGate->vMapSessionIDInGate.begin();
	for ( ; iter != pGate->vMapSessionIDInGate.end(); ++iter )
	{
		SendClientDisconnectMsg(iter->first) ;
	}
	pGate->Reset();
}

CCenterServerApp::stGateInfo* CCenterServerApp::GetGateInfoByNetworkID(CONNECT_ID nNetworkID )
{
	for ( uint16_t nIdx = 0 ; nIdx < m_uGateCounts ; ++nIdx )
	{
		if ( m_vGateInfos[nIdx].nNetworkID == nNetworkID )
		{
			return &m_vGateInfos[nIdx] ;
		}
	}
	return NULL ;
}

CCenterServerApp::stGateInfo* CCenterServerApp::GetOwerGateInfoBySessionID(uint32_t nSessionID )
{
	uint16_t nIdx = nSessionID % m_uGateCounts ;
	return &m_vGateInfos[nIdx] ;
}

void  CCenterServerApp::OnPeerDisconnected( CONNECT_ID nPeerDisconnected, ConnectInfo* IpInfo )
{
	// check server dis connect ;
	stGateInfo* pinfo = GetGateInfoByNetworkID(nPeerDisconnected);
	if ( pinfo )
	{
		if ( IpInfo )
		{
			CLogMgr::SharedLogMgr()->ErrorLog("a gate idx = %d ,disconnected , ip = %s , port = %d",pinfo->nIdx,IpInfo->strAddress,IpInfo->nPort ) ;
		}
		else
		{
			CLogMgr::SharedLogMgr()->ErrorLog("a gate idx = %d ,disconnected , ip = %s",pinfo->nIdx,m_pNetwork->GetIPInfoByConnectID(nPeerDisconnected)) ;
		}
		OnGateDisconnected(nPeerDisconnected);
		pinfo->Reset();
		return ;
	}

	// check other server ;
	for ( uint16_t nIdx = 0 ; nIdx < eSvrType_Max ; ++nIdx )
	{
		if ( m_vTargetServers[nIdx] == nPeerDisconnected )
		{
			CLogMgr::SharedLogMgr()->ErrorLog("Server type = %s disconnected ", GetServerDescByType((eServerType)nIdx) ) ;
			m_vTargetServers[nIdx] = INVALID_CONNECT_ID ;
			return ;
		}
	}

	if ( IpInfo )
	{
		CLogMgr::SharedLogMgr()->ErrorLog( "a unknown peer dis conntcted ip = %s port = %d",IpInfo->strAddress,IpInfo->nPort ) ;
	}
	else
	{
		CLogMgr::SharedLogMgr()->ErrorLog("a unknown peer disconnect ip = unknown") ;
	}
}

const char* CCenterServerApp::GetServerDescByType(eServerType eType )
{
	static const char* vSvrString[] = 
	{ 
		"eSvrType_Gate",
		"eSvrType_Login",
		"eSvrType_DB",
		"eSvrType_Game",
		"eSvrType_Verify",
		"eSvrType_DataBase",
		"eSvrType_APNS",
		"eSvrType_Log",
		"eSvrType_LogDataBase",
		"eSvrType_Center",
		"eSvrType_Max",
	} ;
	
	if ( eType > eSvrType_Max )
	{
		return "unknown server type ";
	}
	return vSvrString[eType] ;
}

void CCenterServerApp::SendClientDisconnectMsg(uint32_t nSessionID )
{
	static char* s_pBuffer = NULL ;
	if ( s_pBuffer == NULL )
	{
		s_pBuffer = new char[sizeof(stMsgTransferData) + sizeof(stMsgClientDisconnect)];
	}

	stMsgTransferData msgTransData ;
	msgTransData.bBroadCast = false ;
	msgTransData.nSessionID = nSessionID ;
	stMsgClientDisconnect msgDisc ;
	msgDisc.nSeesionID = nSessionID ;

	uint16_t nLen = 0 ;
	memcpy(s_pBuffer,&msgTransData,sizeof(stMsgTransferData) );
	nLen += sizeof(msgTransData) ;
	memcpy(s_pBuffer + nLen , &msgDisc,sizeof(msgDisc)) ;
	nLen += sizeof (msgDisc);
	
	if ( m_vTargetServers[eSvrType_Data] != INVALID_CONNECT_ID )
	{
		m_pNetwork->SendMsg(s_pBuffer,nLen,m_vTargetServers[eSvrType_Data] ) ;
	}
	else
	{
		CLogMgr::SharedLogMgr()->ErrorLog("data svr is disconnected") ;
	}
}
eServerType CCenterServerApp::GetServerTypeByMsgTarget(uint16_t nTarget)
{
	switch ( nTarget )
	{
	case ID_MSG_PORT_CLIENT:
		{
			return eSvrType_Max ;
		}
		break;
	case ID_MSG_PORT_DATA:
		{
			return eSvrType_Data ;
		}
		break;
	case ID_MSG_PORT_GATE:
		{
			return eSvrType_Gate;
		}
		break;
	case ID_MSG_PORT_VERIFY:
		{
			return eSvrType_Verify ;
		}
		break;
	case ID_MSG_PORT_LOGIN:
		{
			return eSvrType_Login ;
		}
		break;
	case ID_MSG_PORT_DB:
		{
			return eSvrType_DB ;
		}
		break;
	case ID_MSG_PORT_CENTER:
		{
			return eSvrType_Center ;
		}
		break;
	case ID_MSG_PORT_APNS:
		{
			return eSvrType_APNS;
		}
		break;
	case ID_MSG_PORT_LOG:
		{
			return eSvrType_Log;
		}
		break;
	case ID_MSG_PORT_TAXAS:
		{
			return eSvrType_Taxas ;
		}
		break;
	default:
		return eSvrType_Max ;
	}
}
