#include "LoginApp.h"
#include "LogManager.h"
#include "ServerMessageDefine.h"
#include "LoginDBManager.h"
#include "DataBaseThread.h"
CLoginApp::CLoginApp()
{
	m_pNetWork = NULL ;
	m_nCenterSvrNetworkID = INVALID_CONNECT_ID ;
	m_pDBThread = NULL ;
	m_pDBMgr = NULL ;

	memset(m_pSendBuffer,0,sizeof(m_pSendBuffer));
}

CLoginApp::~CLoginApp()
{
	if ( m_pNetWork )
	{
		m_pNetWork->ShutDown();
		delete m_pNetWork ;
		m_pNetWork = NULL ;
	}

	if ( m_pDBMgr )
	{
		delete m_pDBMgr ;
		m_pDBMgr = NULL ;
	}

	if ( m_pDBThread )
	{
		m_pDBThread->StopWork();
		delete m_pDBThread ;
		m_pDBThread = NULL ;
	}
}

void CLoginApp::Init()
{
	CLogMgr::SharedLogMgr()->SetOutputFile("LoginSvr");
	m_stSvrConfigMgr.LoadFile("../configFile/serverConfig.txt");
	

	// start Db thread 
	stServerConfig* pSvrConfigItem = m_stSvrConfigMgr.GetServerConfig(eSvrType_DataBase );
	if ( pSvrConfigItem == NULL )
	{
		CLogMgr::SharedLogMgr()->ErrorLog("Data base config is null , can not start login svr ") ;
		return ;
	}
	m_pDBThread = new CDataBaseThread ;
	bool bRet = m_pDBThread->InitDataBase(pSvrConfigItem->strIPAddress,pSvrConfigItem->nPort,pSvrConfigItem->strAccount,pSvrConfigItem->strPassword,"taxpokerdb");
	if ( bRet )
	{
		m_pDBThread->Start() ;
		CLogMgr::SharedLogMgr()->SystemLog("start db thread ok") ;
	}
	else
	{	
		delete m_pDBThread ;
		m_pDBThread = NULL ;
		CLogMgr::SharedLogMgr()->ErrorLog("start db thread errror ") ;
		return ;
	}

	m_pDBMgr = new CDBManager;
	m_pDBMgr->Init(this);

	// connected to center ;
	pSvrConfigItem = m_stSvrConfigMgr.GetServerConfig(eSvrType_Center );
	if ( pSvrConfigItem == NULL )
	{
		CLogMgr::SharedLogMgr()->ErrorLog("center svr config is null so can not start svr ") ;
		delete m_pNetWork ;
		m_pNetWork = NULL ;
		return ;
	}
	m_pNetWork = new CNetWorkMgr ;
	m_pNetWork->SetupNetwork(2);
	m_pNetWork->AddMessageDelegate(this);
	m_pNetWork->ConnectToServer(pSvrConfigItem->strIPAddress,pSvrConfigItem->nPort,pSvrConfigItem->strPassword) ;
	CLogMgr::SharedLogMgr()->SystemLog("connectting to center svr...") ;

}

void CLoginApp::MainLoop()
{
	while( true )
	{
		if ( m_pNetWork) 
		{
			m_pNetWork->ReciveMessage() ;
		}
		Sleep(1);
	}
	
	if ( m_pDBThread )
	{
		m_pDBThread->StopWork();
	}
}

bool CLoginApp::OnMessage( Packet* pMsg )
{
	CHECK_MSG_SIZE(stMsg,pMsg->_len) ;
	stMsg* pmsg = (stMsg*)pMsg->_orgdata ;
	if ( pmsg->cSysIdentifer == ID_MSG_VERIFY )
	{
		CLogMgr::SharedLogMgr()->SystemLog("no need recieve verify msg") ;
		return true ;
	}

	stMsg* pRet = (stMsg*)pMsg->_orgdata ;
	if ( pRet->usMsgType != MSG_TRANSER_DATA )
	{
		CLogMgr::SharedLogMgr()->ErrorLog("why msg type is not transfer data , type = %d",pRet->usMsgType ) ;
		return true;
	}

	stMsgTransferData* pData = (stMsgTransferData*)pRet ;
	if ( m_pDBMgr )
	{
		stMsg* preal = (stMsg*)(pMsg->_orgdata + sizeof(stMsgTransferData));
		m_pDBMgr->OnMessage(preal,(eMsgPort)pData->nSenderPort,pData->nSessionID ) ;
	}
	return true ;
}

bool CLoginApp::OnLostSever(Packet* pMsg)
{
	m_pNetWork->DisconnectServer(pMsg->_connectID);
	m_nCenterSvrNetworkID = INVALID_CONNECT_ID ;
	CLogMgr::SharedLogMgr()->ErrorLog("center svr lost ") ;
	return true ;
}

bool CLoginApp::OnConnectStateChanged( eConnectState eSate, Packet* pMsg)
{
	if ( eSate == eConnect_Accepted )
	{
		stMsg cMsg ;
		cMsg.cSysIdentifer = ID_MSG_PORT_CENTER ;
		cMsg.usMsgType = MSG_VERIFY_LOGIN ;
		m_pNetWork->SendMsg((char*)&cMsg,sizeof(stMsg),pMsg->_connectID) ;
		CLogMgr::SharedLogMgr()->SystemLog("Connected to Center Svr") ;
		return true ;
	}
	CLogMgr::SharedLogMgr()->ErrorLog("connect Center svr failed") ;
	return true ;
}

bool CLoginApp::SendMsg( const char* pBuffer , unsigned int nBufferLen, uint32_t nSessioniD  )
{
	if ( m_pNetWork == NULL || m_nCenterSvrNetworkID == INVALID_CONNECT_ID  )
	{
		CLogMgr::SharedLogMgr()->ErrorLog("center svr is not connected can not send msg") ;
		return false ;
	}

	stMsgTransferData msgTransData ;
	msgTransData.nSenderPort = ID_MSG_PORT_LOGIN ;
	msgTransData.bBroadCast = false ;
	msgTransData.nSessionID = nSessioniD ;
	int nLne = sizeof(msgTransData) ;
	if ( nLne + nBufferLen >= MAX_MSG_BUFFER_LEN )
	{
		stMsg* pmsg = (stMsg*)pBuffer ;
		CLogMgr::SharedLogMgr()->ErrorLog("msg send to session id = %d , is too big , cannot send , msg id = %d ",nSessioniD,pmsg->usMsgType) ;
		return false;
	}
	memcpy(m_pSendBuffer,&msgTransData,nLne);
	memcpy(m_pSendBuffer + nLne , pBuffer,nBufferLen );
	nLne += nBufferLen ;
	m_pNetWork->SendMsg(m_pSendBuffer,nLne,m_nCenterSvrNetworkID ) ;
	return true ;
}