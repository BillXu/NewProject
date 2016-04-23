#pragma once
#include "IConfigFile.h"
#include <list>
class CRobotConfigFile
	:public IConfigFile
{
public:
	struct stRobotItem
	{
		struct stWorkPoint
		{
			unsigned char nHour ;
			unsigned char nMini ;
		};

		typedef std::list<stWorkPoint> VEC_WORK_POINT ;

		unsigned short nRobotID ;
		std::string strAccount ;
		std::string strPassword ;
		std::string strAiFileName;
		unsigned int  nMinLeftCoin ;
		unsigned int fMostLeftCoin ; // 100 percent ;
		unsigned int nMaxCanLoseCoin ; // 100 percent ;
		float fActDelayBegin;
		float fActDelayEnd ;
		short nLevel ;
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