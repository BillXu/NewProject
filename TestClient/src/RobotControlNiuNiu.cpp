#include "RobotControlNiuNiu.h"
#include "NiuNiuScene.h"
#include <vector>
#include "RobotConfig.h"
#include "PlayerData.h"
#include "ClientRobot.h"
bool CRobotControlNiuNiu::init(CNiuNiuScene* pScene )
{
	CRobotControl::init() ;
	m_pScene = pScene ;
	m_eState = eRcs_StandUp ;
	m_fWaitSitDownTicket = 0 ;
	m_nSelfIdx = MAX_PEERS_IN_TAXAS_ROOM ;
	return true ;
}

void CRobotControlNiuNiu::onCanTryBanker()
{
	if ( m_nSelfIdx != MAX_PEERS_IN_TAXAS_ROOM )
	{
		float nRate = (float)rand() / float(RAND_MAX);
		nRate = 1.0f + nRate * 2.0f ;
		uint32_t nType = 1 ;
		fireDelayAction(nRate,(void*)nType);
	}
}

void CRobotControlNiuNiu::onCanBet()
{
	if ( m_nSelfIdx != MAX_PEERS_IN_TAXAS_ROOM && m_nSelfIdx != m_pScene->getPokerData()->nBankerIdx )
	{
		float nRate = (float)rand() / float(RAND_MAX);
		nRate = 1.0f + nRate * 4.0f ;
		uint32_t nType = 2 ;
		fireDelayAction(nRate,(void*)nType);
	}
}

void CRobotControlNiuNiu::onGameEnd()
{
	// check if we need standup ;
	float fStandUpRate = 0 ;
	if ( m_pScene->getPokerData()->getSitDownPlayerCnt() < m_pScene->getClientApp()->GetPlayerData()->pRobotItem->nApplyLeaveWhenPeerCount ) 
	{
		fStandUpRate = 0.11;
	}
	else
	{
		fStandUpRate = 0.36 ;
	}
	
	float nRate = (float)rand() / float(RAND_MAX);
	if ( nRate <= fStandUpRate )
	{
		printf("rate me to stand up , fStandRate = %02f\n",fStandUpRate) ;
		standUp() ;
	}
}

void CRobotControlNiuNiu::doDelayAction(void* pUserData )
{
	uint32_t nType = (uint32_t)pUserData ;
	if ( nType == 1 ) // try banker 
	{
		float nRate = (float)rand() / float(RAND_MAX); 
		bool bTryBanker = nRate < 0.3 ;
		if ( !bTryBanker )
		{
			stMsgNNPlayerTryBanker msgTryBanker ;
			msgTryBanker.nRoomID = m_pScene->getPokerData()->nRoomID ;
			msgTryBanker.nTryBankerBetTimes = 0 ;
			m_pScene->SendMsg((char*)&msgTryBanker,sizeof(msgTryBanker)) ;
			return ;
		}

		// 1 , 2 , 3 , 4 
		std::vector<uint8_t> vTimes ;
		vTimes.push_back(1);
		vTimes.push_back(2);
		vTimes.push_back(3);
		vTimes.push_back(4);

		while( vTimes.empty() == false )
		{
			uint8_t nTime = vTimes.back() ;
			uint32_t nMostMayLose = m_pScene->getPokerData()->getBaseBet() * nTime * 25 * ( m_pScene->getPokerData()->getSitDownPlayerCnt() - 1 );
			stNNRoomInfoPayerItem* pPlayer = m_pScene->getPokerData()->getPlayerByIdx( m_nSelfIdx ) ;
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
		msgTryBanker.nRoomID = m_pScene->getPokerData()->nRoomID ;
		msgTryBanker.nTryBankerBetTimes = nTryTimes ;
		m_pScene->SendMsg((char*)&msgTryBanker,sizeof(msgTryBanker)) ;
		printf("i do try banker times = %d \n",nTryTimes) ;
	}
	else if ( 2 == nType ) // bet 
	{
		// 5 , 10 , 15 , 20, 25 
		std::vector<uint8_t> vTimes ;
		vTimes.push_back(5);
		vTimes.push_back(10);
		vTimes.push_back(15);
		vTimes.push_back(25);

		while ( vTimes.empty() == false )
		{
			uint8_t nTimes = vTimes.back() ;
			uint32_t nMayLost = m_pScene->getPokerData()->getFinalBaseBet() * nTimes ;
			if ( nMayLost > m_pScene->getPokerData()->getPlayerByIdx(m_nSelfIdx)->nCoin || nMayLost > m_pScene->getPokerData()->getLeftCanBetCoin() )
			{
				vTimes.pop_back() ;
				continue;
			}
			else
			{
				break;
			}
		}

		uint8_t nBetTimes  = 0 ;
		if ( vTimes.empty() == false )
		{
			uint8_t nIdx = rand() % vTimes.size() ;
			nBetTimes = vTimes[nIdx] ;
		}

		stMsgNNPlayerBet msgBet ;
		msgBet.nRoomID = m_pScene->getPokerData()->nRoomID ;
		msgBet.nBetTimes = nBetTimes ;
		m_pScene->SendMsg((char*)&msgBet,sizeof(msgBet)) ;
		printf("i do bet , times = %d\n",nBetTimes) ;
	}
}

void CRobotControlNiuNiu::update(float fdeta )
{
	CRobotControl::update(fdeta) ;
	updateWaitSitdown(fdeta);

}

void CRobotControlNiuNiu::updateWaitSitdown(float fdeta )
{
	if ( m_eState == eRcs_WaitToSitDown )
	{
		if ( (m_fWaitSitDownTicket -= fdeta ) <= 0 ) // do sholud sit down operatertion 
		{
			if ( m_pScene->getPokerData()->getSitDownPlayerCnt() < m_pScene->getClientApp()->GetPlayerData()->pRobotItem->nApplyLeaveWhenPeerCount ) 
			{
				float fRate = (float)rand() / float(RAND_MAX);  
				if ( fRate < 0.45 )  // 40% rate i sit down , this time
				{
					// sit down 
					m_eState = eRcs_SitingDown ;
					stMsgNNPlayerSitDown msgSitDown ;
					msgSitDown.nRoomID = m_pScene->getPokerData()->nRoomID ;
					msgSitDown.nSeatIdx = m_pScene->getPokerData()->getRandEmptySeatIdx();
					m_pScene->SendMsg((char*)&msgSitDown,sizeof(msgSitDown)) ;
					printf("get the rate i sit down \n") ;
				}
				else
				{
					// go on wait to sit down 
					printf("do not get the rate,i go wait to sit down \n") ;
					waitToSitdown() ;
				}
			}
			else
			{
				// go on wait to sit down 
				printf("too many player , i go wait to sit down \n") ;
				waitToSitdown() ;
			}

		}
	}
}

bool CRobotControlNiuNiu::onMsg(stMsg* pmsg)
{
	switch ( pmsg->usMsgType )
	{
	case MSG_NN_ROOM_INFO:
		{
			waitToSitdown() ;
		}
		break;
	case MSG_NN_PLAYER_SITDOWN:
		{
			stMsgNNPlayerSitDownRet* pRet = (stMsgNNPlayerSitDownRet*)pmsg ;
			if ( pRet->nRet )
			{
				printf("sit down failed , i go on wait to sit down ret = %d\n",pRet->nRet) ;
				waitToSitdown() ;
			}
			else
			{
				m_eState = eRcs_SitDown ;
				printf("i sit down ok , player game \n") ;
			}
		}
		break;
	case MSG_NN_SITDOWN:
		{
			stMsgNNSitDown* pSitDown = (stMsgNNSitDown*)pmsg ;
			if ( pSitDown->tSitDownPlayer.nUserUID == m_pScene->getClientApp()->GetPlayerData()->getUserUID() )
			{
				m_eState = eRcs_SitDown ;
				m_nSelfIdx = pSitDown->tSitDownPlayer.nIdx ;
				printf("i sit down ok , player game idx = %d \n",m_nSelfIdx) ;
			}
		}
		break;
	case MSG_NN_STANDUP:
		{
			if ( m_eState == eRcs_StandingUp )
			{
				stMsgNNStandUp* pStandUp = (stMsgNNStandUp*)pmsg ;
				if ( pStandUp->nPlayerIdx == m_nSelfIdx )
				{
					m_nSelfIdx = MAX_PEERS_IN_TAXAS_ROOM ;

					printf("i was standup , wait to sit down again \n") ;
					waitToSitdown() ;
				}
			}
		}
		break;
	default:
		return false;
	}
	return true ;
}

void CRobotControlNiuNiu::waitToSitdown()
{
	m_fWaitSitDownTicket = (float)rand() / float(RAND_MAX);  
	m_fWaitSitDownTicket *= 6.0f ;
	m_fWaitSitDownTicket += 1.0f ;
	m_eState = eRcs_WaitToSitDown ;
}

void CRobotControlNiuNiu::standUp()
{
	if ( m_eState == eRcs_SitDown )
	{
		stMsgNNPlayerStandUp msgStandUp ;
		msgStandUp.nRoomID = m_pScene->getPokerData()->nRoomID ;
		m_pScene->SendMsg((char*)&msgStandUp,sizeof(msgStandUp)) ;
		m_eState = eRcs_StandingUp ;
	}
}