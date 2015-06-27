#pragma once
#include "IBaseScene.h"
#include "cocos2d.h"
#include "cocostudio/CocoStudio.h"
#include "ui/CocosGUI.h"
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
	ui::EditBox* m_pAccount ;
	ui::EditBox* m_pPassword ;
	ui::EditBox* m_pName ;

};
