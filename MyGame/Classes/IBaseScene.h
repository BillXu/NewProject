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
	virtual bool OnMessage( stMsg* pMsg )final{ return onMsg(pMsg) ;} ;
	virtual bool onMsg(stMsg* pmsg );
	virtual bool OnLostSever();
	virtual void onEnter();
	virtual void onExit();
	void sendMsg(stMsg* pmsg, uint16_t nLen );
};