#pragma once
#include "IConfigFile.h"
#include <list>
class CRobotConfigFile
	:public IConfigFile
{
public:
	struct stRobotItem
	{
		unsigned short nRobotID ;
		std::string strAccount ;
		std::string strPassword ;
		unsigned short nAIConfigID ;
		unsigned int  nMinLeftCoin ;
		unsigned int nMinLeftDiamond ;
		float fMustWinRate ; // 100 percent ;
		unsigned char nApplyLeaveWhenPeerCount ;
		float fActDelayBegin;
		float fActDelayEnd ;
	};
public:
	typedef std::list<stRobotItem*> LIST_ROBOT_ITEM ;
public:
	~CRobotConfigFile();
	virtual bool OnPaser(CReaderRow& refReaderRow ) ;
	virtual void OnFinishPaseFile();
	stRobotItem* EnumConfigItem();  // must enum to end ;
protected:
	LIST_ROBOT_ITEM::iterator enum_iter ;
	LIST_ROBOT_ITEM m_vListRobot ;
};