#pragma once
#include "IBaseScene.h"
#include "cocos2d.h"
USING_NS_CC ;
class CLoginScene
	:public IBaseScene
	,ui::EditBoxDelegate
{
public:
	static cocos2d::Scene* createLoginScene();
	CREATE_FUNC(CLoginScene);
	bool init();
	bool onMsg(stMsg* pmsg ) ;
	void editBoxReturn(ui::EditBox* editBox);
protected:
	ui::TextField* m_pAccount ;
	ui::TextField* m_pPassword ;
	ui::TextField* m_pName ;
};
