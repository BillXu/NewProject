#include "RobotConfig.h"
#include "Utility.h"
CRobotConfigFile::~CRobotConfigFile()
{
	LIST_ROBOT_ITEM::iterator iter = m_vListRobot.begin();
	for ( ; iter != m_vListRobot.end(); ++iter )
	{
		delete *iter ;
	}
	m_vListRobot.clear();
}

bool vSortWorkTime( CRobotConfigFile::stRobotItem::stWorkPoint& left ,CRobotConfigFile::stRobotItem::stWorkPoint& right )
{
	if ( left.nHour == right.nHour )
	{
		return left.nMini < right.nMini ;
	}
	return left.nHour < right.nHour ;
}

bool CRobotConfigFile::OnPaser(CReaderRow& refReaderRow )
{
	if ( refReaderRow["IsEnable"]->IntValue() == 0 )
	{
		return true ;
	}
	stRobotItem* pItem = new stRobotItem ;
	pItem->nRobotID = refReaderRow["RobotID"]->IntValue();
	pItem->fMostLeftCoin = refReaderRow["MostLeftCoin"]->IntValue();
	pItem->nMinLeftCoin = refReaderRow["MinLeftCoin"]->IntValue();
	pItem->nMaxCanLoseCoin = refReaderRow["MaxCanLoseCoin"]->IntValue();
	pItem->nLevel = refReaderRow["Level"]->IntValue();
	if ( pItem->nMinLeftCoin >= pItem->fMostLeftCoin )
	{
		printf("robot coin range error , robot id = %u\n",pItem->nRobotID) ;
		pItem->fMostLeftCoin = pItem->nMinLeftCoin + 10000; 
	}
	pItem->strAccount = refReaderRow["RobotAccount"]->StringValue();
	pItem->strPassword = refReaderRow["RobotPassword"]->StringValue();
	pItem->strAiFileName = refReaderRow["AiFileName"]->StringValue();
	pItem->fActDelayBegin = refReaderRow["ActDelayBegin"]->FloatValue();
	pItem->fActDelayEnd = refReaderRow["ActDelayEnd"]->FloatValue() ;
	pItem->fActDelayBegin = pItem->fActDelayBegin < pItem->fActDelayEnd ? pItem->fActDelayBegin : pItem->fActDelayEnd ;
	pItem->fActDelayEnd = pItem->fActDelayBegin > pItem->fActDelayEnd ? pItem->fActDelayBegin : pItem->fActDelayEnd ;
	m_vListRobot.push_back(pItem) ;
	return true ;
}

void CRobotConfigFile::OnFinishPaseFile()
{
	enum_iter = m_vListRobot.begin() ;
}

CRobotConfigFile::stRobotItem* CRobotConfigFile::EnumConfigItem()
{
	stRobotItem* pItem = NULL ;
	if ( enum_iter == m_vListRobot.end() )
	{
		enum_iter = m_vListRobot.begin() ;
		return pItem ;
	}

	pItem = *enum_iter ;
	
	++enum_iter ;

	return pItem ;
}

