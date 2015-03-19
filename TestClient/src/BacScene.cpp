#include "BacScene.h"
#include "BaccaratMessageDefine.h"
#include "ClientRobot.h"
#include "PlayerData.h"
CBacScene::CBacScene(CClientRobot* pClient)
	:IScene(pClient)
{
	m_eSceneType = eScene_Bac ;
	m_fTimeTicket = 0 ;
	m_bCanMyAction = false ;
	m_bLastActionFinished = true ;
}

void CBacScene::OnUpdate(float fDeltaTime )
{
	if ( m_bCanMyAction == false )
	{
		return ;
	}

	m_fTimeTicket -= fDeltaTime ;
	if ( m_fTimeTicket <= 0 && m_bLastActionFinished )
	{
		m_bCanMyAction = false ;
		m_fTimeTicket = 0 ;
		DoMyAction() ;
	}
}

bool CBacScene::OnMessage( Packet* pPacket )
{
	if ( IScene::OnMessage(pPacket) )
	{
		return true ;
	}

	stMsg* pmsg = (stMsg*)pPacket->_orgdata ;
	switch ( pmsg->usMsgType )
	{
	case MSG_BC_DISTRIBUTE:
		{
			m_bCanMyAction = false ; 
		}
		break;
	case MSG_BC_ROOM_INFO:
		{
			stMsgBaccaratRoomInfo* pInfo = (stMsgBaccaratRoomInfo*)pmsg ;
			memcpy(m_vBetPort,pInfo->vBetPort,sizeof(m_vBetPort));
		}
		break;
	case MSG_BC_OTHER_BET:
		{
			stMsgBCOtherPlayerBet* pBetmsg = (stMsgBCOtherPlayerBet*)pmsg ;
			m_vBetPort[pBetmsg->cBetPort] += pBetmsg->nBetCoin ;

			if ( pBetmsg->nSession == m_pClient->GetSessionID() )
			{
				m_pClient->GetPlayerData()->nMyCoin -= pBetmsg->nBetCoin;
				m_bLastActionFinished = true ;
				MyAction();
			}
		}
		break;
	case MSG_BC_BET:
		{
			stMsgBCPlayerBetRet* pRetMsg = (stMsgBCPlayerBetRet*)pmsg ;
			if ( pRetMsg->nRet == 4 )
			{
				++nBetTimes ;
			}
			m_bLastActionFinished = true ;
			MyAction();
		}
		break;
	case MSG_BC_CACULATE:
		{
			stMsgBCCaculate* pMsgRet = (stMsgBCCaculate*)pmsg ;
			m_pClient->GetPlayerData()->nMyCoin = pMsgRet->nFinalCoin ;
		}
		break;
	case MSG_BC_START_BET:
		{
			nBetTimes = rand() % 4 +  1 ;
			m_bLastActionFinished = true ;
			printf("I start bet !\n") ;
			MyAction();
		}
		break;
	default:
		return false ;
	}
	return true ;
}

void CBacScene::DoMyAction()
{
	if ( nBetTimes-- < 0 )
	{
		m_bCanMyAction = false ;
		nBetTimes = 0 ;
	}

	if ( m_pClient->GetPlayerData()->nMyCoin < 50000 )
	{
		m_bCanMyAction = false ;
		return ;
	}
	//eBC_BetPort_BankerWin = eBC_BetPort_One,
	//	eBC_BetPort_IdleWin,
	//	eBC_BetPort_TheSame,
	//	eBC_BetPort_BankerPair,
	//	eBC_BetPort_IdlePair,

	static int nPortRate[eBC_BetPort_Max] = {39,39,6,8,8 };
	int i = 0 ;//rand() % eBC_BetPort_Max ; 
	int nRate = rand() % 100 ;
	printf( "rand get a nRate = %d \n",nRate ) ;
	int nTatal = 0 ;
	for ( ; i < eBC_BetPort_Max * 2 ; ++i )
	{
		int nIdx = i % eBC_BetPort_Max ;
		if ( nTatal <= nRate && nRate < nTatal + nPortRate[nIdx] )
		{
			break;
		}
		else
		{
			nTatal += nPortRate[nIdx]  ;
		}
	}

	stMsgBCPlayerBet msgBet ;
	msgBet.cBetPort = i % eBC_BetPort_Max ;
	printf("bet port %d \n",msgBet.cBetPort);
	msgBet.nBetCoin = 50000;
	SendMsg(&msgBet,sizeof(msgBet)) ;
	m_bLastActionFinished = false ;
	printf("I send bet msg \n") ;
}

void CBacScene::MyAction()
{
	m_bCanMyAction = true ;
	m_fTimeTicket = (float)rand() / float(RAND_MAX) ;
	m_fTimeTicket = m_pClient->GetPlayerData()->pRobotItem->fActDelayBegin + m_fTimeTicket * (m_pClient->GetPlayerData()->pRobotItem->fActDelayEnd - m_pClient->GetPlayerData()->pRobotItem->fActDelayBegin ) ;
	m_fTimeTicket *= 2 ;
}