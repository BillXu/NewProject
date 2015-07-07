#include "TaxasPokerData.h"
#include "TaxasMessageDefine.h"
#include "assert.h"
#include <string>
#include "cocos2d.h"
stTaxasPeerBaseData* stTaxasPokerData::getTaxasPlayerData( uint8_t nSvrIdx)
{
	if ( nSvrIdx >= MAX_PEERS_IN_TAXAS_ROOM )
		return nullptr ;
	return &vAllTaxasPlayerData[nSvrIdx] ;
}

void stTaxasPokerData::setTaxasPlayerData(stTaxasPeerBaseData* pdata )
{
	if ( pdata == nullptr || pdata->nSeatIdx >= MAX_PEERS_IN_TAXAS_ROOM )
		return ;
	memcpy_s(&vAllTaxasPlayerData[pdata->nSeatIdx],sizeof(vAllTaxasPlayerData[pdata->nSeatIdx]),pdata,sizeof(stTaxasPeerBaseData));
}

bool stTaxasPokerData::onMsg(stMsg* pmsg )
{
	switch (pmsg->usMsgType)
	{
	case MSG_TP_ROOM_BASE_INFO:
		{
			stMsgTaxasRoomInfoBase* pRet = (stMsgTaxasRoomInfoBase*)pmsg ;
			nRoomID = pRet->nRoomID;
			nMaxSeat = pRet->nMaxSeat;
			nLittleBlind = pRet->nLittleBlind;
			nMiniTakeIn = pRet->nMiniTakeIn;
			nMaxTakeIn = pRet->nMaxTakeIn;
			// running members ;
			eCurRoomState = pRet->eCurRoomState; // eeRoomState ;
			nBankerIdx = pRet->nBankerIdx;
			nLittleBlindIdx = pRet->nLittleBlindIdx;
			nBigBlindIdx = pRet->nBigBlindIdx;
			nCurWaitPlayerActionIdx = pRet->nCurWaitPlayerActionIdx;
			nCurMainBetPool = pRet->nCurMainBetPool;
			nMostBetCoinThisRound = pRet->nMostBetCoinThisRound;
			uint8_t nCCnt = TAXAS_PUBLIC_CARD ;
			while ( nCCnt-- )
			{
				vPublicCardNums[nCCnt] = pRet->vPublicCardNums[nCCnt];
			}
		}
		break;
	case MSG_TP_ROOM_VICE_POOL:
		{
			stMsgTaxasRoomInfoVicePool* pRet = (stMsgTaxasRoomInfoVicePool*)pmsg ;
			uint8_t nCnt = MAX_PEERS_IN_TAXAS_ROOM ;
			while(nCnt--)
			{
				vVicePool[nCnt] = pRet->vVicePool[nCnt];
			}
		}
		break;
	case MSG_TP_ROOM_PLAYER_DATA:
		{
			stMsgTaxasRoomInfoPlayerData* pRet = (stMsgTaxasRoomInfoPlayerData*)pmsg;
			if ( pRet->tPlayerData.nSeatIdx < MAX_PEERS_IN_TAXAS_ROOM )
			{
				memcpy_s(&vAllTaxasPlayerData[pRet->tPlayerData.nSeatIdx],sizeof(vAllTaxasPlayerData[pRet->tPlayerData.nSeatIdx]),&pRet->tPlayerData,sizeof(pRet->tPlayerData));
			}
		}
		break;
	case MSG_TP_ENTER_STATE:
		{
			stMsgTaxasRoomEnterState* pRet = (stMsgTaxasRoomEnterState*)pmsg ;
			eCurRoomState = pRet->nNewState ;
		}
		break;
	case MSG_TP_ROOM_SIT_DOWN:
		{
			stMsgTaxasRoomSitDown* pRet = (stMsgTaxasRoomSitDown*)pmsg ;
			if ( pRet->tPlayerData.nSeatIdx < MAX_PEERS_IN_TAXAS_ROOM )
			{
				memcpy_s(&vAllTaxasPlayerData[pRet->tPlayerData.nSeatIdx],sizeof(vAllTaxasPlayerData[pRet->tPlayerData.nSeatIdx]),&pRet->tPlayerData,sizeof(pRet->tPlayerData));
			}
		}
		break;
	case MSG_TP_ROOM_STAND_UP:
		{
			stMsgTaxasRoomStandUp* pRet = (stMsgTaxasRoomStandUp*)pmsg ;
			if ( pRet->nSeatIdx < MAX_PEERS_IN_TAXAS_ROOM )
			{
				memset(&vAllTaxasPlayerData[pRet->nSeatIdx],0,sizeof(vAllTaxasPlayerData[pRet->nSeatIdx]));
			}
		}
		break;
	case MSG_TP_START_ROUND:
		{
			stMsgTaxasRoomStartRound* pRet = (stMsgTaxasRoomStartRound*)pmsg ;
			nLittleBlindIdx = pRet->nLittleBlindIdx ;
			nBigBlindIdx = pRet->nBigBlindIdx ;
			nBankerIdx = pRet->nBankerIdx ;

			vAllTaxasPlayerData[nLittleBlindIdx].nBetCoinThisRound = nLittleBlind ;
			vAllTaxasPlayerData[nBigBlindIdx].nBetCoinThisRound = nLittleBlind *2;
		}
		break;
	case MSG_TP_PRIVATE_CARD:
		{
			stMsgTaxasRoomPrivateCard* pRet = (stMsgTaxasRoomPrivateCard*)pmsg ;
			stTaxasHoldCardItems* pHolder = (stTaxasHoldCardItems*)(((char*)pmsg) + sizeof(stMsgTaxasRoomPrivateCard));
			uint8_t nCnt = pRet->nPlayerCnt ;
			for ( ; nCnt > 0 ; nCnt-- )
			{
				if ( pHolder->cPlayerIdx < MAX_PEERS_IN_TAXAS_ROOM )
				{
					vAllTaxasPlayerData[pHolder->cPlayerIdx].vHoldCard[0] = pHolder->vCards[0];
					vAllTaxasPlayerData[pHolder->cPlayerIdx].vHoldCard[1] = pHolder->vCards[1];
					vAllTaxasPlayerData[pHolder->cPlayerIdx].nStateFlag = eRoomPeer_CanAct ;
				}
				++pHolder ;
			}
		}
		break;
	case MSG_TP_WAIT_PLAYER_ACT:
		{
			stMsgTaxasRoomWaitPlayerAct* pRet = (stMsgTaxasRoomWaitPlayerAct*)pmsg ;
			nCurWaitPlayerActionIdx = pRet->nActPlayerIdx ;
		}
		break;
	case MSG_TP_ROOM_ACT:
		{
			stMsgTaxasRoomAct* pret = (stMsgTaxasRoomAct*)pmsg;
			stTaxasPeerBaseData& pPlayerData = vAllTaxasPlayerData[pret->nPlayerIdx];
			if ( pPlayerData.nUserUID == 0 )
			{
				assert(0&& "act player do not sit down");
				return true ;
			}

			if ( eRoomPeerAction_Max <= pret->nPlayerAct )
			{
				assert(0&& "act value error");
				return true ;
			}

			switch ( pret->nPlayerAct )
			{
			case eRoomPeerAction_Follow:
				{
					uint8_t nBetCoin = nMostBetCoinThisRound - pPlayerData.nBetCoinThisRound ;
					pPlayerData.nBetCoinThisRound = nMostBetCoinThisRound ;
					assert(nBetCoin<=pPlayerData.nTakeInMoney && "can not follow" );
					pPlayerData.nTakeInMoney -= nBetCoin ;
				}
				break;
			case eRoomPeerAction_GiveUp:
				{
					pPlayerData.nStateFlag = eRoomPeer_GiveUp ;
					nCurMainBetPool += pPlayerData.nBetCoinThisRound ;
					pPlayerData.nBetCoinThisRound = 0 ;
				}
				break;
			case eRoomPeerAction_Add:
				{
					if ( pPlayerData.nTakeInMoney <= pret->nValue )
					{
						CCLOG("you should all in not add\n");
						pret->nValue = pPlayerData.nTakeInMoney ;
					}
					pPlayerData.nBetCoinThisRound += pret->nValue ;
					pPlayerData.nTakeInMoney -= pret->nValue ;

					if ( nMostBetCoinThisRound < pPlayerData.nBetCoinThisRound )
					{
						nMostBetCoinThisRound = pPlayerData.nBetCoinThisRound ;
					}
				}
				break;
			case eRoomPeerAction_AllIn:
				{
					pPlayerData.nBetCoinThisRound += pPlayerData.nTakeInMoney;
					if ( pPlayerData.nTakeInMoney < pret->nValue )
					{
						pPlayerData.nTakeInMoney = 0;
					}
					else
					{
						pPlayerData.nTakeInMoney -= pret->nValue;
					}

					if ( nMostBetCoinThisRound < pPlayerData.nBetCoinThisRound )
					{
						nMostBetCoinThisRound = pPlayerData.nBetCoinThisRound ;
					}
					pPlayerData.nStateFlag = eRoomPeer_AllIn ;
				}
				break;
			default:
				break;
			}
			pPlayerData.eCurAct = pret->nPlayerAct ;
		}
		break;
	case MSG_TP_ONE_BET_ROUND_RESULT:
		{
			nMostBetCoinThisRound = 0 ;
			stMsgTaxasRoomOneBetRoundResult* pRet = (stMsgTaxasRoomOneBetRoundResult*)pmsg;
			nCurMainBetPool = pRet->nCurMainPool;
			for ( uint8_t nIdx = getVicePoolCnt() , nNewIdx = 0 ; nIdx < MAX_PEERS_IN_TAXAS_ROOM && nNewIdx < pRet->nNewVicePoolCnt;++nNewIdx, ++nIdx )
			{
				if ( pRet->vNewVicePool[nNewIdx] == 0 )
				{
					break;
				}
				vVicePool[nIdx] = pRet->vNewVicePool[nNewIdx];
			}

			// refresh player data ;
			for ( uint8_t nIdx = 0 ; nIdx < MAX_PEERS_IN_TAXAS_ROOM ; ++nIdx )
			{
				if ( vAllTaxasPlayerData[nIdx].nUserUID )
				{
					vAllTaxasPlayerData[nIdx].nBetCoinThisRound = 0 ;
					vAllTaxasPlayerData[nIdx].eCurAct = eRoomPeerAction_None ;
				}
			}
		}
		break;
	case MSG_TP_PUBLIC_CARD:
		{
			stMsgTaxasRoomPublicCard* pRet = (stMsgTaxasRoomPublicCard*)pmsg ;
			if (pRet->nCardSeri == 0 )
			{
				uint8_t nCnt = 0 ;
				while ( nCnt < 3 )
				{
					vPublicCardNums[nCnt] = pRet->vCard[nCnt]; 
					++nCnt ;
				}
				
			}
			else if ( 1 == pRet->nCardSeri )
			{
				 vPublicCardNums[3] = pRet->vCard[3]; 
			}
			else if ( 2 == pRet->nCardSeri )
			{
				vPublicCardNums[4] = pRet->vCard[4]; 
			}
		}
		break;
	case MSG_TP_GAME_RESULT:
		{
			stMsgTaxasRoomGameResult* pRet = (stMsgTaxasRoomGameResult*)pmsg ;
			uint8_t nWinCnt = pRet->nWinnerCnt ;
			while ( nWinCnt >= 0  )
			{
				uint8_t ndx = pRet->vWinnerIdx[nWinCnt] ;
				if ( ndx >= MAX_PEERS_IN_TAXAS_ROOM || vAllTaxasPlayerData[ndx].nUserUID == 0 )
				{
					continue;
				}
				vAllTaxasPlayerData[ndx].nTakeInMoney += pRet->nCoinPerWinner ;
				--nWinCnt ;
			}
		}
		break;
	default:
		return false;
	}
	return true ;
}

uint8_t stTaxasPokerData::getVicePoolCnt()
{
	uint8_t nFristEmptyIdx = 0 ;
	uint8_t nCnt = 0 ;
	for (  ; nFristEmptyIdx < MAX_PEERS_IN_TAXAS_ROOM ; ++nFristEmptyIdx)
	{
		if ( vVicePool[nFristEmptyIdx] == 0 )
		{
			break;;
		}
		++nCnt ;
	}
	return nCnt ;
}

void stTaxasPokerData::resetRuntimeData()
{
	nCurWaitPlayerActionIdx = 0 ;
	nCurMainBetPool = 0 ;
	nMostBetCoinThisRound = 0 ;
	memset(vPublicCardNums,0,sizeof(vPublicCardNums));
	memset(vVicePool,0,sizeof(vVicePool));

	for ( uint8_t nIdx = 0 ; nIdx < MAX_PEERS_IN_TAXAS_ROOM ; ++nIdx )
	{
		stTaxasPeerBaseData& data = vAllTaxasPlayerData[nIdx] ;
		data.eCurAct = eRoomPeerAction_None ;
		data.nBetCoinThisRound = 0 ;
		data.nStateFlag = eRoomPeer_WaitNextGame ;
		memset(data.vHoldCard,0,sizeof(data.vHoldCard));
	}
}

uint32_t stTaxasPokerData::getPlayerAddCoinLowLimit( uint8_t nPlayerSvrIdx )
{
	if ( nMostBetCoinThisRound == 0 )
	{
		return nLittleBlind*2 ;
	}

	uint32_t nbet = nMostBetCoinThisRound - vAllTaxasPlayerData[nPlayerSvrIdx].nBetCoinThisRound ;
	return ( nbet + nLittleBlind*2 );
}

uint64_t stTaxasPokerData::getPlayerAddCoinUpLimit( uint8_t nPlayerSvrIdx)
{
	uint64_t nFirstBig = 0, nSecondBig = 0  ;
	for ( uint32_t nIdx = 0 ; nIdx < MAX_PEERS_IN_TAXAS_ROOM ; ++nIdx )
	{
		stTaxasPeerBaseData& pdata = vAllTaxasPlayerData[nIdx] ;
		if (pdata.nUserUID == 0 || ((pdata.nStateFlag & eRoomPeer_CanAct) != eRoomPeer_CanAct ) )
		{
			continue;
		}

		if (pdata.nTakeInMoney >= nFirstBig )
		{
			nSecondBig = nFirstBig ;
			nFirstBig = pdata.nTakeInMoney ;
		}
		else
		{
			if ( pdata.nTakeInMoney > nSecondBig )
			{
				nSecondBig = pdata.nTakeInMoney; 
			}
		}
	}
	return nSecondBig ;
}