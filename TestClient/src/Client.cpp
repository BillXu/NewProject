#include <windows.h>
#include "Client.h"
#include "MessageDefine.h"
#include "CommonDefine.h"
#include "LoginScene.h"
#include <synchapi.h>
#include "commonCmdScene.h"
#define  TIME_FOR_RECONNECT 5
CClientRobot::CClientRobot()
{
	m_pPlayerData = new CPlayerData ;
	m_pCurentScene = NULL ;
	m_pLastScene = NULL ;
	m_bWaitReonnect = false;
	m_fReconnectTick = TIME_FOR_RECONNECT;
	m_strTargetIP = "" ;
	m_nTargetPort = 0 ;
}

CClientRobot::~CClientRobot()
{
	delete m_pPlayerData ;
	if ( m_pCurentScene )
	{
		m_pCurentScene->OnEixtScene();
		delete m_pCurentScene ;
	}
}

bool CClientRobot::Init(const char* pIPString, unsigned short nPort )
{
	m_pNetWork.SetupNetwork();  
	m_pCurentScene = new commonCmdScene(this) ;//new CLoginScene(this);
	m_pCurentScene->OnEnterScene();
	m_pNetWork.ConnectToServer(pIPString,nPort,"123456") ;  // inner net ;-
	//m_pNetWork.ConnectToServer("203.186.75.136",50001,"123456") ;  // Out net ;
	m_pNetWork.AddMessageDelegate(this,2) ;
	m_pNetWork.AddMessageDelegate(m_pPlayerData) ;
	m_strTargetIP = pIPString ;
	m_nTargetPort = nPort ;
	return true ;
}

void CClientRobot::Run()
{
	clock_t nTick = clock();
	while ( 1  )
	{
		m_pNetWork.ReciveMessage();

		float fElasp = (float)(clock() - nTick ) / (float)CLOCKS_PER_SEC ;
		nTick = clock();
		if ( m_pCurentScene)
		{
			m_pCurentScene->OnUpdate(fElasp);
		}

		processReconnect(fElasp);
		Sleep(2);
		if ( m_pLastScene )
		{
			delete m_pLastScene ;
			m_pLastScene = NULL ;
		}


	}
	m_pNetWork.ShutDown();
}

void CClientRobot::ChangeScene(IScene* pTargetScene )
{
	if ( m_pCurentScene )
	{
		m_pCurentScene->OnEixtScene() ;
		m_pCurentScene = m_pCurentScene;
	}
	pTargetScene->OnEnterScene() ;
	delete m_pCurentScene ;
	m_pCurentScene = nullptr ;
	m_pCurentScene = pTargetScene ;
}

bool CClientRobot::OnConnectStateChanged( eConnectState eSate, Packet* pMsg )
{
	if ( eConnect_Accepted == eSate )
	{
		// send client verify ;
		stMsg msg ;
		msg.cSysIdentifer = ID_MSG_PORT_GATE ;
		msg.usMsgType = MSG_VERIFY_CLIENT ;
		GetNetWork()->SendMsg((char*)&msg,sizeof(msg));
		printf("connected to svr \n") ;

		//if ( GetSessionID() != 0 )  // means reconnect ;
		//{
		//	stMsgReconnect msgRec ;
		//	msgRec.nSessionID = GetSessionID();
		//	GetNetWork()->SendMsg((char*)&msgRec,sizeof(msgRec));
		//	return true ;
		//}

	}
	else if ( eConnect_Failed == eSate )
	{
		m_bWaitReonnect = true ;
	}
	return false ;
}

bool CClientRobot::OnLostSever()
{
	m_bWaitReonnect = true ;
	m_fReconnectTick = TIME_FOR_RECONNECT ;

	printf("lost server connect , change to login scene\n") ;
	auto au = new CLoginScene(this);
	ChangeScene(au);
	return true ;
}

void CClientRobot::processReconnect( float fDelt )
{
	if ( !m_bWaitReonnect )
	{
		return ;
	}

	m_fReconnectTick -= fDelt ;
	if ( m_fReconnectTick <= 0 )
	{
		m_pNetWork.ConnectToServer(m_strTargetIP.c_str(),m_nTargetPort,"123456") ; 
		m_fReconnectTick = TIME_FOR_RECONNECT ;
		m_bWaitReonnect = false ;
		printf("start reconnect\n") ;
	}
}

void CClientRobot::sendMsg(Json::Value& jsMsg , uint16_t nTargetPort )
{
	stMsgJsonContent msg ;
	msg.cSysIdentifer = nTargetPort ;
	static char pBuffer[2048] ;
	memset(pBuffer,0,sizeof(pBuffer));
	Json::StyledWriter jsw ;
	auto p = jsw.write(jsMsg) ;
	msg.nJsLen = p.size() ;
	memcpy(pBuffer,&msg,sizeof(msg));
	memcpy(pBuffer + sizeof(msg),p.c_str(),msg.nJsLen);
	sendMsg((stMsg*)pBuffer,sizeof(msg) + msg.nJsLen) ;
}

void CClientRobot::sendMsg(stMsg* pmsg ,uint16_t nLen )
{
	GetNetWork()->SendMsg((char*)pmsg,nLen) ;
}

bool CClientRobot::OnMessage( Packet* pMsg )
{
	stMsg* pmsg = (stMsg*)pMsg->_orgdata ;
	if ( pmsg->usMsgType == MSG_RECONNECT )
	{
		stMsgReconnectRet* pRet = (stMsgReconnectRet*)pmsg ;
		if ( pRet->nRet )
		{
			printf("robot reconnect failed \n") ;
		}
		else
		{
			printf("robot reconnect success \n") ;
		}
		return true ;
	}

	if ( MSG_JSON_CONTENT == pmsg->usMsgType )
	{
		stMsgJsonContent* pret = (stMsgJsonContent*)pmsg ;
		char* pBuffer = (char*)pmsg ;
		pBuffer += sizeof(stMsgJsonContent);
		Json::Reader jsReaer ;
		Json::Value js ;
		jsReaer.parse(pBuffer,pBuffer + pret->nJsLen,js,false);
		return m_pCurentScene->onMessage(js) ;
	}
	else
	{
		if ( m_pCurentScene->onMessage(pmsg) )
		{
			return true ;
		}
	}
	return m_pCurentScene->OnMessage(pMsg) ;
}
