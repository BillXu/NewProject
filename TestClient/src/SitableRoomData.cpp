#include "SitableRoomData.h"
#include <string>
#include "robotControl.h"
#include "Client.h"
#include "PlayerData.h"
#include "IScene.h"
CSitableRoomData::CSitableRoomData()
{
	memset(m_vSitDownPlayer,0,sizeof(m_vSitDownPlayer));
	m_isActive = true ;
	m_pScene = nullptr ;
	m_nCurGameOffset = 0 ;
}

CSitableRoomData::~CSitableRoomData()
{
	for ( auto player : m_vSitDownPlayer )
	{
		if ( player )
		{
			delete player ;
			player = nullptr ;
		}
	}

	if ( m_pRobot )
	{
		delete m_pRobot ;
		m_pRobot = nullptr ;
	}

	memset(m_vSitDownPlayer,0,sizeof(m_vSitDownPlayer));
}

void CSitableRoomData::init(IScene* pScene )
{
	m_pScene = pScene ;
	m_pRobot = doCreateRobotControl();
	m_pRobot->init( pScene->getClient()->GetPlayerData()->getConfigItem(),this,pScene->getClient()->GetPlayerData()->getUserUID()) ;
}

void CSitableRoomData::setBaseInfo(uint32_t nRoomID, uint8_t nSeatCnt, uint32_t nDeskFee, uint8_t nRoomState,uint8_t nSubRoomIdx)
{
	m_nDeskFee = nDeskFee ;
	m_nRoomID = nRoomID ;
	m_nSeatCount = nSeatCnt ;
	m_nRoomState = nRoomState ;
	m_nSubRoomIdx = nSubRoomIdx ;
	memset(m_vSitDownPlayer,0,sizeof(m_vSitDownPlayer)) ;
	getRobotControl()->onReicvedRoomData();
}

bool CSitableRoomData::onMsg(stMsg* pmsg )
{
	switch (pmsg->usMsgType)
	{
	case MSG_REQ_CUR_GAME_OFFSET:
		{
			stMsgReqRobotCurGameOffsetRet* pRet = (stMsgReqRobotCurGameOffsetRet*)pmsg ;
			m_nCurGameOffset = pRet->nCurGameOffset ; 
			printf("recieved game offset = %d uid = %u\n",m_nCurGameOffset,m_pScene->getClient()->GetPlayerData()->getUserUID()) ;
		}
		break;
	case MSG_PLAYER_BE_ADDED_FRIEND:
		{
			stMsgPlayerBeAddedFriend* pret = (stMsgPlayerBeAddedFriend*)pmsg ;
			printf("uid = %d want add me friend \n",pret->nPlayerUserUID);

			stMsgPlayerBeAddedFriendReply msgReply ;
			msgReply.bAgree = true ;
			msgReply.nReplayToPlayerUserUID = pret->nPlayerUserUID ;
			sendMsg(&msgReply,sizeof(msgReply));
		}
		break;
	case MSG_PLAYER_BE_ADDED_FRIEND_REPLY:
		{
			stMsgPlayerBeAddedFriendReplyRet* pret = (stMsgPlayerBeAddedFriendReplyRet*)pmsg ;
			printf("recive replay ret = %d , new friend uid = %d\n",pret->nRet,pret->nNewFriendUserUID) ;
		}
		break;
	case MSG_ROOM_SITDOWN:
		{
			stMsgRoomSitDown* p = (stMsgRoomSitDown*)pmsg ;
			auto pPlayer = m_vSitDownPlayer[p->nIdx] ;
			if ( !pPlayer )
			{
				pPlayer = doCreateSitDownPlayer();
				m_vSitDownPlayer[p->nIdx] = pPlayer ;
			}
			pPlayer->reset() ;
			pPlayer->nIdx = p->nIdx ;
			pPlayer->nCoin = p->nTakeInCoin ;
			pPlayer->nUserUID = p->nSitDownPlayerUserUID ;
			pPlayer->nStateFlag = eRoomPeer_WaitNextGame ;
			printf("player idx = %d uid = %d sit down \n",pPlayer->nIdx,pPlayer->nUserUID);
		}
		break;
	case MSG_ROOM_STANDUP:
		{
			stMsgRoomStandUp* p = (stMsgRoomStandUp*)pmsg ;
			if ( m_vSitDownPlayer[p->nIdx] )
			{
				m_vSitDownPlayer[p->nIdx]->reset() ;
			}
		}
		break;
	case MSG_ROOM_ENTER_NEW_STATE:
		{
			stMsgRoomEnterNewState* pRet = (stMsgRoomEnterNewState*)pmsg ;
			m_nRoomState = pRet->nNewState ;
			if ( pRet->nNewState == eRoomState_Close )
			{
				m_isActive = false ;

				stMsgPlayerLeaveRoom msgLeave ;
				msgLeave.cSysIdentifer = getTargetSvrPort() ;
				msgLeave.nRoomID = getRoomID() ;
				msgLeave.nSubRoomIdx = getSubRoomIdx() ;
				sendMsg(&msgLeave,sizeof(msgLeave)) ;
				printf("room closed just leave this room\n") ;
			}
			else if ( eRoomState_WaitJoin == pRet->nNewState )
			{
				m_isActive = true ;
			}

			if ( eRoomState_DidGameOver == pRet->nNewState )
			{
				onGameEnd() ;
			}
		}
		break;
	default:
		break;
	}

	if ( getRobotControl() )
	{
		getRobotControl()->onMsg(pmsg) ;
	}
	return false ;
}

void CSitableRoomData::onGameBegin()
{
	for ( auto player : m_vSitDownPlayer )
	{
		if ( player != nullptr && player->isValid() )
		{
			player->nCoin -= m_nDeskFee ;
			player->nStateFlag = eRoomPeer_CanAct ;
			player->onGameBegin() ;
		}
	}

	if ( getRobotControl() )
	{
		getRobotControl()->onGameBegin();
	}
}

void CSitableRoomData::onGameEnd()
{
	for ( auto player : m_vSitDownPlayer )
	{
		if ( player != nullptr && player->isValid() )
		{
			player->onGameEnd() ;
		}
	}

	if ( getRobotControl() )
	{
		getRobotControl()->onGameEnd();
	}

	stMsgReqRobotCurGameOffset msgMsg ;
	msgMsg.nRoomID = getRoomID();
	msgMsg.nSubRoomIdx = getSubRoomIdx();
	msgMsg.cSysIdentifer = getTargetSvrPort();
	sendMsg(&msgMsg,sizeof(msgMsg)) ;
}

uint8_t CSitableRoomData::getPlayerCntWithState(  uint32_t nState  )
{
	uint8_t nCnt = 0 ;
	for ( auto player : m_vSitDownPlayer )
	{
		if ( player != nullptr && player->isValid() && player->isHaveState(nState) )
		{
			++nCnt ;
		}
	}
	return nCnt ;
}

stSitableRoomPlayer* CSitableRoomData::getPlayerByIdx( uint8_t nIdx )
{ 
	if ( nIdx >= m_nSeatCount )
	{
		return nullptr ;
	}
	return m_vSitDownPlayer[nIdx];
}

int8_t CSitableRoomData::getRandEmptySeatIdx()
{
	uint8_t nMaxSeatCnt = m_nSeatCount ;
	uint8_t nStartIdx = rand() % nMaxSeatCnt ;
	for ( uint8_t nIdx = nStartIdx ; nIdx < nMaxSeatCnt * 2 ; ++nIdx )
	{
		uint8_t nRealIdx = nIdx % nMaxSeatCnt ;
		if ( m_vSitDownPlayer[nRealIdx] == nullptr || m_vSitDownPlayer[nRealIdx]->isValid() == false )
		{
			return nRealIdx ;
		}
	}
	return -1 ;
}

void CSitableRoomData::sendMsg(stMsg* pmsg , uint16_t nLen )
{
	if ( m_pScene )
	{
		m_pScene->SendMsg(pmsg,nLen) ;
	}
}

int32_t CSitableRoomData::getGameOffset()
{
	return m_pScene->getClient()->GetPlayerData()->getTotalGameOffset() + m_nCurGameOffset ;
}