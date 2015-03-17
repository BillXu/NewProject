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
	m_nCenterSvrConnectID = INVALID_CONNECT_ID ;
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
	stServerConfig* pCenter = m_stSvrConfigMgr.GetServerConfig(eSvrType_Center);
	if ( pCenter == NULL )
	{
		CLogMgr::SharedLogMgr()->ErrorLog("center svr config is null canont start DB server") ;
		return ;
	}
	// setup net work
	m_pNetWork = new CNetWorkMgr ;
	m_pNetWork->SetupNetwork(1);
	m_pNetWork->ConnectToServer(pCenter->strIPAddress,pCenter->nPort,pCenter->strPassword);
	m_pNetWork->AddMessageDelegate(this) ;

	// set up data base thread 
	stServerConfig* pDatabase = m_stSvrConfigMgr.GetServerConfig(eSvrType_DataBase);
	if ( pDatabase == NULL )
	{
		CLogMgr::SharedLogMgr()->ErrorLog("data base config is null, cant not start server") ;
		m_pNetWork->ShutDown();
		delete m_pNetWork ;
		m_pNetWork = NULL ;
		return ;
	}

	m_pDBWorkThread = new CDataBaseThread ;
	m_pDBWorkThread->InitDataBase(pDatabase->strIPAddress,pDatabase->nPort,pDatabase->strAccount,pDatabase->strPassword,"taxpokerdb");
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
		m_pNetWork->ReciveMessage() ;
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
bool CDBServerApp::OnMessage( Packet* pMsg )
{
	CHECK_MSG_SIZE(stMsg,pMsg->_len) ;
	stMsg* pmsg = (stMsg*)pMsg->_orgdata ;
	if ( pmsg->cSysIdentifer == ID_MSG_VERIFY )
	{
		return true ;
	}

	stMsg* pRet = (stMsg*)pMsg->_orgdata ;
	if ( pRet->usMsgType != MSG_TRANSER_DATA )
	{
		CLogMgr::SharedLogMgr()->ErrorLog("why msg type is not transfer data , type = %d",pRet->usMsgType ) ;
		return true;
	}

	stMsgTransferData* pData = (stMsgTransferData*)pRet ;
	if ( m_pDBManager )
	{
		stMsg* preal = (stMsg*)(pMsg->_orgdata + sizeof(stMsgTransferData));
		m_pDBManager->OnMessage(preal,(eMsgPort)pData->nSenderPort,pData->nSessionID ) ;
	}
	return true ;
}

bool CDBServerApp::OnConnectStateChanged( eConnectState eSate, Packet* pMsg)
{
	if ( eConnect_Accepted == eSate )
	{
		stMsg msg ;
		msg.cSysIdentifer = ID_MSG_PORT_CENTER ;
		msg.usMsgType = MSG_VERIFY_DB ;
		m_pNetWork->SendMsg((char*)&msg,sizeof(msg),pMsg->_connectID ) ;
		m_nCenterSvrConnectID = pMsg->_connectID ;
		CLogMgr::SharedLogMgr()->SystemLog("connected to Center Server") ;
	}
	else
	{
		CLogMgr::SharedLogMgr()->ErrorLog("connect center svr failed error code = %d",eSate ) ;
	}
	return true ;
}

bool CDBServerApp::OnLostSever( Packet* pMsg )
{
	CLogMgr::SharedLogMgr()->ErrorLog("center svr is lost") ;
	m_nCenterSvrConnectID = INVALID_CONNECT_ID ;
	return true ;
}

void CDBServerApp::SendMsg(const char* pBuffer, int nBufferLen, uint32_t nSessionID )
{
	if ( m_nCenterSvrConnectID == INVALID_CONNECT_ID )
	{
		CLogMgr::SharedLogMgr()->ErrorLog("center svr is discnnected") ;
		return ;
	}

	stMsgTransferData msgTransData ;
	msgTransData.nSenderPort = ID_MSG_PORT_DB ;
	msgTransData.bBroadCast = false ;
	msgTransData.nSessionID = nSessionID ;
	int nLne = sizeof(msgTransData) ;
	if ( nLne + nBufferLen >= MAX_MSG_BUFFER_LEN )
	{
		stMsg* pmsg = (stMsg*)pBuffer ;
		CLogMgr::SharedLogMgr()->ErrorLog("msg send to session id = %d , is too big , cannot send , msg id = %d ",nSessionID,pmsg->usMsgType) ;
		return ;
	}
	memcpy(m_pSendBuffer,&msgTransData,nLne);
	memcpy(m_pSendBuffer + nLne , pBuffer,nBufferLen );
	nLne += nBufferLen ;
	m_pNetWork->SendMsg(m_pSendBuffer,nLne,m_nCenterSvrConnectID ) ;
}

void CDBServerApp::OnExit()
{
	m_pNetWork->ShutDown();
	m_pDBWorkThread->StopWork();
	CLogMgr::SharedLogMgr()->SystemLog("DBServer ShutDown!");
}