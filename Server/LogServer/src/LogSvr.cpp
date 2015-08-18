#include "LogSvrApp.h"
#include "DBManager.h"
#include "DataBaseThread.h"
#include "DBRequest.h"
#include "ServerMessageDefine.h"
#include "CommonDefine.h"
#include "LogManager.h"
CLogSvrApp::CLogSvrApp()
{
	m_pDBManager = NULL ;
	m_pDBWorkThread = NULL ;
}

CLogSvrApp::~CLogSvrApp()
{
	if ( m_pDBManager )
	{
		delete m_pDBManager ;
	}

	if ( m_pDBWorkThread )
	{
		delete m_pDBWorkThread ;
	}
}

bool CLogSvrApp::init()
{
	IServerApp::init();

	CSeverConfigMgr stSvrConfigMgr;
	stSvrConfigMgr.LoadFile("../configFile/serverConfig.txt");
	// setup net work
	stServerConfig* pConfig = stSvrConfigMgr.GetServerConfig(eSvrType_Center);
	if ( pConfig == NULL )
	{
		CLogMgr::SharedLogMgr()->ErrorLog("center svr config is null , so can not connected to !") ;
		return false;
	}
	setConnectServerConfig(pConfig);
	// set up data base thread 
	m_pDBWorkThread = new CDataBaseThread ;
	stServerConfig* pDatabase = stSvrConfigMgr.GetServerConfig(eSvrType_LogDataBase);
	m_pDBWorkThread->InitDataBase(pDatabase->strIPAddress,pDatabase->nPort,pDatabase->strAccount,pDatabase->strPassword,"taxpokerlogdb");
	m_pDBWorkThread->Start();

	// dbManager ;
	m_pDBManager = new CDBManager() ;
	m_pDBManager->Init();

	CLogMgr::SharedLogMgr()->SystemLog("log server Start!");
	return true ;
}
void CLogSvrApp::update(float fDeta)
{
	// process DB Result ;
	CDBRequestQueue::VEC_DBRESULT vResultOut ;
	CDBRequestQueue::SharedDBRequestQueue()->GetAllResult(vResultOut) ;
	CDBRequestQueue::VEC_DBRESULT::iterator iter = vResultOut.begin() ;
	for ( ; iter != vResultOut.end(); ++iter )
	{
		stDBResult* pRet = *iter ;
		m_pDBManager->OnDBResult(pRet) ;
		delete pRet ;
	}
	vResultOut.clear();
}

// net delegate
bool CLogSvrApp::onLogicMsg( stMsg* prealMsg , eMsgPort eSenderPort , uint32_t nSessionID )
{
	if (  IServerApp::onLogicMsg(prealMsg,eSenderPort,nSessionID) )
	{
		return true;
	}

	if ( prealMsg->cSysIdentifer == ID_MSG_VERIFY )
	{
		return true ;
	}

	if ( m_pDBManager )
	{
		m_pDBManager->OnMessage(prealMsg,eSenderPort,nSessionID) ;
	}
	return true ;
}


void CLogSvrApp::onExit()
{
	m_pDBWorkThread->StopWork();
	CLogMgr::SharedLogMgr()->SystemLog("log svr ShutDown!");
}

uint16_t CLogSvrApp::getLocalSvrMsgPortType()
{
	return ID_MSG_PORT_LOG ;
}