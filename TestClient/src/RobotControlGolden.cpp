#include "RobotControlGolden.h"
#include "GoldenData.h"
CRobotControlGolden::CRobotControlGolden()
{

}

CRobotControlGolden::~CRobotControlGolden()
{

}

void CRobotControlGolden::doDelayAction(uint8_t nActType,void* pUserData )
{
	if ( nActType == eRoomPeerAction_GiveUp && getTempHalo() > 70 )
	{
		nActType = eRoomPeerAction_Follow ;
	}

	auto pSelf = (stGoldenPlayer*)getRoomData()->getPlayerByIdx(getSeatIdx()) ;
	CGoldenData* pdata = (CGoldenData*)getRoomData();
	if ( pSelf == nullptr )
	{
		printf("do act self ptr is null \n") ;
		return ;
	}

	stMsgGoldenPlayerAct msgAct ;
	msgAct.nRoomID = getRoomData()->getRoomID() ;
	msgAct.nSubRoomIdx = getRoomData()->getSubRoomIdx() ;
	msgAct.nValue = 0 ;
	msgAct.nPlayerAct = nActType ;

	uint8_t nNeedBetCoin = 0 ;
	switch (nActType)
	{
	case eRoomPeerAction_LeaveRoom:
		{
			leaveRoom() ;
			printf("do leave room \n");
		}
		break;
	case eRoomPeerAction_Ready:
		{

		}
		break;
	case eRoomPeerAction_Follow:
		{
			nNeedBetCoin = pdata->getCurBet() ;
		}
		break ;
	case eRoomPeerAction_Add:
		{
			if ( pdata->getCurBet() < 10*pdata->getBaseBet() )
			{
				msgAct.nValue = 10 ;
				nNeedBetCoin = pdata->getCurBet() + msgAct.nValue ;
				if ( pSelf->isHaveState(eRoomPeer_Looked) )
				{
					nNeedBetCoin *= 2 ;
				}
			}
			else
			{
				nActType = eRoomPeerAction_Follow ;
				nNeedBetCoin = pdata->getCurBet() ;
			}
		}
		break ;
	case eRoomPeerAction_GiveUp:
		{

		}
		break ;
	case eRoomPeerAction_PK:
		{
			stMsgGoldenPlayerPK msgPK ;
			msgPK.nRoomID = getRoomData()->getRoomID() ;
			msgPK.nSubRoomIdx = getRoomData()->getSubRoomIdx() ;
			msgPK.nPkTargetIdx = randPKTargetIdx();
			sendMsg(&msgPK,sizeof(msgPK)) ;
			printf("uid = %u do pk target = %d \n",getUserUID(),msgPK.nPkTargetIdx) ;
		}
		break ;
	case eRoomPeerAction_ViewCard:
		{

		}
		break ;
	default:
		printf("unknown act type = %u \n",nActType) ;
		return;
	}

	if ( nNeedBetCoin > pSelf->nCoin && nNeedBetCoin != 0 )
	{
		stMsgGoldenPlayerPK msgPK ;
		msgPK.nRoomID = getRoomData()->getRoomID() ;
		msgPK.nSubRoomIdx = getRoomData()->getSubRoomIdx() ;
		msgPK.nPkTargetIdx = randPKTargetIdx();
		sendMsg(&msgPK,sizeof(msgPK)) ;
		printf("uid = %u lact of coin , so pk target = %d \n",getUserUID(),msgPK.nPkTargetIdx) ;
		return ;
	}
	sendMsg(&msgAct,sizeof(msgAct)) ;

	if ( nActType == eRoomPeerAction_GiveUp && pSelf->bDelayLeave )
	{
		printf("delay leave , after give up \n") ;
		float nRate = (float)rand() / float(RAND_MAX);
		nRate = 1.5f + nRate * 3.0f ;
		fireDelayAction(eRoomPeerAction_LeaveRoom,nRate,nullptr);
	}
	printf("uid = %u do act = %u , value = %d \n",getUserUID(),nActType,msgAct.nValue) ;
}

void CRobotControlGolden::informRobotAction(uint8_t nActType)
{
	if ( isSelfSitDown() == false )
	{
		return ;
	}

	auto pSelf = (stGoldenPlayer*)getRoomData()->getPlayerByIdx(getSeatIdx()) ;
	if ( !pSelf )
	{
		printf("self player is null why ? \n") ;
		return ;
	}

	if ( pSelf->isHaveState(eRoomPeer_CanAct) == false )
	{
		printf("self have not can act flag \n");
		return ;
	}

	if ( nCardtype == CPeerCard::ePeerCard_Max )
	{
		CPeerCard tcard ;
		tcard.SetPeerCardByNumber(pSelf->vHoldChard[0],pSelf->vHoldChard[1],pSelf->vHoldChard[2]);
		nCardtype = tcard.GetType() ;
	}

	if ( nActType != 0 )
	{
		if ( canGiveUp() )
		{
			float nRate = (float)rand() / float(RAND_MAX);
			nRate = 0.5f + nRate * 1.0f ;
			fireDelayAction(eRoomPeerAction_GiveUp,nRate,nullptr);
			return ;
		}

		if ( canViewCard() )
		{
			float nRate = (float)rand() / float(RAND_MAX);
			nRate = 0.5f + nRate * 1.0f ;
			fireDelayAction(eRoomPeerAction_ViewCard,nRate,nullptr);
			return ;
		}
	}

	if ( canAddBet() )
	{
		float nRate = (float)rand() / float(RAND_MAX);
		nRate = 1.0f + nRate * 4.0f ;
		fireDelayAction(eRoomPeerAction_Add,nRate,nullptr);
		return ;
	}

	if ( canPKCard() )
	{
		float nRate = (float)rand() / float(RAND_MAX);
		nRate = 1.5f + nRate * 3.0f ;
		fireDelayAction(eRoomPeerAction_PK,nRate,nullptr);
		return ;
	}

	float nRate = (float)rand() / float(RAND_MAX);
	nRate = 0.5f + nRate * 1.0f ;

	auto tt = (CGoldenData*)getRoomData() ;
	if ( tt->getRound() > 15 )
	{
		nRate = 0.5f + nRate * 3.0f ;
	}
	fireDelayAction(eRoomPeerAction_Follow,nRate,nullptr);
}

uint32_t CRobotControlGolden::getTakeInCoinWhenSitDown()
{
	return 0 ;
}

bool CRobotControlGolden::canViewCard()
{
	auto pSelf = (stGoldenPlayer*)getRoomData()->getPlayerByIdx(getSeatIdx()) ;
	if ( !pSelf )
	{
		printf("self player is null why ? \n") ;
		return false;
	}

	if ( pSelf->isHaveState(eRoomPeer_Looked) )
	{
		return false ;
	}

	CGoldenData* pdata = (CGoldenData*)getRoomData();
	if ( pdata->getRound() < 5 )
	{
		return false ;
	}

	float fRate = 25 ;
	if ( pdata->getRound() < 15 )
	{
		fRate = 10 ;
		if ( nCardtype == CPeerCard::ePeerCard_None )
		{
			fRate = 25 ;
		}
		
	}
	else if ( pdata->getRound() < 25 )
	{
		fRate = 15 ;
	}
	else if ( pdata->getRound() < 40 )
	{
		fRate = 20 ;
	}

	float nRate = (float)rand() / float(RAND_MAX);
	nRate *= 100 ;
	return nRate < fRate  ;
}

bool CRobotControlGolden::canPKCard()
{
	auto pSelf = (stGoldenPlayer*)getRoomData()->getPlayerByIdx(getSeatIdx()) ;
	if ( !pSelf )
	{
		printf("self player is null why ? \n") ;
		return false;
	}

	CGoldenData* pdata = (CGoldenData*)getRoomData();
	if ( pdata->getRound() < 5 )
	{
		return false ;
	}

	float fRate = 10 ;
	if ( pdata->getRound() < 20 )
	{
		fRate = 8 ;
	}
	else if ( nCardtype == CPeerCard::ePeerCard_None )
	{
		fRate = 18 ;
	}

	float nRate = (float)rand() / float(RAND_MAX);
	nRate *= 100 ;
	return nRate < fRate  ;
}

bool CRobotControlGolden::canAddBet()
{
	auto pSelf = (stGoldenPlayer*)getRoomData()->getPlayerByIdx(getSeatIdx()) ;
	if ( !pSelf )
	{
		printf("self player is null why ? \n") ;
		return false;
	}

	CGoldenData* pdata = (CGoldenData*)getRoomData();
	if ( pdata->getRound() > 25 )
	{
		return false ;
	}

	float fRate = 0 ;
	if ( nCardtype == CPeerCard::ePeerCard_None )
	{
		fRate = 8 ;
	}
	else if ( nCardtype == CPeerCard::ePeerCard_Pair )
	{
		fRate = 15 ;
	}
	else
	{
		fRate = 20 ;
	}

	float nRate = (float)rand() / float(RAND_MAX);
	nRate *= 100 ;
	return nRate < fRate  ;
}

bool CRobotControlGolden::canGiveUp()
{
	auto pSelf = (stGoldenPlayer*)getRoomData()->getPlayerByIdx(getSeatIdx()) ;
	if ( !pSelf )
	{
		printf("self player is null why ? \n") ;
		return false;
	}

	CGoldenData* pdata = (CGoldenData*)getRoomData();
	if ( pdata->getRound() < 5 )
	{
		return false ;
	}

	if ( pSelf->isHaveState(eRoomPeer_Looked) == false )
	{
		return false ;
	}

	float fRate = 0 ;
	if ( nCardtype == CPeerCard::ePeerCard_None )
	{
		fRate = 8 ;
		if ( pdata->getRound() > 20 )
		{
			fRate = 10 ;
		}

		if ( pdata->getRound() > 30 )
		{
			fRate = 15 ;
		}
	}
	else
	{
		return false ;
	}

	float nRate = (float)rand() / float(RAND_MAX);
	nRate *= 100 ;
	return nRate < fRate  ;
}

uint8_t CRobotControlGolden::randPKTargetIdx()
{
	uint8_t nSeatCnt = getRoomData()->getSeatCount() ;
	uint8_t nIdx = rand() % nSeatCnt ;
	uint8_t nRealIdx = 0 ;
	for ( ; nIdx < nSeatCnt * 2 ; ++nIdx )
	{
		 nRealIdx = nIdx % nSeatCnt ;
		 auto pp = getRoomData()->getPlayerByIdx(nRealIdx) ;
		 if ( pp && pp->isHaveState(eRoomPeer_CanAct) && nRealIdx != getSeatIdx() )
		 {
			 return nRealIdx ;
		 }
	}
	printf("can not find pk target proper, will crash \n") ;
	return -1 ;
}

void CRobotControlGolden::onSelfSitDown()
{
	if ( getRoomData()->getRoomState() == eRoomState_TP_WaitJoin )
	{
		float nRate = (float)rand() / float(RAND_MAX);
		nRate = 0.5f + nRate * 1.0f ;
		fireDelayAction(eRoomPeerAction_Ready,nRate,nullptr);
	}
}

void CRobotControlGolden::onGameEnd()
{
	CRobotControl::onGameEnd();
	float nRate = (float)rand() / float(RAND_MAX);
	nRate = 2.0f + nRate * 2.0f ;
	fireDelayAction(eRoomPeerAction_Ready,nRate,nullptr);
}