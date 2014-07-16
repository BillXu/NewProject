#include "LoginApp.h"
#include "LogManager.h"
#include "ServerMessageDefine.h"
#include "LoginPeerMgr.h"
CLoginApp::CLoginApp()
{
	m_pNetWork = NULL ;
	m_pReconnctGate = NULL ;
	m_pReconnectDB = NULL ;
	m_pPeerMgr = NULL ;
}

CLoginApp::~CLoginApp()
{
	if ( m_pNetWork )
	{
		m_pNetWork->ShutDown();
		delete m_pNetWork ;
		m_pNetWork = NULL ;
	}

	if ( m_pReconnectDB )
	{
		m_pTimerMgr->RemoveTimer(m_pReconnectDB) ;
		m_pReconnectDB = NULL ;
	}

	if ( m_pReconnctGate )
	{
		m_pTimerMgr->RemoveTimer(m_pReconnctGate) ;
		m_pReconnctGate = NULL ;
	}

	if ( m_pPeerMgr )
	{
		delete m_pPeerMgr ;
		m_pPeerMgr = NULL ;
	}
}

void CLoginApp::Init()
{
	m_pTimerMgr = new CTimerManager ;
	m_pNetWork = new CNetWorkMgr ;
	m_pNetWork->SetupNetwork(2);
	m_pNetWork->AddMessageDelegate(this);

	m_stSvrConfigMgr.LoadFile("../configFile/serverConfig.txt");
	// connected to Gate ;
	m_stGateServer.m_bConnected = false ;
	m_stGateServer.m_strIPAddress = m_stSvrConfigMgr.GetServerConfig(eSvrType_Gate)->strIPAddress;
	m_stGateServer.m_nPort = m_stSvrConfigMgr.GetServerConfig(eSvrType_Gate)->nPort;
	m_pNetWork->ConnectToServer(m_stGateServer.m_strIPAddress.c_str(),m_stGateServer.m_nPort,m_stSvrConfigMgr.GetServerConfig(eSvrType_Gate)->strPassword) ;

	// connected to DB ;
	m_stDBServer.m_bConnected = false ;
	m_stDBServer.m_strIPAddress = m_stSvrConfigMgr.GetServerConfig(eSvrType_DB)->strIPAddress;
	m_stDBServer.m_nPort = m_stSvrConfigMgr.GetServerConfig(eSvrType_DB)->nPort;
	m_pNetWork->ConnectToServer(m_stDBServer.m_strIPAddress.c_str(),m_stDBServer.m_nPort,m_stSvrConfigMgr.GetServerConfig(eSvrType_DB)->strPassword) ;

	// Peer Mgr 
	m_pPeerMgr = new CLoginPeerMgr(this);
	CLogMgr::SharedLogMgr()->SystemLog("Start Login Server,Connecting to DB and Gate ");
}

void CLoginApp::MainLoop()
{
	while( true )
	{
		if ( m_pNetWork) 
		{
			m_pNetWork->ReciveMessage() ;
		}
		m_pTimerMgr->Update();
		Sleep(1);
	}
}

bool CLoginApp::OnMessage( RakNet::Packet* pMsg )
{
	CHECK_MSG_SIZE(stMsg,pMsg->length) ;
	stMsg* pmsg = (stMsg*)pMsg->data ;
	if ( pmsg->cSysIdentifer == ID_MSG_VERIFY && MSG_VERIFY_GATE == pmsg->usMsgType )
	{
		m_stGateServer.m_bConnected = true ;
		m_stGateServer.m_nServerNetID = pMsg->guid ;
		CLogMgr::SharedLogMgr()->SystemLog("Connected to GateServer!");
	}
	else if ( pmsg->cSysIdentifer == ID_MSG_VERIFY && MSG_VERIFY_DB == pmsg->usMsgType )
	{
		m_stDBServer.m_bConnected = true ;
		m_stDBServer.m_nServerNetID = pMsg->guid ;
		CLogMgr::SharedLogMgr()->SystemLog("Connected to DBServer!");
	}
	else
	{
		if ( m_pPeerMgr )
		{
			m_pPeerMgr->OnMessage(pMsg) ;
		}
	}
	return true ;
}

bool CLoginApp::OnLostSever(RakNet::Packet* pMsg)
{
	if ( pMsg->guid == m_stDBServer.m_nServerNetID )
	{
		m_stDBServer.m_bConnected = false ;
		CLogMgr::SharedLogMgr()->ErrorLog("DBServer Lost");
		TryConnect(false);
	}
	else
	{
		m_stGateServer.m_bConnected = false ;
		CLogMgr::SharedLogMgr()->ErrorLog("GateServer Lost");
		TryConnect(true);
	}
	return true ;
}

bool CLoginApp::OnConnectStateChanged( eConnectState eSate, RakNet::Packet* pMsg)
{
	if ( eSate == eConnect_Accepted )
	{
		stMsg cMsg ;
		cMsg.cSysIdentifer = ID_MSG_VERIFY ;
		cMsg.usMsgType = MSG_VERIFY_LOGIN ;
		m_pNetWork->SendMsg((char*)&cMsg,sizeof(stMsg),pMsg->guid) ;
		return true ;
	}

	// connected failed ; try again ;
	const char* pIP = pMsg->systemAddress.ToString(false) ;
	if ( strcmp(pIP,m_stGateServer.m_strIPAddress.c_str()) == 0 && m_stGateServer.m_nPort == pMsg->systemAddress.GetPort() )
	{
		// gate connected failed ;
		TryConnect(true);
	}
	else
	{
		// db connected failed ;
		TryConnect(false);
	}
	return true ;
}

bool CLoginApp::SendMsg( const char* pBuffer , unsigned int nLen , bool bGate )
{
	if ( m_pNetWork == NULL )
		return false ;
	if ( bGate )
	{
		return SendMsgToGate(pBuffer,nLen) ;
	}
	else
	{
		return SendMsgToDB(pBuffer,nLen) ;
	}
}

void CLoginApp::ReconnectDB(float fTimeElaps,unsigned int nTimerID )
{
	m_pReconnectDB->Stop();
	CLogMgr::SharedLogMgr()->SystemLog("Try Connect to DBServer...");
	m_pNetWork->ConnectToServer(m_stDBServer.m_strIPAddress.c_str(),m_stDBServer.m_nPort,m_stSvrConfigMgr.GetServerConfig(eSvrType_DB)->strPassword) ;
}

void CLoginApp::ReconnectGate(float fTimeElaps,unsigned int nTimerID )
{
	m_pReconnctGate->Stop() ;
	CLogMgr::SharedLogMgr()->SystemLog("Try Connect to GateServer...");
	m_pNetWork->ConnectToServer(m_stGateServer.m_strIPAddress.c_str(),m_stGateServer.m_nPort,m_stSvrConfigMgr.GetServerConfig(eSvrType_Gate)->strPassword) ;
}

bool CLoginApp::SendMsgToGate(const char* pBuffer , unsigned int nLen)
{
	if ( m_stGateServer.m_bConnected == false )
	{
		return false ;
	}
	m_pNetWork->SendMsg(pBuffer,nLen,m_stGateServer.m_nServerNetID) ;
	return true ;
}

bool CLoginApp::SendMsgToDB(const char* pBuffer , unsigned int nLen)
{
	if ( m_stDBServer.m_bConnected == false )
	{
		return false ;
	}
	m_pNetWork->SendMsg(pBuffer,nLen,m_stDBServer.m_nServerNetID) ;
	return true ;
}

void CLoginApp::TryConnect(bool bGate )
{
	if ( !bGate )
	{
		if ( m_pReconnectDB == NULL )
		{
			m_pReconnectDB = m_pTimerMgr->AddTimer(this,(CTimerDelegate::lpTimerSelector)&CLoginApp::ReconnectDB);
			m_pReconnectDB->SetDelayTime(5);
		}
		m_pReconnectDB->Reset();
		m_pReconnectDB->Start() ;
		return ;
	}

	if ( m_pReconnctGate == NULL )
	{
		m_pReconnctGate = m_pTimerMgr->AddTimer(this,(CTimerDelegate::lpTimerSelector)&CLoginApp::ReconnectGate);
		m_pReconnctGate->SetDelayTime(5);
	}
	m_pReconnctGate->Reset();
	m_pReconnctGate->Start() ;
}