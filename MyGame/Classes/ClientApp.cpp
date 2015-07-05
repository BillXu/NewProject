#include "ClientApp.h"
#include "cocos2d.h"
#include "loginScene.h"
#include "CardPoker.h"
CClientApp* CClientApp::s_ClientApp = NULL ;
CClientApp* CClientApp::getInstance()
{
	if ( s_ClientApp == nullptr )
	{
		s_ClientApp = new CClientApp ;
	}
	return s_ClientApp ;
}

void CClientApp::destroyInstance()
{
	if ( s_ClientApp )
	{
		delete s_ClientApp ;
		s_ClientApp = nullptr ;
	}
}

CClientApp::CClientApp()
{
	m_pNetwork = NULL ;
	m_pConnectID = INVALID_CONNECT_ID ;
}

CClientApp::~CClientApp()
{
	if ( m_pNetwork )
	{
		m_pNetwork->ShutDown() ;
		delete m_pNetwork ;
		m_pNetwork = nullptr ;
	}
}

bool CClientApp::init()
{
	if ( m_pNetwork == nullptr )
	{
		m_pNetwork = new CNetWorkMgr ;
		m_pNetwork->AddMessageDelegate(this);
	}

	m_eNetState = CNetWorkMgr::eConnectType_None ;

	FileUtils::getInstance()->addSearchPath("res/");
	// create a scene. it's an autorelease object
	auto scene = CLoginScene::createLoginScene();
	// run
	 cocos2d::Director::getInstance()->runWithScene(scene);

	 // connect to svr
	 connectToSvr();
	 cocos2d::Director::getInstance()->getScheduler()->schedule([=](float fTime){ update(fTime);},this,0,false,"clientUpdate");
	return true ;
}

void CClientApp::enterForground()
{
	connectToSvr();
	cocos2d::Director::getInstance()->getScheduler()->unscheduleAllForTarget(this);
	cocos2d::Director::getInstance()->getScheduler()->schedule([=](float fTime){ update(fTime);},this,0,false,"clientUpdate");
}

void CClientApp::enterBackground()
{
	disconnectFromSvr();
	cocos2d::Director::getInstance()->getScheduler()->unscheduleAllForTarget(this);
}

void CClientApp::connectToSvr()
{
	if ( m_eNetState == CNetWorkMgr::eConnectType_Connected || m_eNetState == CNetWorkMgr::eConnectType_Connecting )
	{
		CCLOG("connecting or connected so do not need connect to \n");
		return ;
	}

	if ( m_pNetwork )
	{
		m_pNetwork->SetupNetwork(1) ;
		m_pNetwork->ConnectToServer("203.186.75.136",50001);
	}
	m_eNetState = CNetWorkMgr::eConnectType_Connecting ;
	CCLOG("start to connectting tosvr");
}

void CClientApp::disconnectFromSvr()
{
	if ( m_pNetwork )
	{
		m_pNetwork->DisconnectServer(m_pConnectID);
		m_pNetwork->ShutDown();
	}
	m_eNetState = CNetWorkMgr::eConnectType_None ;
}

// net delegate 
bool CClientApp::OnMessage( Packet* pMsg )
{
	return false ;
}

bool CClientApp::OnLostSever(Packet* pMsg)
{
	m_pConnectID = INVALID_CONNECT_ID ;
	m_eNetState = CNetWorkMgr::eConnectType_Disconnectd ;
	CCLOG("svr disconnect");
	return false ;
}

bool CClientApp::OnConnectStateChanged( eConnectState eSate, Packet* pMsg)
{
	if ( eSate == CNetMessageDelegate::eConnect_Accepted )
	{
		m_pConnectID = pMsg->_connectID ;
		m_eNetState = CNetWorkMgr::eConnectType_Connected ;
		stMsg msgVerify ;
		msgVerify.cSysIdentifer = ID_MSG_VERIFY ;
		msgVerify.usMsgType = MSG_VERIFY_CLIENT ;
		sendMsg(&msgVerify,sizeof(msgVerify));
		CCLOG("connect to svr success ");
	}
	else
	{
		m_eNetState = CNetWorkMgr::eConnectType_None ;
		CCLOG("connect to svr failed ");
	}
	return false ;
}

bool CClientApp::sendMsg(stMsg* pMsg , uint16_t nLen )
{
	if ( isConnecting() )
	{
		return m_pNetwork->SendMsg((char*)pMsg,nLen,m_pConnectID ) ;
	}
	else
	{
		CCLOG("server is disconnected , cann't send msg\n");
	}
	return false ;
}

bool CClientApp::isConnecting()
{
	return (m_pConnectID != INVALID_CONNECT_ID ) && m_eNetState == CNetWorkMgr::eConnectType_Connected ;
}


void CClientApp::update(float fDeta )
{
	if ( m_pNetwork )
	{
		m_pNetwork->ReciveMessage() ;
	}
}

void CClientApp::addMsgDelegate(CNetMessageDelegate* pDelegate )
{
	if ( m_pNetwork )
	{
		m_pNetwork->AddMessageDelegate(pDelegate) ;
	}
}

void CClientApp::removeMsgDelegate( CNetMessageDelegate* pDelegate )
{
	if ( m_pNetwork )
	{
		m_pNetwork->RemoveMessageDelegate(pDelegate);
	}
}

SpriteFrame* CClientApp::getCardSpriteByCompsiteNum(uint16_t nNum )
{
	CCard tc ;
	tc.RsetCardByCompositeNum(nNum);
	std::string spriteValue  ;
	uint8_t nType = 0 ;
	switch ( tc.GetType() )
	{
	case CCard::eCard_Diamond:
		nType = 4 ;
		break;
	case CCard::eCard_Heart:
		nType = 2 ;
	case CCard::eCard_Sword:
		nType = 1 ;
		break;
	case CCard::eCard_Club:
		nType = 3 ;
		break;
	default:
		break;
	}

	if ( nType == 0 || tc.GetCardCompositeNum() == 0 )
	{
		return nullptr ;
	}

	if ( tc.GetCardFaceNum(true) <= 9 )
	{
		spriteValue = String::createWithFormat("%d0%d.png",nType,tc.GetCardFaceNum(true))->getCString();
	}
	else
	{
		spriteValue = String::createWithFormat("%d0%c.png",nType,'a'+ tc.GetCardFaceNum(true)%10)->getCString();
	}

	return SpriteFrameCache::getInstance()->getSpriteFrameByName(spriteValue.c_str());
}
