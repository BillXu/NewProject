#include "GoldenData.h"
#include "RobotControlGolden.h"
#include "json/json.h"
#include <cassert>
bool CGoldenData::onMsg(stMsg* pmsg )
{
	if ( CSitableRoomData::onMsg(pmsg) )
	{
		return true ;
	}

	switch ( pmsg->usMsgType )
	{
	case MSG_ROOM_INFO:
		{
			stMsgRoomInfo* pInfo = (stMsgRoomInfo*)pmsg ;
			char* pBuffer = (char*)pmsg ;
			pBuffer = pBuffer + sizeof(stMsgRoomInfo);

			Json::Reader rt ;
			Json::Value jCont ;
			rt.parse(pBuffer,pBuffer + pInfo->nJsonLen,jCont) ;

			nBankerIdx = jCont["bankIdx"].asInt();
			nRound = jCont["betRound"].asInt();
			nBaseBet = jCont["baseBet"].asInt();
			nCurBottomBet = jCont["curBet"].asInt();
			setBaseInfo(pInfo->nRoomID,5,pInfo->nDeskFee,pInfo->eCurRoomState,pInfo->nSubIdx);
		}
		break;
	case MSG_GOLDEN_ROOM_PLAYERS:
		{
			stMsgGoldenRoomPlayers* pInfo = (stMsgGoldenRoomPlayers*)pmsg ;
			char* pBuffer = (char*)pmsg ;
			pBuffer = pBuffer + sizeof(stMsgGoldenRoomPlayers);
			stGoldenRoomInfoPayerItem* pPlayerItem = (stGoldenRoomInfoPayerItem*)pBuffer ;
			while ( pInfo->nPlayerCnt-- )
			{
				stGoldenPlayer* pPlayer = (stGoldenPlayer*)getPlayerByIdx(pPlayerItem->nIdx) ;
				if ( pPlayer == nullptr )
				{
					pPlayer = new stGoldenPlayer ;
					m_vSitDownPlayer[pPlayerItem->nIdx] = pPlayer;
				}
				pPlayer->reset();
				pPlayer->nCoin = pPlayerItem->nCoin ;
				pPlayer->nIdx = pPlayerItem->nIdx ;
				pPlayer->nUserUID = pPlayerItem->nUserUID ;
				pPlayer->nStateFlag = pPlayerItem->nStateFlag ;
				memcpy(pPlayer->vHoldChard,pPlayerItem->vHoldChard,sizeof(pPlayer->vHoldChard));
				++pPlayerItem ;
			}
		}
		break;
	case MSG_GOLDEN_ROOM_DISTRIBUTE:
		{
			onGameBegin();
			stMsgGoldenDistribute* pRet = (stMsgGoldenDistribute*)pmsg ;
			nBankerIdx = pRet->nBankIdx ;
			stGoldenHoldPeerCard* pItem = (stGoldenHoldPeerCard*)(((char*)pmsg) + sizeof(stMsgGoldenDistribute));
			while ( pRet->nCnt )
			{
				if ( pItem->nIdx >= 5 )
				{
					continue; ;
				}

				stGoldenPlayer* pPlayerItem = (stGoldenPlayer*)getPlayerByIdx(pItem->nIdx) ;
				if ( pPlayerItem )
				{
					memcpy(pPlayerItem->vHoldChard,pItem->vCard,sizeof(pItem->vCard));
					printf("distrubute card idx = %u , c1 = %u, c2 = %u , c3 = %u\n",pItem->nIdx,pPlayerItem->vHoldChard[0],pPlayerItem->vHoldChard[1],pPlayerItem->vHoldChard[2]) ;
				}
				++pItem;
				pRet->nCnt-- ;
			}
		}
		break ;
	case MSG_GOLDEN_WAIT_PLAYER_ACT:
		{
			stMsgGoldenRoomWaitPlayerAct* pRet = (stMsgGoldenRoomWaitPlayerAct*)pmsg ;
			if ( pRet->nActPlayerIdx == getRobotControl()->getSeatIdx() )
			{
				getRobotControl()->informRobotAction(0) ;
			}
			else
			{
				getRobotControl()->informRobotAction(1) ;
			}
		}
		break ;
	case MSG_GOLDEN_ROOM_ACT:
		{
			stMsgGoldenRoomAct* pRet = (stMsgGoldenRoomAct*)pmsg ;
			stGoldenPlayer* pPlayerItem = (stGoldenPlayer*)getPlayerByIdx(pRet->nPlayerIdx) ;

			if ( nPreActIdx < pRet->nPlayerIdx )
			{
				if ( nPreActIdx < nBankerIdx && nBankerIdx <= pRet->nPlayerIdx )
				{
					++nRound ;
				}
			}
			else if ( nPreActIdx > pRet->nPlayerIdx )
			{
				if ( nBankerIdx > nPreActIdx || nBankerIdx <= pRet->nPlayerIdx )
				{
					++nRound ;
				}
			}

			nPreActIdx = pRet->nPlayerIdx ;
			assert(pPlayerItem != nullptr);
			switch ( pRet->nPlayerAct )
			{
			case eRoomPeerAction_Add:
				{
					nCurBottomBet += pRet->nValue ;
					uint32_t nbetCoin = nCurBottomBet ;
					if ( pPlayerItem->isHaveState(eRoomPeer_Looked) )
					{
						nbetCoin *= 2 ;
					}
					pPlayerItem->nCoin -= nbetCoin ; 
				}
				break; 
			case eRoomPeerAction_GiveUp:
				{
					pPlayerItem->nStateFlag = eRoomPeer_GiveUp ;
				}
				break ;
			case eRoomPeerAction_ViewCard:
				{
					pPlayerItem->nStateFlag = eRoomPeer_Looked ;
				}
				break ;
			case eRoomPeerAction_Follow:
				{
					uint32_t nbetCoin = nCurBottomBet ;
					if ( pPlayerItem->isHaveState(eRoomPeer_Looked) )
					{
						nbetCoin *= 2 ;
					}
					pPlayerItem->nCoin -= nbetCoin ; 
				}
				break ;
			default:
				return false ;
			}
			break ;
		}
		break ;
	case MSG_GOLDEN_ROOM_PK:
		{
			stMsgGoldenRoomPK* pRet = (stMsgGoldenRoomPK*)pmsg ;
			stGoldenPlayer* pPlayerItem = (stGoldenPlayer*)getPlayerByIdx(pRet->nActPlayerIdx) ;
			stGoldenPlayer* pTargetPlayer = (stGoldenPlayer*)getPlayerByIdx(pRet->nTargetIdx) ;
			if ( pRet->bWin )
			{
				pTargetPlayer->nStateFlag = eRoomPeer_PK_Failed ;
				if ( pTargetPlayer->bDelayLeave && pTargetPlayer->nIdx == getRobotControl()->getSeatIdx() )
				{
					printf("pk failed delay leave \n") ;
					float nRate = (float)rand() / float(RAND_MAX);
					nRate = 1.0f + nRate * 2.0f ;
					getRobotControl()->fireDelayAction(eRoomPeerAction_LeaveRoom,nRate,nullptr);
				}
			}
			else 
			{
				pPlayerItem->nStateFlag = eRoomPeer_PK_Failed ;

				if ( pPlayerItem->bDelayLeave && pPlayerItem->nIdx == getRobotControl()->getSeatIdx() )
				{
					printf("pk failed delay leave \n") ;
					float nRate = (float)rand() / float(RAND_MAX);
					nRate = 2.5f + nRate * 5.0f ;
					getRobotControl()->fireDelayAction(eRoomPeerAction_LeaveRoom,nRate,nullptr);
				}
			}

			uint32_t nbetCoin = nCurBottomBet ;
			if ( pPlayerItem->isHaveState(eRoomPeer_Looked) )
			{
				nbetCoin *= 2 ;
			}

			nbetCoin *= 2 ;
			pPlayerItem->nCoin -= nbetCoin ;
		}
		break ;
	case MSG_GOLDEN_ROOM_RESULT:
		{
			stMsgGoldenResult* pRet = (stMsgGoldenResult*)pmsg ;
			stGoldenPlayer* pPlayerItem = (stGoldenPlayer*)getPlayerByIdx(pRet->cWinnerIdx) ;
			pPlayerItem->nCoin = pRet->nFinalCoin ; 
		}
		break;
	default:
		return false ;
	}
	return true ;
}

stSitableRoomPlayer* CGoldenData::doCreateSitDownPlayer()
{
	return new stGoldenPlayer ;
}

uint8_t CGoldenData::getRoomType()
{
	return eRoom_Golden ;
}

uint8_t CGoldenData::getTargetSvrPort()
{
	return ID_MSG_PORT_GOLDEN ;
}

CRobotControl* CGoldenData::doCreateRobotControl()
{
	return new CRobotControlGolden ;
}

void CGoldenData::onGameBegin()
{
	CSitableRoomData::onGameBegin();
	nRound = 0 ;
	nCurBottomBet = nBaseBet ;
	nPreActIdx = 0 ;
}