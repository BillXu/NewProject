#include "DBApp.h"
#include "DBManager.h"
#include "DataBaseThread.h"
#include "DBRequest.h"
#include "ServerMessageDefine.h"
#include "CommonDefine.h"
#include "LogManager.h"
CDBServerApp::CDBServerApp()
{
	m_pNetWork = NULL ;
	m_pDBManager = NULL ;
	m_pDBWorkThread = NULL ;
}

CDBServerApp::~CDBServerApp()
{
	if ( m_pNetWork )
	{
		m_pNetWork->ShutDown();
		delete m_pNetWork ;
	}

	if ( m_pDBManager )
	{
		delete m_pDBManager ;
	}

	if ( m_pDBWorkThread )
	{
		delete m_pDBWorkThread ;
	}
}

void CDBServerApp::Init()
{
	m_bRunning = true ;
	m_stSvrConfigMgr.LoadFile("../configFile/serverConfig.txt");
	// setup net work
	m_pNetWork = new CServerNetwork ;
	m_pNetWork->StartupNetwork(m_stSvrConfigMgr.GetServerConfig(eSvrType_Log)->nPort,10,m_stSvrConfigMgr.GetServerConfig(eSvrType_Log)->strPassword) ;
	m_pNetWork->AddDelegate(this);

	// set up data base thread 
	m_pDBWorkThread = new CDataBaseThread ;
	stServerConfig* pDatabase = m_stSvrConfigMgr.GetServerConfig(eSvrType_LogDataBase);
	m_pDBWorkThread->InitDataBase(pDatabase->strIPAddress,pDatabase->nPort,pDatabase->strAccount,pDatabase->strPassword,"taxpokerlogdb");
	m_pDBWorkThread->Start();

	// dbManager ;
	m_pDBManager = new CDBManager(this) ;
	m_pDBManager->Init();

	CLogMgr::SharedLogMgr()->SystemLog("DBServer Start!");
}
bool CDBServerApp::MainLoop()
{
	if ( m_pNetWork )
	{
		m_pNetWork->RecieveMsg() ;
	}

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
	return true ;
}

// net delegate
bool CDBServerApp::OnMessage( RakNet::Packet* pData )
{
	CHECK_MSG_SIZE(stMsg,pData->length) ;
	stMsg* pmsg = (stMsg*)pData->data ;
	if ( pmsg->cSysIdentifer == ID_MSG_VERIFY )
	{
		return true ;
	}

	if ( m_pDBManager )
	{
		m_pDBManager->OnMessage(pData) ;
	}
	return true ;
}

void CDBServerApp::OnNewPeerConnected(RakNet::RakNetGUID& nNewPeer, RakNet::Packet* pData )
{
	stMsg msg ;
	msg.cSysIdentifer = ID_MSG_VERIFY ;
	msg.usMsgType = MSG_VERIFY_LOG ;
	m_pNetWork->SendMsg((char*)&msg,sizeof(msg),nNewPeer,false) ;
	CLogMgr::SharedLogMgr()->SystemLog("a peer connected IP = %s",pData->systemAddress.ToString(true));
}

void CDBServerApp::OnPeerDisconnected(RakNet::RakNetGUID& nPeerDisconnected, RakNet::Packet* pData )
{
	CLogMgr::SharedLogMgr()->SystemLog("a peer Disconnected IP = %s",pData->systemAddress.ToString(true));
}

void CDBServerApp::SendMsg(const char* pBuffer, int nLen, RakNet::RakNetGUID& nTarget )
{
	m_pNetWork->SendMsg(pBuffer,nLen,nTarget,false) ;
}

void CDBServerApp::OnExit()
{
	m_pNetWork->ShutDown();
	m_pDBWorkThread->StopWork();
	CLogMgr::SharedLogMgr()->SystemLog("DBServer ShutDown!");
}