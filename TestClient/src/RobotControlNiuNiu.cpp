#include "RobotControlNiuNiu.h"
#include "NiuNiuScene.h"
#include <vector>
#include "RobotConfig.h"
#include "PlayerData.h"
#include "ClientRobot.h"
#include "LoginScene.h"
#include "NiuNiuData.h"

void CRobotControlNiuNiu::informRobotAction(uint8_t nActType)
{
	stSitableRoomPlayer* pPlayer = getRoomData()->getPlayerByIdx( getSeatIdx() ) ;
	if ( !pPlayer )
	{
		printf("not sit down \n");
		return ;
	}

	if ( pPlayer->isHaveState(eRoomPeer_CanAct) == false )
	{
		printf("uid = %u not join this game wait next\n",pPlayer->nUserUID);
		return ;
	}

	switch ( nActType )
	{
	case eAct_Bet:
		{
			auto roomData = (stNiuNiuData*)getRoomData();
			if ( roomData->nBankerIdx == getSeatIdx() )
			{
				printf("uid = %u i am banker need not bet \n",pPlayer->nUserUID );
				return ;
			}

			float nRate = (float)rand() / float(RAND_MAX);
			nRate = 1.0f + nRate * 3.0f ;
			fireDelayAction(nActType,nRate,nullptr);
		}
		break;
	case eAct_TryBanker:
		{
			float nRate = (float)rand() / float(RAND_MAX);
			nRate = 1.0f + nRate * 2.0f ;
			fireDelayAction(nActType,nRate,nullptr);
		}
		break;
	case eAct_CaculateCards:
		{
			float nRate = (float)rand() / float(RAND_MAX);
			nRate = 2.3f + nRate * 4.0f ;
			fireDelayAction(nActType,nRate,nullptr);
		}
		break;
	default:
		break;
	}
}

void CRobotControlNiuNiu::doDelayAction(uint8_t nActType,void* pUserData )
{
	if ( this == nullptr )
	{
		printf("why this is null \n") ;
		return ;
	}

	//if ( 3 == nType )
	//{
	//	m_pScene->getClientApp()->GetPlayerData()->setIsLackOfCoin(true) ;
	//	auto au = new CLoginScene(m_pScene->getClientApp());
	//	m_pScene->getClientApp()->ChangeScene(au);
	//	printf("do change room to get coin \n");
	//	return ;
	//}
	else if ( nActType == eAct_TryBanker ) // try banker 
	{
		bool bHaveNiu = ((stNiuNiuData*)getRoomData())->isHaveNiu(getSeatIdx());
		float fTargetRate = bHaveNiu ? 0.8 : 0.15;
		float nRate = (float)rand() / float(RAND_MAX); 
		bool bTryBanker = nRate <= fTargetRate ;
		if ( !bTryBanker )
		{
			stMsgNNPlayerTryBanker msgTryBanker ;
			msgTryBanker.nRoomID = getRoomData()->getRoomID() ;
			msgTryBanker.nTryBankerBetTimes = 0 ;
			sendMsg(&msgTryBanker,sizeof(msgTryBanker)) ;
			return ;
		}

		// 1 , 2 , 3 , 4 
		std::vector<uint8_t> vTimes ;
		vTimes.push_back(1);
		vTimes.push_back(2);
		vTimes.push_back(3);
		if ( bHaveNiu || ( rand() % 100 <= 40 ) )
		{
			vTimes.push_back(4);
		}

		while( vTimes.empty() == false )
		{
			uint8_t nTime = vTimes.back() ;
			uint32_t nMostMayLose = ((stNiuNiuData*)getRoomData())->getBaseBet() * nTime * 25 * ( getRoomData()->getPlayerCnt() - 1 );
			stSitableRoomPlayer* pPlayer = getRoomData()->getPlayerByIdx( getSeatIdx() ) ;
			if ( pPlayer == nullptr )
			{
				return ;
			}

			if ( pPlayer->nCoin >= nMostMayLose )
			{
				break; 
			}
			else
			{
				 vTimes.pop_back() ;
			}
		}

		uint8_t nTryTimes  = 0 ;
		if ( vTimes.empty() == false )
		{
			uint8_t nIdx = rand() % vTimes.size() ;
			nTryTimes = vTimes[nIdx] ;
		}
		stMsgNNPlayerTryBanker msgTryBanker ;
		msgTryBanker.nRoomID = getRoomData()->getRoomID() ;
		msgTryBanker.nTryBankerBetTimes = nTryTimes ;
		msgTryBanker.nSubRoomIdx = getRoomData()->getSubRoomIdx() ;
		sendMsg(&msgTryBanker,sizeof(msgTryBanker)) ;
		printf("i do try banker times = %d \n",nTryTimes) ;
	}
	else if ( nActType == eAct_Bet ) // bet 
	{
		bool bHaveNiu = ((stNiuNiuData*)getRoomData())->isHaveNiu(getSeatIdx());
		// 5 , 10 , 15 , 20, 25 
		std::vector<uint8_t> vTimes ;
		vTimes.push_back(5);
		vTimes.push_back(10);
		if ( bHaveNiu || ( rand() % 100 <= 30 ) )
		{
			vTimes.push_back(15);
			vTimes.push_back(25);
		}

		while ( vTimes.empty() == false )
		{
			uint8_t nTimes = vTimes.back() ;
			uint32_t nMayLost = ((stNiuNiuData*)getRoomData())->getFinalBaseBet() * nTimes ;
			if ( nMayLost > getRoomData()->getPlayerByIdx(getSeatIdx())->nCoin || nMayLost > ((stNiuNiuData*)getRoomData())->getLeftCanBetCoin() )
			{
				vTimes.pop_back() ;
				continue;
			}
			else
			{
				break;
			}
		}

		uint8_t nBetTimes  = 5 ;
		if ( vTimes.empty() == false )
		{
			uint8_t nIdx = rand() % vTimes.size() ;
			nBetTimes = vTimes[nIdx] ;
		}

		stMsgNNPlayerBet msgBet ;
		msgBet.nRoomID = getRoomData()->getRoomID() ;
		msgBet.nBetTimes = nBetTimes ;
		msgBet.nSubRoomIdx = getRoomData()->getSubRoomIdx() ;
		sendMsg(&msgBet,sizeof(msgBet)) ;
		printf("i do bet , times = %d\n",nBetTimes) ;
	}
	else if ( eAct_CaculateCards == nActType )
	{
		stMsgNNPlayerCaculateCardOk msg ;
		msg.nRoomID = getRoomData()->getRoomID() ;
		msg.nSubRoomIdx = getRoomData()->getSubRoomIdx() ;
		sendMsg(&msg,sizeof(msg)) ;
		printf("i do caculate card ok\n");
	}
	CRobotControl::doDelayAction(nActType,pUserData);
}


