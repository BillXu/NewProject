#include "NiuNiuData.h"
#include <string>
#include "CardPoker.h"
stNiuNiuData::stNiuNiuData()
{
	nRoomID = 0;
	nBankerIdx = 0;
	nBottomBet = 0  ;
	nBankerBetTimes = 0;
	nRoomState = 0;
	nDeskFee = 0;
	memset(vPlayers,0,sizeof(vPlayers));
}

void stNiuNiuData::onStartNewGame()
{
	for ( stNNRoomInfoPayerItem& refPlayer : vPlayers )
	{
		if ( refPlayer.nUserUID == 0 )
		{
			continue; 
		}

		refPlayer.nBetTimes = 0 ;
		refPlayer.nCoin -= nDeskFee ;
		memset(refPlayer.vHoldChard,0,sizeof(refPlayer.vHoldChard)) ;
	}

	nBankerBetTimes = 0 ;
	nBankerIdx = 0 ;
}

uint8_t stNiuNiuData::getSitDownPlayerCnt()
{
	uint8_t nCnt = 0 ;
	for ( stNNRoomInfoPayerItem& refPlayer : vPlayers )
	{
		if ( refPlayer.nUserUID == 0 )
		{
			continue; 
		}
		++nCnt ;
	}
	return nCnt ;
}

uint32_t stNiuNiuData::getLeftCanBetCoin()
{
	uint16_t nPlayerTimes = 0  ;
	for ( stNNRoomInfoPayerItem& refPlayer : vPlayers )
	{
		if ( refPlayer.nUserUID == 0 )
		{
			continue; 
		}
		
		if ( refPlayer.nIdx == nBankerIdx )
		{
			continue; 
		}

		if ( refPlayer.nBetTimes == 0 )
		{
			nPlayerTimes += 5 ;
		}
		else 
		{
			nPlayerTimes += refPlayer.nBetTimes ;
		}
	}

	if ( getFinalBaseBet() * nPlayerTimes > vPlayers[nBankerIdx].nCoin )
	{
		return 0 ;
	}

	return vPlayers[nBankerIdx].nCoin - getFinalBaseBet() * nPlayerTimes ;
}

uint32_t stNiuNiuData::getFinalBaseBet()
{
	return nBankerBetTimes * nBottomBet ;
}

stNNRoomInfoPayerItem* stNiuNiuData::getPlayerByIdx(uint8_t nIdx )
{
	if ( MAX_PEERS_IN_TAXAS_ROOM <= nIdx )
	{
		return nullptr ;
	}

	if ( vPlayers[nIdx].nUserUID == 0 )
	{
		return nullptr ;
	}
	return &vPlayers[nIdx];
}

bool stNiuNiuData::isHaveNiu(uint8_t nIdx )
{
	stNNRoomInfoPayerItem* pPlayerItem = getPlayerByIdx(nIdx) ;
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
	switch ( pmsg->usMsgType )
	{
	case MSG_ROOM_ENTER_NEW_STATE:
		{
			stMsgRoomEnterNewState* pRet = (stMsgRoomEnterNewState*)pmsg ;
			nRoomState = pRet->nNewState ;
		}
		break;
	case MSG_NN_DISTRIBUTE_4_CARD:
		{
			stMsgNNDistriute4Card* pRet = (stMsgNNDistriute4Card*)pmsg ;
			stDistriuet4CardItem* pItem = (stDistriuet4CardItem*)(((char*)pmsg) + sizeof(stMsgNNDistriute4Card));
			while ( pRet->nPlayerCnt-- )
			{
				if ( pItem->nSeatIdx >= 5 )
				{
					continue; ;
				}

				memcpy(vPlayers[pItem->nSeatIdx].vHoldChard,pItem->vCardCompsitNum,sizeof(pItem->vCardCompsitNum));
			}
		}
		break;
	case MSG_NN_ROOM_INFO:
		{
			stMsgNNRoomInfo* pInfo = (stMsgNNRoomInfo*)pmsg ;
			nRoomID = pInfo->nRoomID ;
			nBankerIdx = pInfo->nBankerIdx;
			nBottomBet = pInfo->nBottomBet ;
			nBankerBetTimes = pInfo->nBankerBetTimes;
			nRoomState = pInfo->nRoomState;
			nDeskFee = pInfo->nDeskFee;

			char* pBuffer = (char*)pmsg ;
			pBuffer = pBuffer + sizeof(stMsgNNRoomInfo);
			stNNRoomInfoPayerItem* pPlayerItem = (stNNRoomInfoPayerItem*)pBuffer ;
			while ( pInfo->nPlayerCnt-- )
			{
				vPlayers[pPlayerItem->nIdx] ;
				memcpy(vPlayers + pPlayerItem->nIdx,pPlayerItem,sizeof(stNNRoomInfoPayerItem));
				++pPlayerItem ;
			}
		}
		break;
	case MSG_NN_SITDOWN:
		{
			stMsgNNSitDown* pSitDown = (stMsgNNSitDown*)pmsg ;
			memcpy(vPlayers + pSitDown->tSitDownPlayer.nIdx,&pSitDown->tSitDownPlayer,sizeof(stNNRoomInfoPayerItem));
		}
		break;
	case MSG_NN_STANDUP:
		{
			stMsgNNStandUp* pStandUp = (stMsgNNStandUp*)pmsg ;
			vPlayers[pStandUp->nPlayerIdx].nStateFlag |= eRoomPeer_WillStandUp ;
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
				vPlayers[pItem->nPlayerIdx].nCoin = pItem->nFinalCoin ;
			}

			for ( stNNRoomInfoPayerItem& pPlayer : vPlayers )
			{
				memset(pPlayer.vHoldChard,0,sizeof(pPlayer.vHoldChard)) ;
				if ( pPlayer.nUserUID == 0 )
				{
					continue;
				}

				if ( (pPlayer.nStateFlag & eRoomPeer_WillStandUp) == eRoomPeer_WillStandUp )
				{
					// do stand up 
					memset(vPlayers + pPlayer.nIdx , 0 , sizeof(stNNRoomInfoPayerItem)) ;
				}
			}
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
			vPlayers[pRet->nPlayerIdx].nBetTimes = pRet->nBetTimes ;
		}
		break;
	default:
		return false;
	}
	return true ;
}

uint8_t stNiuNiuData::getRandEmptySeatIdx()
{
	uint8_t nMaxSeatCnt = 6 ;
	uint8_t nStartIdx = rand() % nMaxSeatCnt ;
	for ( uint8_t nIdx = nStartIdx ; nIdx < nMaxSeatCnt * 2 ; ++nIdx )
	{
		uint8_t nRealIdx = nIdx % nMaxSeatCnt ;
		if ( vPlayers[nRealIdx].nUserUID == 0 )
		{
			return nRealIdx ;
		}
	}
	return 0 ;
}