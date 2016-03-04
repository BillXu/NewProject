#include "NiuNiuData.h"
#include <string>
#include "CardPoker.h"
#include "RobotControlNiuNiu.h"
stNiuNiuData::stNiuNiuData()
{
	nBankerBetTimes = 0 ;
	nBankerIdx = 0 ;
}

void stNiuNiuData::onGameBegin()
{
	CSitableRoomData::onGameBegin() ;
	nBankerBetTimes = 0 ;
	nBankerIdx = 0 ;
}

uint32_t stNiuNiuData::getLeftCanBetCoin()
{
	uint16_t nPlayerTimes = 0  ;
	for ( uint8_t nIdx = 0 ; nIdx < getSeatCount(); ++nIdx )
	{
		stNiuNiuPlayer* pPlayer = (stNiuNiuPlayer*)getPlayerByIdx(nIdx);
		if ( pPlayer == nullptr )
		{
			continue;
		}

		if ( pPlayer->isHaveState(eRoomPeer_CanAct) == false )
		{
			continue; 
		}

		if ( pPlayer->nBetTimes == 0 )
		{
			nPlayerTimes += 5 ;
		}
		else 
		{
			nPlayerTimes += pPlayer->nBetTimes ;
		}
	}

	auto pBanker = (stNiuNiuPlayer*)getPlayerByIdx(nBankerIdx);
	if ( getFinalBaseBet() * nPlayerTimes > pBanker->nCoin )
	{
		return 0 ;
	}

	return pBanker->nCoin - getFinalBaseBet() * nPlayerTimes ;
}

uint32_t stNiuNiuData::getFinalBaseBet()
{
	return nBankerBetTimes * nBottomBet ;
}

bool stNiuNiuData::isHaveNiu(uint8_t nIdx )
{
	stNiuNiuPlayer* pPlayerItem = (stNiuNiuPlayer*)getPlayerByIdx(nIdx) ;
	if ( nullptr == pPlayerItem )
	{
		return false ;
	}

	unsigned char vPoint[4] = { 0 } ;
	CCard tSingle ;
	for ( uint8_t nIdx = 0 ; nIdx < 4 ; ++nIdx )
	{
		if ( (pPlayerItem->vHoldChard[nIdx] > 0 && pPlayerItem->vHoldChard[nIdx] <= 54) != true )
		{
			printf("recived card error can not tell wheth have niu , num = %d\n") ;
			return false ;
		}

		tSingle.RsetCardByCompositeNum(pPlayerItem->vHoldChard[nIdx]) ;
		vPoint[nIdx] = tSingle.GetCardFaceNum(false);
		if ( vPoint[nIdx] > 10 )
		{
			vPoint[nIdx] = 10 ;
		}
	}

	return ( (vPoint[0] + vPoint[1] + vPoint[2] ) % 10 == 0 ) || ( (vPoint[0] + vPoint[3] + vPoint[2] ) % 10 == 0 ) || ( (vPoint[0] + vPoint[1] + vPoint[3] ) % 10 == 0 ) ;
}

bool stNiuNiuData::onMsg(stMsg* pmsg)
{
	if ( CSitableRoomData::onMsg(pmsg) )
	{
		return true ;
	}

	switch ( pmsg->usMsgType )
	{
	case MSG_ROOM_ENTER_NEW_STATE:
		{
			stMsgRoomEnterNewState* pRet = (stMsgRoomEnterNewState*)pmsg ;
			if ( pRet->nNewState == eRoomState_NN_TryBanker )
			{
				getRobotControl()->informRobotAction(CRobotControlNiuNiu::eAct_TryBanker) ;
			}
			else if ( eRoomState_NN_StartBet == pRet->nNewState )
			{
				getRobotControl()->informRobotAction(CRobotControlNiuNiu::eAct_Bet) ;
			}
		}
		break;
	case MSG_NN_DISTRIBUTE_4_CARD:
		{
			onGameBegin();
			stMsgNNDistriute4Card* pRet = (stMsgNNDistriute4Card*)pmsg ;
			stDistriuet4CardItem* pItem = (stDistriuet4CardItem*)(((char*)pmsg) + sizeof(stMsgNNDistriute4Card));
			while ( pRet->nPlayerCnt-- )
			{
				if ( pItem->nSeatIdx >= 5 )
				{
					continue; ;
				}

				stNiuNiuPlayer* pPlayerItem = (stNiuNiuPlayer*)getPlayerByIdx(pItem->nSeatIdx) ;
				if ( pPlayerItem )
				{
					memcpy(pPlayerItem->vHoldChard,pItem->vCardCompsitNum,sizeof(pItem->vCardCompsitNum));
				}
			}
		}
		break;
	case MSG_NN_ROOM_INFO:
		{
			stMsgNNRoomInfo* pInfo = (stMsgNNRoomInfo*)pmsg ;
			nBankerIdx = pInfo->nBankerIdx;
			nBottomBet = pInfo->nBottomBet ;
			nBankerBetTimes = pInfo->nBankerBetTimes;
			setBaseInfo(pInfo->nRoomID,5,pInfo->nDeskFee,pInfo->nRoomState);

			char* pBuffer = (char*)pmsg ;
			pBuffer = pBuffer + sizeof(stMsgNNRoomInfo);
			stNNRoomInfoPayerItem* pPlayerItem = (stNNRoomInfoPayerItem*)pBuffer ;
			while ( pInfo->nPlayerCnt-- )
			{
				stNiuNiuPlayer* pPlayer = (stNiuNiuPlayer*)getPlayerByIdx(pPlayerItem->nIdx) ;
				if ( pPlayer == nullptr )
				{
					pPlayer = new stNiuNiuPlayer ;
				}
				pPlayer->reset();
				pPlayer->nBetTimes = pPlayerItem->nBetTimes ;
				pPlayer->nCoin = pPlayerItem->nCoin ;
				pPlayer->nIdx = pPlayerItem->nIdx ;
				pPlayer->nUserUID = pPlayerItem->nUserUID ;
				pPlayer->nStateFlag = pPlayerItem->nStateFlag ;
				memcpy(pPlayer->vHoldChard,pPlayerItem->vHoldChard,sizeof(pPlayer->vHoldChard));
				++pPlayerItem ;
			}
		}
		break;
	case MSG_NN_GAME_RESULT:
		{
			stMsgNNGameResult*pRet = (stMsgNNGameResult*)pmsg ;
			char* pBuffer = (char*)pmsg ;
			pBuffer = pBuffer + sizeof(stMsgNNGameResult) ;
			stNNGameResultItem* pItem = (stNNGameResultItem*)pBuffer ;
			while ( pRet->nPlayerCnt-- )
			{
				stNiuNiuPlayer* pPlayer = (stNiuNiuPlayer*)getPlayerByIdx(pItem->nPlayerIdx) ;
				if ( pPlayer )
				{
					pPlayer->nCoin = pItem->nFinalCoin ;
				}
			}
			onGameEnd();
		}
		break;
	case MSG_NN_PRODUCED_BANKER:
	case MSG_NN_RAND_BANKER:
		{
			stMsgNNProducedBanker* pRet = (stMsgNNProducedBanker*)pmsg;
			nBankerIdx = pRet->nBankerIdx ;
			nBankerBetTimes = pRet->nBankerBetTimes ;
		}
		break;
	case MSG_NN_BET:
		{
			stMsgNNBet* pRet = (stMsgNNBet*)pmsg ;
			stNiuNiuPlayer* pPlayer = (stNiuNiuPlayer*)getPlayerByIdx(pRet->nPlayerIdx) ;
			if ( pPlayer )
			{
				pPlayer->nBetTimes = pRet->nBetTimes ;
			}
		}
		break;
	default:
		return false;
	}
	return true ;
}

