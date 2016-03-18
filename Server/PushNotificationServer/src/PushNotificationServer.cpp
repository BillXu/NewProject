#include "PushNotificationServer.h"
#include "LogManager.h"
#include "ServerMessageDefine.h"
CPushNotificationServer::~CPushNotificationServer()
{

}

bool CPushNotificationServer::init()
{
	IServerApp::init();
	CLogMgr::SharedLogMgr()->SetOutputFile("LoginSvr");
	m_stSvrConfigMgr.LoadFile("../configFile/serverConfig.txt");

	stServerConfig* pSvrConfigItem = m_stSvrConfigMgr.GetServerConfig(eSvrType_DataBase );
	if ( pSvrConfigItem == NULL )
	{
		CLogMgr::SharedLogMgr()->ErrorLog("Data base config is null , can not start login svr ") ;
		return false;
	}

	// connected to center ;
	pSvrConfigItem = m_stSvrConfigMgr.GetServerConfig(eSvrType_Center );
	if ( pSvrConfigItem == NULL )
	{
		CLogMgr::SharedLogMgr()->ErrorLog("center svr config is null so can not start svr ") ;
		return false;
	}
	setConnectServerConfig(pSvrConfigItem);

	m_nPushThread.InitSSLContex();
	if ( m_nPushThread.ConnectToAPNs() == false )
	{
		CLogMgr::SharedLogMgr()->ErrorLog("connect to apns svr failed") ;
	}
	else
	{
		CLogMgr::SharedLogMgr()->SystemLog("connect to apns svr success") ;
	}
	return true; 
}

bool CPushNotificationServer::onLogicMsg( stMsg* prealMsg , eMsgPort eSenderPort , uint32_t nSessionID )
{
	if (  IServerApp::onLogicMsg(prealMsg,eSenderPort,nSessionID) )
	{
		return true;
	}

	if ( m_tNoticePlayerMgr.onMsg(prealMsg,nSessionID) )
	{
		return true ;
	}

	return false ;
}