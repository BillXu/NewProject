#include "MainScene.h"
#include "ClientApp.h"
#include "TaxasMessageDefine.h"
cocos2d::Scene* CMainScene::createScene()
{
	auto pScene = Scene::create();
	auto layer = CMainScene::create() ;
	pScene->addChild(layer);
	return pScene ;
}

bool CMainScene::init()
{
	IBaseScene::init();
	Node* pRoot = CSLoader::getInstance()->createNodeWithFlatBuffersFile("res/playerData.csb");
	addChild(pRoot);
	auto cSize = pRoot->getContentSize();
	pRoot->setContentSize(Director::getInstance()->getWinSize()) ;
	ui::Helper::doLayout(pRoot);

	m_pName = (Label*)pRoot->getChildByName("name");
	m_pCoin = (Label*)pRoot->getChildByName("coin");
	m_pID = (Label*)pRoot->getChildByName("id");

	auto pBtn = (ui::Button*)pRoot->getChildByName("Button_1");
	pBtn->addClickEventListener([=](Ref*){
		//cocos2d::MessageBox("send enter room msg","tip");
		stMsgTaxasEnterRoom msgEnterRoom ;
		msgEnterRoom.nLevel = 0 ;
		msgEnterRoom.nRoomID = 0 ;
		msgEnterRoom.nType = 0 ;
		sendMsg(&msgEnterRoom,sizeof(msgEnterRoom)) ;
	});

	m_pCoin->setString(String::createWithFormat("%d",CClientApp::getInstance()->getPlayerData()->nCoin)->getCString()) ;
	m_pID->setString(String::createWithFormat("%d",CClientApp::getInstance()->getPlayerData()->nUserUID)->getCString()) ;
	//m_pCoin->setString(String::createWithFormat("%d",CClientApp::getInstance()->getPlayerData()->nCoin)->getCString()) ;
	m_pName->setString(CClientApp::getInstance()->getPlayerData()->cName);
	return true ;
}

bool CMainScene::onMsg(stMsg* pmsg )
{
	if ( MSG_TP_ENTER_ROOM == pmsg->usMsgType )
	{
		// change to poker scene
		return true ;
	}
	return false ;
}