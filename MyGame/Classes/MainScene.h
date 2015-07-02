#pragma once
#include "IBaseScene.h"
class CMainScene
	:public IBaseScene
{
public:
	static cocos2d::Scene* createScene();
	CREATE_FUNC(CMainScene);
	bool init();
	bool onMsg(stMsg* pmsg );
protected:
	Label* m_pName ,*m_pCoin,*m_pID;
};