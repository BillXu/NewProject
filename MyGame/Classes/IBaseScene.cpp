#include "IBaseScene.h"
#include "ClientApp.h"
#include "LogManager.h"
//#include "GotyeAPI.h"
bool IBaseScene::OnLostSever()
{
	cocos2d::MessageBox("lost server connect","error");
	return false ;
}

bool IBaseScene::onMsg(stMsg* pmsg )
{ 
	if ( MSG_PLAYER_OTHER_LOGIN == pmsg->usMsgType )
	{
		cocos2d::MessageBox("account was logined in from other device","Warnning");
		CClientApp::getInstance()->disconnectFromSvr() ; // as the account was logined other device , so we must dis connect svr 
	}
	return false ; 
}

void IBaseScene::onEnter()
{
	Node::onEnter() ;
	CClientApp::getInstance()->addMsgDelegate(this);
	//GotyeAPI::getInstance()->addListener(*this);
}

void IBaseScene::onExit()
{
	Node::onExit();
	CClientApp::getInstance()->removeMsgDelegate(this);
	/*GotyeAPI::getInstance()->removeListener(*this);*/
}

void IBaseScene::sendMsg(stMsg* pmsg, uint16_t nLen )
{
	if ( ! CClientApp::getInstance()->sendMsg(pmsg,nLen) )
	{
		cocos2d::MessageBox("send msg failed","network disconnect");
	}
}

void IBaseScene::onLogin(GotyeStatusCode code, const GotyeLoginUser& user)
{
	if ( GotyeStatusCodeOK != code )
	{
		CCLOG("login error ret = %d",code);
		CCLOG("socail login success, name = %s, nickeName = %s",user.name.c_str(),user.nickname.c_str());
	}
	else
	{
		CCLOG("socail login success, name = %s, nickeName = %s",user.name.c_str(),user.nickname.c_str());
	}
}

void IBaseScene::onLogout(GotyeStatusCode code)
{
	CCLOG("do logout ret = %d",code);
}