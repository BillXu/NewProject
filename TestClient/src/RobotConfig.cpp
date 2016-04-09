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
	pItem->nApplyLeaveWhenPeerCount = refReaderRow["LeaveWhenPlayerCount"]->IntValue();
	pItem->nMinLeftCoin = refReaderRow["MinLeftCoin"]->IntValue();

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
	pItem->nDstRoomID = refReaderRow["dstRoomID"]->IntValue();
	pItem->nDstGameType = refReaderRow["nDstGameType"]->IntValue() ;
	pItem->nDstSubIdx = refReaderRow["dstSubRoomIdx"]->IntValue() ;
	pItem->fActDelayBegin = pItem->fActDelayBegin < pItem->fActDelayEnd ? pItem->fActDelayBegin : pItem->fActDelayEnd ;
	pItem->fActDelayEnd = pItem->fActDelayBegin > pItem->fActDelayEnd ? pItem->fActDelayBegin : pItem->fActDelayEnd ;

	// parse work point ;
	VEC_STRING vOutS ;
	refReaderRow["workPoint"]->VecString(vOutS);
	if ( vOutS.empty() )
	{
		printf("work point is empty  robot id = %d \n",pItem->nRobotID) ;
		return true ;
	}

	for(auto str : vOutS )
	{
		CReaderCell t (str.c_str());
		std::vector<int> vTime ;
		t.VecInt(vTime,':');
		if (vTime.size() != 2 )
		{
			printf("work point format error , robot id = %d\n",pItem->nRobotID) ;
			continue;
		}

		stRobotItem::stWorkPoint pt ;
		pt.nHour = vTime[0];
		pt.nMini = vTime[1] ;

		if ( pt.nHour > 24 )
		{
			pt.nHour = 24 ;
			printf("work point hour big than 24 , robot id = %d\n",pItem->nRobotID) ;
		}

		if ( pt.nMini >= 60 )
		{
			pt.nMini = 59 ;
			printf("work point minite big than 60 , robot id = %d\n",pItem->nRobotID) ;
		}
		pItem->vWorkPoints.push_back(pt) ;
	}

	if ( pItem->vWorkPoints.empty() )
	{
		printf("work point parse error , robot id = %d\n",pItem->nRobotID) ;
		return true ;
	}

	if ( pItem->vWorkPoints.size() >= 2 )
	{
		pItem->vWorkPoints.sort(vSortWorkTime);
	}
	printf("robot id = %d , work point = %d\n",pItem->nRobotID,pItem->vWorkPoints.size()) ;
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

