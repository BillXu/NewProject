#pragma once
#include "cocos2d.h"
#include "cocos-ext.h"
USING_NS_CC ;
USING_NS_CC_EXT ;
class CCheckUpdateLayer
	:public Layer
{
public:
	static Scene* scene();
	bool init();
	CREATE_FUNC(CCheckUpdateLayer);
	void updateCallBack(EventAssetsManagerEx* pEvent);
	void onUpdateFinish();
protected:
	AssetsManagerEx* m_pmg ;
	LabelTTF* m_pText ;
};