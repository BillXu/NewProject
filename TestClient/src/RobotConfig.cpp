#include "RobotConfig.h"
CRobotConfigFile::~CRobotConfigFile()
{
	LIST_ROBOT_ITEM::iterator iter = m_vListRobot.begin();
	for ( ; iter != m_vListRobot.end(); ++iter )
	{
		delete *iter ;
	}
	m_vListRobot.clear();
}

bool CRobotConfigFile::OnPaser(CReaderRow& refReaderRow )
{
	if ( refReaderRow["IsEnable"]->IntValue() == 0 )
	{
		return true ;
	}
	stRobotItem* pItem = new stRobotItem ;
	pItem->nRobotID = refReaderRow["RobotID"]->IntValue();
	pItem->fMustWinRate = refReaderRow["MustWinRate"]->FloatValue();
	pItem->nAIConfigID = refReaderRow["AIConfigID"]->IntValue();
	pItem->nApplyLeaveWhenPeerCount = refReaderRow["LeaveWhenPlayerCount"]->IntValue();
	pItem->nMinLeftDiamond = refReaderRow["MinLeftDiamoned"]->IntValue();
	pItem->nMinLeftCoin = refReaderRow["MinLeftCoin"]->IntValue();
	pItem->strAccount = refReaderRow["RobotAccount"]->StringValue();
	pItem->strPassword = refReaderRow["RobotPassword"]->StringValue();
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

