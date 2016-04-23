#include "PlayerData.h"
#include "MessageDefine.h"
#include "LogManager.h"
bool CPlayerData::OnMessage( Packet* pMsg )
{
	stMsg* pmsg = (stMsg*)pMsg->_orgdata ;
	if ( pmsg->usMsgType == MSG_PLAYER_BASE_DATA )
	{
		stMsgPlayerBaseData* pRet = (stMsgPlayerBaseData*)pmsg ;
		memcpy(&stBaseData,&pRet->stBaseData,sizeof(stBaseData));

	}

	if ( pmsg->usMsgType == MSG_PLAYER_UPDATE_MONEY )
	{
		stMsgPlayerUpdateMoney* pRet = (stMsgPlayerUpdateMoney*)pMsg;
		stBaseData.nCoin = pRet->nFinalCoin ;
	}

	if ( MSG_REQ_TOTAL_GAME_OFFSET == pmsg->usMsgType )
	{
		stMsgReqRobotTotalGameOffsetRet* pRet = (stMsgReqRobotTotalGameOffsetRet*)pMsg ;
		m_nTotalOffset = pRet->nTotalGameOffset ;
	}
 
	return false ;
}

void CPlayerData::OnWinCoin( int64_t nCoinOffset, bool bDiamoned  )
{
	if ( bDiamoned )
	{
		stBaseData.nDiamoned += nCoinOffset ;
	}
	else
	{
		stBaseData.nCoin += nCoinOffset ;
	}
}

int64_t CPlayerData::GetCoin(bool bDiamond )
{ 
	return ( bDiamond ?stBaseData.nDiamoned: stBaseData.nCoin) ;
}
void CPlayerData::TakeIn(uint64_t nCoin , bool bDimoned)
{
	if ( !bDimoned )
	{
		 uint64_t nTakeInCoin = nCoin < stBaseData.nCoin ? nCoin : stBaseData.nCoin ;
		stBaseData.nCoin -= nTakeInCoin ;
		printf("%s take in %d, leftAll = %I64d\n",stBaseData.cName,nTakeInCoin,stBaseData.nCoin) ;
	}
	else
	{
		uint64_t nTakeInDiamoned = nCoin < stBaseData.nDiamoned ? nCoin : stBaseData.nDiamoned ;
		stBaseData.nDiamoned -= nTakeInDiamoned ;
		printf("%s take in %d, leftAll = %d\n",stBaseData.cName,nTakeInDiamoned,stBaseData.nDiamoned) ;
	}
}

bool CPlayerData::IsMustWin( bool bDiamoned )
{
	return false ;
	//if ( bDiamoned )
	//{
	//	if ( nDiamoned + nTakeInDiamoned < pRobotItem->nMinLeftDiamond  )
	//	{
	//		float frate = (float)rand() / float(RAND_MAX) ;
	//		frate *= 100 ;
	//		if ( frate <= pRobotItem->fMustWinRate )
	//		{
	//			return true ;
	//		}
	//		else
	//		{
	//			return false ;
	//		}
	//	}
	//	else
	//	{
	//		return false ;
	//	}
	//}
	//else
	//{
	//	if ( nTakeInCoin + nMyCoin < pRobotItem->nMinLeftCoin  )
	//	{
	//		float frate = (float)rand() / float(RAND_MAX) ;
	//		frate *= 100 ;
	//		if ( frate <= pRobotItem->fMustWinRate )
	//		{
	//			return true ;
	//		}
	//		else
	//		{
	//			return false ;
	//		}
	//	}
	//	else
	//	{
	//		return false ;
	//	}
	//}
	//return true ;
}