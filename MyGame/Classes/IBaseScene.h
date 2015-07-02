#pragma once 
#include "cocos2d.h"
#include "NetWorkManager.h"
#include "CommonDefine.h"
#include "cocostudio/CocoStudio.h"
#include "ui/CocosGUI.h"
USING_NS_CC ;
struct stMsg ;
class IBaseScene
	:public Node,public CNetMessageDelegate
{
public:
	virtual bool OnMessage( Packet* pMsg ){ return onMsg((stMsg*)pMsg->_orgdata) ;} ;
	virtual bool onMsg(stMsg* pmsg );
	virtual bool OnLostSever(Packet* pMsg);
	virtual void onEnter();
	virtual void onExit();
	void sendMsg(stMsg* pmsg, uint16_t nLen );
};