#include "GameServerApp.h"
#include "CommonDefine.h"
#include "MessageDefine.h"
#include "LogManager.h"
#include "PlayerManager.h"
#include "ServerMessageDefine.h"
#include <ctime>
#include "Timer.h"
#include "Brocaster.h"
#include "PlayerMail.h"
#include "EventCenter.h"
#ifndef USHORT_MAX
#define USHORT_MAX 65535 
#endif

char* CGameServerApp::s_pBuffer = NULL ;
CGameServerApp* CGameServerApp::s_GameServerApp = NULL ;
CGameServerApp* CGameServerApp::SharedGameServerApp()
{
	 
	return s_GameServerApp ;
}

CGameServerApp::~CGameServerApp()
{
	delete m_pTimerMgr ;
	delete m_pNetWork ;
	delete m_pPlayerManager ;
	delete m_pConfigManager ;
	delete m_pGameRanker ;
	delete m_pBrocaster;
}

void CGameServerApp::Init()
{
	m_bRunning = true ;
	if ( s_GameServerApp == NULL )
	{
		s_GameServerApp = this ;
	}
	else
	{
		CLogMgr::SharedLogMgr()->ErrorLog("Game Server App can not be init more than once !") ;
	}

	srand((unsigned int)time(0));
	if ( s_pBuffer == NULL )
	{
		s_pBuffer = new char[MAX_MSG_BUFFER_LEN] ;
	}
	memset(m_vIsSvrConnected,0,sizeof(m_vIsSvrConnected)) ;
	m_stSvrConfigMgr.LoadFile("../configFile/serverConfig.txt");
	m_pNetWork = new CNetWorkMgr ;
	m_pNetWork->SetupNetwork(eSvrType_Max - 1);
	m_pNetWork->AddMessageDelegate(this);
	ConnectToOtherServer();

	m_pConfigManager = new CConfigManager ;
	m_pConfigManager->LoadAllConfigFile("../configFile/") ;
	// init component ;
	m_pTimerMgr = new CTimerManager ;

	m_pRoomMgr.Init() ;
	m_pPlayerManager = new CPlayerManager ;

	m_pGameRanker = NULL ;

	m_pBrocaster = new CBrocaster ;
	
	time_t tNow = time(NULL) ;
	m_nCurDay = localtime(&tNow)->tm_mday ;
}

bool CGameServerApp::OnMessage( RakNet::Packet* pMsg )
{
	stMsg* MsgVer = (stMsg*)pMsg->data;
	if ( MsgVer->cSysIdentifer == ID_MSG_VERIFY )
	{
		// send back 
		stMsg msg ;
		msg.cSysIdentifer = ID_MSG_VERIFY ;
		msg.usMsgType = MSG_VERIFY_GAME ;
		m_pNetWork->SendMsg((char*)&msg,sizeof( msg),pMsg->guid ) ;

		// check idenditfy ;
		if ( MSG_VERIFY_GATE == MsgVer->usMsgType )
		{
			m_vSvrNetUID[eSvrType_Gate] = pMsg->guid ;
			m_vIsSvrConnected[eSvrType_Gate] = true;
			CLogMgr::SharedLogMgr()->SystemLog("Connnected Gate Server !") ;
			return true ;
		}
		else if ( MSG_VERIFY_DB == MsgVer->usMsgType )
		{
			m_vSvrNetUID[eSvrType_DB] = pMsg->guid ;
			m_vIsSvrConnected[eSvrType_DB] = true;
			OnDBServerConnected();
			CLogMgr::SharedLogMgr()->SystemLog("Connnected DB Server !") ;
			return true ;
		}
		else if ( MSG_VERIFY_VERIYF == MsgVer->usMsgType )
		{
			m_vSvrNetUID[eSvrType_Verify] = pMsg->guid ;
			m_vIsSvrConnected[eSvrType_Verify] = true;
			CLogMgr::SharedLogMgr()->SystemLog("Connnected VerifyServer !") ;
			return true ;
		}
		else if ( MSG_VERIFY_APNS == MsgVer->usMsgType )
		{
			m_vSvrNetUID[eSvrType_APNS] = pMsg->guid ;
			m_vIsSvrConnected[eSvrType_APNS] = true;
			CLogMgr::SharedLogMgr()->SystemLog("Connnected MSG_VERIFY_APNS !") ;
			return true ;
		}
		else if ( MSG_VERIFY_LOG == MsgVer->usMsgType )
		{
			m_vSvrNetUID[eSvrType_Log] = pMsg->guid ;
			m_vIsSvrConnected[eSvrType_Log] = true;
			CLogMgr::SharedLogMgr()->SystemLog("Connnected Log Svr !") ;
			return true ;
		}
	}

	if ( ProcessGobalArgument(MsgVer) )
	{
		return true ;
	}

	if ( m_pGameRanker->OnMessage(pMsg) )
	{
		return true ;
	}

	return m_pPlayerManager->OnMessage(pMsg) ;
}

bool CGameServerApp::ProcessGobalArgument(stMsg* MsgVer )
{
	if ( MsgVer->cSysIdentifer == ID_MSG_DB2GM && MSG_GAME_SERVER_GET_MAX_MAIL_UID == MsgVer->usMsgType )
	{
		stMsgGameServerGetMaxMailUIDRet* pRetMsg = (stMsgGameServerGetMaxMailUIDRet*)MsgVer ;
		CPlayerMailComponent::s_nCurMaxMailUID = pRetMsg->nMaxMailUID ;
		return  true;
	}
	return false ;
}

void CGameServerApp::RequestGlobalArgumentFromDB()
{
	// get max mail UID ;
	stMsgGameServerGetMaxMailUID msg ;
	SendMsgToDBServer((char*)&msg,sizeof(msg)) ;
}

void CGameServerApp::OnDBServerConnected()
{
	if ( !m_pGameRanker )
	{
		m_pGameRanker = new CGamerRank ;
		m_pGameRanker->Init() ;
	}
	RequestGlobalArgumentFromDB();
}

bool CGameServerApp::OnLostSever(RakNet::Packet* pMsg)
{
	bool bOurSvr = false ;
	for ( int i = 0 ; i < eSvrType_Max ; ++i )
	{
		if ( m_vSvrNetUID[i] == pMsg->guid )
		{
			m_vIsSvrConnected[i] = false ;
			CLogMgr::SharedLogMgr()->ErrorLog("SvrType = %d Lost",i);
			bOurSvr = true ;
			break; ;
		}
	}

	if ( bOurSvr == false )
	{
		CLogMgr::SharedLogMgr()->ErrorLog("unknown server lost") ;
		return true;
	}
	ConnectToOtherServer();
	return false ;
}

bool CGameServerApp::OnConnectStateChanged( eConnectState eSate, RakNet::Packet* pMsg)
{
	if ( eConnect_Failed == eSate || eConnect_SeverFull == eSate )
	{
		CLogMgr::SharedLogMgr()->PrintLog("connected Failed, Try Again !");
		ConnectToOtherServer();
	}
	return false;
}

bool CGameServerApp::Run()
{
	while (m_bRunning )
	{
		m_pNetWork->ReciveMessage() ;
		m_pTimerMgr->Update() ;
		m_pPlayerManager->Update(0);	
		CheckNewDay();
		Sleep(6);
	}
	ShutDown() ;
	return true ;
}

void CGameServerApp::ShutDown()
{
	m_pNetWork->ShutDown() ;
}

void CGameServerApp::SendMsgToGateServer( unsigned int nSessionID , const char* pBuffer , int nLen, bool bBroadcast )
{
	stMsgTransferData msg ;
	msg.cSysIdentifer = ID_MSG_GM2GA ;
	msg.bBroadCast = bBroadcast ;
	msg.nSessionID = nSessionID ;
	memcpy(s_pBuffer,&msg,sizeof(stMsgTransferData));
	memcpy((void*)(s_pBuffer + sizeof(stMsgTransferData)),pBuffer,nLen);
	SendMsgToSvr(eSvrType_Gate,s_pBuffer,nLen + sizeof(stMsgTransferData));
}

void CGameServerApp::SendMsgToDBServer(const char* pBuffer , int nLen )
{
	SendMsgToSvr(eSvrType_DB,pBuffer,nLen);
}

void CGameServerApp::SendMsgToAPNsServer(const char* pBuffer, int nLen )
{
	SendMsgToSvr(eSvrType_APNS,pBuffer,nLen);
}

void CGameServerApp::SendMsgToVerifyServer(const char* pBuffer, int nLen )
{
	SendMsgToSvr(eSvrType_Verify,pBuffer,nLen);
}

void CGameServerApp::ConnectToOtherServer()
{
	for ( int i = 0 ; i < eSvrType_Max ; ++i )
	{
		if ( IsSeverNeedToConnect((eServerType)i))
		{
			if ( m_vIsSvrConnected[i] == false )
			{
				stServerConfig* pConfig = GetServerConfig((eServerType)i) ;
				if ( !pConfig )
				{
					continue;
				}
				m_pNetWork->ConnectToServer(pConfig->strIPAddress,pConfig->nPort,pConfig->strPassword) ; // gate server ;
			}
		}
	}
}

void CGameServerApp::CheckNewDay()
{
	// check new day 
	time_t tNow = time(NULL) ;
	struct tm tmNow = *localtime(&tNow) ;
	if ( tmNow.tm_mday != m_nCurDay )
	{
		m_nCurDay = tmNow.tm_mday ;
		// new day 
		CEventCenter::SharedEventCenter()->PostEvent(eEvent_NewDay,&tmNow) ;
	}
}

stServerConfig* CGameServerApp::GetServerConfig(eServerType eType)
{
	stServerConfig* pDBConfig = m_stSvrConfigMgr.GetServerConfig(eType) ;
	if ( pDBConfig == NULL )
	{
		CLogMgr::SharedLogMgr()->ErrorLog("can not find server config for Svr =%d",eType) ;
		return NULL ;
	}
	return pDBConfig ;
}

void CGameServerApp::SendMsgToSvr(eServerType eType,const char* pBuffer , int nLen )
{
	if ( m_vIsSvrConnected[eType] == false )
	{
		CLogMgr::SharedLogMgr()->ErrorLog("can not send msg to SvrType = %d , because DBServer is not connecting !",eType) ;
		return ;
	}
	m_pNetWork->SendMsg(pBuffer,nLen,m_vSvrNetUID[eType]) ;
}