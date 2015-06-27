#include "DBApp.h"
#include "DBManager.h"
#include "DataBaseThread.h"
#include "DBRequest.h"
#include "ServerMessageDefine.h"
#include "CommonDefine.h"
#include "LogManager.h"
CDBServerApp::CDBServerApp()
{
	m_pDBManager = NULL ;
	m_pDBWorkThread = NULL ;
}

CDBServerApp::~CDBServerApp()
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

bool CDBServerApp::init()
{
	IServerApp::init();
	
	m_stSvrConfigMgr.LoadFile("../configFile/serverConfig.txt");
	stServerConfig* pCenter = m_stSvrConfigMgr.GetServerConfig(eSvrType_Center);
	if ( pCenter == NULL )
	{
		CLogMgr::SharedLogMgr()->ErrorLog("center svr config is null canont start DB server") ;
		return false;
	}
	setConnectServerConfig(pCenter);
	// set up data base thread 
	stServerConfig* pDatabase = m_stSvrConfigMgr.GetServerConfig(eSvrType_DataBase);
	if ( pDatabase == NULL )
	{
		CLogMgr::SharedLogMgr()->ErrorLog("data base config is null, cant not start server") ;
		return false;
	}

	m_pDBWorkThread = new CDataBaseThread ;
	m_pDBWorkThread->InitDataBase(pDatabase->strIPAddress,pDatabase->nPort,pDatabase->strAccount,pDatabase->strPassword,"taxpokerdb");
	m_pDBWorkThread->Start();

	// dbManager ;
	m_pDBManager = new CDBManager(this) ;
	m_pDBManager->Init();

	CLogMgr::SharedLogMgr()->SystemLog("DBServer Start!");
	return true ;
}
void CDBServerApp::update(float fDeta )
{
	IServerApp::update(fDeta);
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
bool CDBServerApp::onLogicMsg( stMsg* prealMsg , eMsgPort eSenderPort , uint32_t nSessionID  )
{
	m_pDBManager->OnMessage(prealMsg,eSenderPort,nSessionID ) ;
	return true ;
}

void CDBServerApp::onExit()
{
	m_pDBWorkThread->StopWork();
	CLogMgr::SharedLogMgr()->SystemLog("DBServer ShutDown!");
}