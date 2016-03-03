#include "TaxasPokerData.h"
#include "TaxasMessageDefine.h"
#include "assert.h"
#include <string>
//#include "cocos2d.h"
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
			if ( pRet->nSeatIdx < MAX_PEERS_IN_TAXAS_ROOM )
			{
				memcpy_s(&vAllTaxasPlayerData[pRet->nSeatIdx],sizeof(vAllTaxasPlayerData[pRet->nSeatIdx]),&pRet->tPlayerData,sizeof(pRet->tPlayerData));
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
	case MSG_TP_UPDATE_PLAYER_STATE:
		{
			stMsgTaxasRoomUpdatePlayerState* pRet = (stMsgTaxasRoomUpdatePlayerState*)pmsg ;
			if ( pRet->nSeatIdx < MAX_PEERS_IN_TAXAS_ROOM )
			{
				vAllTaxasPlayerData[pRet->nSeatIdx].nStateFlag = pRet->nStateFlag ;
				vAllTaxasPlayerData[pRet->nSeatIdx].nTakeInMoney = pRet->nTakeInCoin ;
				if (vAllTaxasPlayerData[pRet->nSeatIdx].nTakeInMoney <= nLittleBlind * 2 )
				{
					{
						vAllTaxasPlayerData[pRet->nSeatIdx].nStateFlag = eRoomPeer_LackOfCoin ;
						printf("update player state player lack of coin idx = %d, name = %s\n",vAllTaxasPlayerData[pRet->nSeatIdx].nSeatIdx,vAllTaxasPlayerData[pRet->nSeatIdx].cName);
					}
				}
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
			vAllTaxasPlayerData[nLittleBlindIdx].nTakeInMoney -= nLittleBlind ;
			vAllTaxasPlayerData[nBigBlindIdx].nBetCoinThisRound = nLittleBlind *2;
			vAllTaxasPlayerData[nBigBlindIdx].nTakeInMoney -= nLittleBlind *2;
			nMostBetCoinThisRound = nLittleBlind * 2 ;
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
			printf("wait act idx = %d\n",nCurWaitPlayerActionIdx);
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
					printf("data here follow bet coin this round = %I64d\n",pPlayerData.nBetCoinThisRound);
					pPlayerData.nTakeInMoney -= nBetCoin ;
				}
				break;
			case eRoomPeerAction_GiveUp:
				{
					pPlayerData.nStateFlag = eRoomPeer_GiveUp ;
					nCurMainBetPool += pPlayerData.nBetCoinThisRound ;

					if (pPlayerData.nTakeInMoney <= nLittleBlind * 2 )
					{
						if ((pPlayerData.nStateFlag & eRoomPeer_WithdrawingCoin) != eRoomPeer_WithdrawingCoin )
						{
							pPlayerData.nStateFlag |= eRoomPeer_LackOfCoin ;
						}
					}
				}
				break;
			case eRoomPeerAction_Add:
				{
					if ( pPlayerData.nTakeInMoney <= pret->nValue )
					{
						printf("you should all in not add\n");
						pret->nValue = pPlayerData.nTakeInMoney ;
					}
					pPlayerData.nBetCoinThisRound += pret->nValue ;
					pPlayerData.nTakeInMoney -= pret->nValue ;

					if ( nMostBetCoinThisRound < pPlayerData.nBetCoinThisRound )
					{
						nMostBetCoinThisRound = pPlayerData.nBetCoinThisRound ;
						printf("monstbet coin update by add = %I64d\n",nMostBetCoinThisRound);
					}
				}
				break;
			case eRoomPeerAction_AllIn:
				{
					pPlayerData.nBetCoinThisRound += pret->nValue;
					if ( pPlayerData.nTakeInMoney <= pret->nValue )
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
			printf("recieved player do act = %d , value = %I64d, final coin = %I64d\n",pret->nPlayerAct,pret->nValue,pPlayerData.nTakeInMoney);
		}
		break;
	case MSG_TP_ONE_BET_ROUND_RESULT:
		{
			nMostBetCoinThisRound = 0 ;
			printf( "one round end result:\n" );
			stMsgTaxasRoomOneBetRoundResult* pRet = (stMsgTaxasRoomOneBetRoundResult*)pmsg;
			nCurMainBetPool = pRet->nCurMainPool;
			printf("nmain pool = %I64d newViceCnt = %d\n",pRet->nCurMainPool,pRet->nNewVicePoolCnt);
			for ( uint8_t nIdx = getVicePoolCnt() , nNewIdx = 0 ; nIdx < MAX_PEERS_IN_TAXAS_ROOM && nNewIdx < pRet->nNewVicePoolCnt;++nNewIdx, ++nIdx )
			{
				if ( pRet->vNewVicePool[nNewIdx] == 0 )
				{
					break;
				}
				vVicePool[nIdx] = pRet->vNewVicePool[nNewIdx];
				printf("new vice pool idx = %d, pool coin = %I64d\n",nIdx,pRet->vNewVicePool[nNewIdx] );
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
				 vPublicCardNums[3] = pRet->vCard[0]; 
			}
			else if ( 2 == pRet->nCardSeri )
			{
				vPublicCardNums[4] = pRet->vCard[0]; 
			}
		}
		break;
	case MSG_TP_GAME_RESULT:
		{
			stMsgTaxasRoomGameResult* pRet = (stMsgTaxasRoomGameResult*)pmsg ;
			printf( "result pool = %d \n",pRet->nPoolIdx );
			for ( uint8_t nIdx = 0 ; nIdx < pRet->nWinnerCnt ; ++nIdx )
			{
				uint8_t nWinIdx = pRet->vWinnerIdx[nIdx] ;
				if ( nWinIdx >= MAX_PEERS_IN_TAXAS_ROOM || vAllTaxasPlayerData[nWinIdx].nUserUID == 0 )
				{
					continue;
				}
				vAllTaxasPlayerData[nWinIdx].nTakeInMoney += pRet->nCoinPerWinner ;
				printf("winer idx = %d , winCoin = %I64d, final Coin = %d\n",nWinIdx,pRet->nCoinPerWinner,vAllTaxasPlayerData[nWinIdx].nTakeInMoney);
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
		if ( data.nUserUID == 0 )
		{
			continue;
		}
		data.eCurAct = eRoomPeerAction_None ;
		data.nBetCoinThisRound = 0 ;
		memset(data.vHoldCard,0,sizeof(data.vHoldCard));
		if (data.nTakeInMoney <= nLittleBlind * 2 )
		{
			//if ((data.nStateFlag & eRoomPeer_WithdrawingCoin) != eRoomPeer_WithdrawingCoin )
			{
				data.nStateFlag = eRoomPeer_LackOfCoin ;
				printf("player lack of coin idx = %d, name = %s\n",data.nSeatIdx,data.cName);
			}
		}
		else
		{
			data.nStateFlag = eRoomPeer_WaitNextGame ;
		}
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

void stTaxasPokerData::resetBetRoundState()
{
	nMostBetCoinThisRound = 0 ;
	// refresh player data ;
	printf("resetBetRoundState\n");
	for ( uint8_t nIdx = 0 ; nIdx < MAX_PEERS_IN_TAXAS_ROOM ; ++nIdx )
	{
		if ( vAllTaxasPlayerData[nIdx].nUserUID )
		{
			vAllTaxasPlayerData[nIdx].nBetCoinThisRound = 0 ;
			vAllTaxasPlayerData[nIdx].eCurAct = eRoomPeerAction_None ;
		}
	}
}

uint8_t stTaxasPokerData::getPlayerCnt()
{
	uint8_t nCnt = 0 ;
	for ( uint8_t nIdx = 0 ; nIdx < MAX_PEERS_IN_TAXAS_ROOM ; ++nIdx )
	{
		if ( vAllTaxasPlayerData[nIdx].nUserUID )
		{
			++nCnt;
		}
	}
	return nCnt ;
}