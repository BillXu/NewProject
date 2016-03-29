#include "TaxasPokerData.h"
#include "TaxasMessageDefine.h"
#include "assert.h"
#include <string>
#include "RobotControlTaxas.h"
#include "json/json.h"
//#include "cocos2d.h"
void CTaxasPokerData::setTaxasPlayerData(stTaxasPeerBaseData* pdata )
{
	if ( pdata == nullptr || pdata->nSeatIdx >= MAX_PEERS_IN_TAXAS_ROOM )
		return ;
	stTaxasPlayer* pPlayer = (stTaxasPlayer*)getPlayerByIdx(pdata->nSeatIdx) ;
	if ( pPlayer == nullptr )
	{
		pPlayer = new stTaxasPlayer ;
	}
	pPlayer->reset() ;
	pPlayer->nBetCoinThisRound = (uint32_t)pdata->nBetCoinThisRound ;
	pPlayer->nCoin = (uint32_t)pdata->nTakeInMoney ;
	pPlayer->nIdx = pdata->nSeatIdx ;
	pPlayer->nCurAct = pdata->eCurAct ;
	pPlayer->nStateFlag = pdata->nStateFlag ;
	pPlayer->nUserUID = pdata->nUserUID ;
	pPlayer->vHoldCard;
	memcpy(pPlayer->vHoldCard,pdata->vHoldCard,sizeof(pPlayer->vHoldCard));
}

bool CTaxasPokerData::onMsg(stMsg* pmsg )
{
	if ( CSitableRoomData::onMsg(pmsg) )
	{
		return true ;
	}

	switch (pmsg->usMsgType)
	{
	case MSG_ROOM_INFO:
		{
			stMsgRoomInfo* pInfo = (stMsgRoomInfo*)pmsg ;
			char* pBuffer = (char*)pmsg ;
			pBuffer = pBuffer + sizeof(stMsgRoomInfo);

			Json::Reader rt ;
			Json::Value jCont ;
			rt.parse(pBuffer,pBuffer + pInfo->nJsonLen,jCont) ;
			setBaseInfo(pInfo->nRoomID,5,pInfo->nDeskFee,pInfo->eCurRoomState);


			nLittleBlind = jCont["litBlind"].asUInt();
			nMiniTakeIn = jCont["minTakIn"].asUInt();
			nMaxTakeIn = jCont["maxTakIn"].asUInt();
			// running members ;
			nCurWaitPlayerActionIdx = jCont["curActIdx"].asInt();
			nCurMainBetPool = jCont["curPool"].asUInt();
			nMostBetCoinThisRound = jCont["mostBet"].asUInt();

			Json::Value vPub = jCont["pubCards"];
			memset(vPublicCardNums,0,sizeof(vPublicCardNums));
			for ( uint8_t nIdx = 0 ; nIdx < (uint8_t)vPub.size(); ++nIdx )
			{
				vPublicCardNums[nIdx] = vPub[nIdx].asUInt();
			}
		}
		break;
	case MSG_TP_ROOM_PLAYER_DATA:
		{
			stMsgTaxasRoomInfoPlayerData* pRet = (stMsgTaxasRoomInfoPlayerData*)pmsg;
			if ( pRet->nSeatIdx < MAX_PEERS_IN_TAXAS_ROOM )
			{
				stTaxasPlayer* pPlayer = (stTaxasPlayer*)getPlayerByIdx(pRet->nSeatIdx) ;
				if ( pPlayer == nullptr )
				{
					pPlayer = new stTaxasPlayer ;
					m_vSitDownPlayer[pRet->nSeatIdx] = pPlayer ;
				}
				pPlayer->reset() ;
				pPlayer->nBetCoinThisRound = pRet->nBetCoinThisRound ;
				pPlayer->nCoin = pRet->nTakeInMoney ;
				pPlayer->nIdx = pRet->nSeatIdx ;
				pPlayer->nCurAct = pRet->eCurAct ;
				pPlayer->nStateFlag = pRet->nStateFlag ;
				pPlayer->nUserUID = pRet->nUserUID ;
				memcpy(pPlayer->vHoldCard,pRet->vHoldCard,sizeof(pPlayer->vHoldCard));
			}
		}
		break;
	case MSG_TP_UPDATE_PLAYER_STATE:
		{
			stMsgTaxasRoomUpdatePlayerState* pRet = (stMsgTaxasRoomUpdatePlayerState*)pmsg ;
			if ( pRet->nSeatIdx < MAX_PEERS_IN_TAXAS_ROOM )
			{
				stTaxasPlayer* pPlayer = (stTaxasPlayer*)getPlayerByIdx(pRet->nSeatIdx) ;
				if ( pPlayer )
				{
					pPlayer->nStateFlag = pRet->nStateFlag ;
					pPlayer->nCoin = pRet->nTakeInCoin ;
				}
			}
		}
		break;
	case MSG_TP_START_ROUND:
		{
			onGameBegin() ;
			stMsgTaxasRoomStartRound* pRet = (stMsgTaxasRoomStartRound*)pmsg ;
			auto pPlayer = (stTaxasPlayer*)getPlayerByIdx(pRet->nLittleBlindIdx) ;
			if ( pPlayer )
			{
				pPlayer->betCoin(nLittleBlind);
			}

			pPlayer = (stTaxasPlayer*)getPlayerByIdx(pRet->nBankerIdx) ;
			if ( pPlayer )
			{
				pPlayer->betCoin(nLittleBlind*2);
			}
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
					auto pPlayer = (stTaxasPlayer*)getPlayerByIdx(pHolder->cPlayerIdx) ;
					if ( pPlayer )
					{
						pPlayer->vHoldCard[0] = pHolder->vCards[0];
						pPlayer->vHoldCard[1] = pHolder->vCards[1];
					}
				}
				++pHolder ;
			}
		}
		break;
	case MSG_TP_WAIT_PLAYER_ACT:
		{
			stMsgTaxasRoomWaitPlayerAct* pRet = (stMsgTaxasRoomWaitPlayerAct*)pmsg ;
			nCurWaitPlayerActionIdx = pRet->nActPlayerIdx ;
			if ( nCurWaitPlayerActionIdx == m_pRobot->getSeatIdx() )
			{
				m_pRobot->informRobotAction(0) ;
			}
			printf("wait act idx = %d\n",nCurWaitPlayerActionIdx);
		}
		break;
	case MSG_TP_ROOM_ACT:
		{
			stMsgTaxasRoomAct* pret = (stMsgTaxasRoomAct*)pmsg;
			auto pPlayer = (stTaxasPlayer*)getPlayerByIdx(pret->nPlayerIdx) ;
			if ( pPlayer == nullptr || pPlayer->isValid() == false )
			{
				printf("act player do not sit down\n");
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
					uint8_t nBetCoin = nMostBetCoinThisRound - pPlayer->nBetCoinThisRound ;
					if ( nBetCoin > pPlayer->nCoin )
					{
						nBetCoin = pPlayer->nCoin ;
						printf("follow , but you don't have coin uid = %d\n",pPlayer->nUserUID ) ;
					}
					pPlayer->betCoin(nBetCoin);
					
					printf("data here follow bet coin this round = %d\n",pPlayer->nBetCoinThisRound);
				}
				break;
			case eRoomPeerAction_GiveUp:
				{
					pPlayer->nStateFlag = eRoomPeer_GiveUp ;
					nCurMainBetPool += pPlayer->nBetCoinThisRound ;
				}
				break;
			case eRoomPeerAction_Add:
				{
					if ( pPlayer->nCoin <= pret->nValue )
					{
						printf("you should all in not add\n");
						pret->nValue = pPlayer->nCoin;
					}
					pPlayer->betCoin(pret->nValue);

					if ( nMostBetCoinThisRound < pPlayer->nBetCoinThisRound )
					{
						nMostBetCoinThisRound = pPlayer->nBetCoinThisRound;
						printf("monstbet coin update by add = %I64d\n",nMostBetCoinThisRound);
					}
				}
				break;
			case eRoomPeerAction_AllIn:
				{
					if ( pPlayer->nCoin <= pret->nValue )
					{
						pret->nValue = pPlayer->nCoin;
					}
					pPlayer->betCoin(pret->nValue) ;

					if ( nMostBetCoinThisRound < pPlayer->nBetCoinThisRound )
					{
						nMostBetCoinThisRound = pPlayer->nBetCoinThisRound;
					}
					pPlayer->nStateFlag = eRoomPeer_AllIn ;
				}
				break;
			default:
				break;
			}
			pPlayer->nCurAct = pret->nPlayerAct ;
			printf("recieved player do act = %d , value = %I64d, final coin = %d\n",pret->nPlayerAct,pret->nValue,pPlayer->nCoin);
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
			resetBetRoundState() ;
		}
		break;
	case MSG_TP_GAME_RESULT:
		{
			stMsgTaxasRoomGameResult* pRet = (stMsgTaxasRoomGameResult*)pmsg ;
			printf( "result pool = %d \n",pRet->nPoolIdx );
			for ( uint8_t nIdx = 0 ; nIdx < pRet->nWinnerCnt ; ++nIdx )
			{
				uint8_t nWinIdx = pRet->vWinnerIdx[nIdx] ;
				auto pPlayer = (stTaxasPlayer*)getPlayerByIdx(nWinIdx) ;
				if ( pPlayer == nullptr || pPlayer->isValid() == false )
				{
					continue;
				}
				pPlayer->nCoin += pRet->nCoinPerWinner ;
				printf("winer idx = %d , winCoin = %d, final Coin = %d\n",nWinIdx,pRet->nCoinPerWinner,pPlayer->nCoin);
			}
		}
		break;
	default:
		return false;
	}
	return true ;
}

void CTaxasPokerData::onGameBegin()
{
	CSitableRoomData::onGameBegin() ;
	nCurWaitPlayerActionIdx = 0 ;
	nCurMainBetPool = 0 ;
	nMostBetCoinThisRound = 0 ;
	memset(vPublicCardNums,0,sizeof(vPublicCardNums));
}

uint32_t CTaxasPokerData::getPlayerAddCoinLowLimit( uint8_t nPlayerSvrIdx )
{
	if ( nMostBetCoinThisRound == 0 )
	{
		return nLittleBlind*2 ;
	}

	auto pPlayer = (stTaxasPlayer*)getPlayerByIdx(nPlayerSvrIdx) ;
	if ( pPlayer == nullptr || pPlayer->isValid() == false )
	{
		return nLittleBlind*2 ;;
	}

	uint32_t nbet = nMostBetCoinThisRound - pPlayer->nBetCoinThisRound ;
	return ( nbet + nLittleBlind*2 );
}

uint64_t CTaxasPokerData::getPlayerAddCoinUpLimit( uint8_t nPlayerSvrIdx)
{
	uint64_t nFirstBig = 0, nSecondBig = 0  ;
	for ( uint32_t nIdx = 0 ; nIdx < MAX_PEERS_IN_TAXAS_ROOM ; ++nIdx )
	{
		auto pPlayer = (stTaxasPlayer*)getPlayerByIdx(nPlayerSvrIdx) ;
		if ( pPlayer == nullptr || pPlayer->isValid() == false || pPlayer->isHaveState(eRoomPeer_CanAct) == false )
		{
			continue;
		}

		if ( pPlayer->nCoin >= nFirstBig )
		{
			nSecondBig = nFirstBig ;
			nFirstBig = pPlayer->nCoin ;
		}
		else
		{
			if ( pPlayer->nCoin > nSecondBig )
			{
				nSecondBig = pPlayer->nCoin; 
			}
		}
	}
	return nSecondBig ;
}

void CTaxasPokerData::resetBetRoundState()
{
	nMostBetCoinThisRound = 0 ;
	// refresh player data ;
	printf("resetBetRoundState\n");
	for ( uint8_t nIdx = 0 ; nIdx < MAX_PEERS_IN_TAXAS_ROOM ; ++nIdx )
	{
		auto pPlayer = (stTaxasPlayer*)getPlayerByIdx(nIdx) ;
		if ( pPlayer == nullptr || pPlayer->isValid() == false || pPlayer->isHaveState(eRoomPeer_StayThisRound) == false )
		{
			continue;
		}

		pPlayer->nBetCoinThisRound = 0 ;
		pPlayer->nCurAct = eRoomPeerAction_None ;
	}
}

CRobotControl* CTaxasPokerData::doCreateRobotControl()
{
	return new CRobotControlTaxas ;
}