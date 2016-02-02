#include "ISitableRoom.h"
#include "RoomConfig.h"
#include "ISitableRoomPlayer.h"
#include <cassert>
#include "MessageDefine.h"
#include "AutoBuffer.h"
#include "ServerMessageDefine.h"
ISitableRoom::~ISitableRoom()
{
	for ( uint8_t nIdx = 0 ; nIdx < m_nSeatCnt ; ++nIdx )
	{
		if ( m_vSitdownPlayers[nIdx])
		{
			delete m_vSitdownPlayers[nIdx];
			m_vSitdownPlayers[nIdx] = nullptr ;
		}
	}
	delete [] m_vSitdownPlayers;

	for ( ISitableRoomPlayer* pPlayer : m_vReserveSitDownObject )
	{
		delete pPlayer ;
		pPlayer = nullptr ;
	}
	m_vReserveSitDownObject.clear() ;
}

bool ISitableRoom::init(stBaseRoomConfig* pConfig, uint32_t nRoomID ) 
{
	IRoom::init(pConfig,nRoomID) ;
	stSitableRoomConfig* pC = (stSitableRoomConfig*)pConfig;
	m_nSeatCnt = pC->nMaxSeat ;
	m_vSitdownPlayers = new ISitableRoomPlayer*[m_nSeatCnt] ;
	for ( uint8_t nIdx = 0 ; nIdx < m_nSeatCnt ; ++nIdx )
	{
		m_vSitdownPlayers[nIdx] = nullptr ;
	}
	return true ;
}

bool ISitableRoom::playerSitDown(ISitableRoomPlayer* pPlayer , uint8_t nIdx )
{
	if ( isSeatIdxEmpty(nIdx) )
	{
		m_vSitdownPlayers[nIdx] = pPlayer ;
		pPlayer->doSitdown(nIdx) ;
		pPlayer->setIdx(nIdx);

		// save standup log ;
		stMsgSaveLog msgLog ;
		msgLog.nJsonExtnerLen = 0 ;
		msgLog.nLogType = eLog_PlayerSitDown ;
		msgLog.nTargetID = pPlayer->getUserUID() ;
		memset(msgLog.vArg,0,sizeof(msgLog.vArg)) ;
		msgLog.vArg[0] = getRoomType() ;
		msgLog.vArg[1] = getRoomID() ;
		msgLog.vArg[2] = pPlayer->getCoin() ;
		sendMsgToPlayer(&msgLog,sizeof(msgLog),getRoomID()) ;
		return true ;
	}
	return false ;
}

void ISitableRoom::playerStandUp( ISitableRoomPlayer* pPlayer )
{
	assert(isSeatIdxEmpty(pPlayer->getIdx()) == false && "player not sit down" );
	pPlayer->willStandUp();
	m_vSitdownPlayers[pPlayer->getIdx()] = nullptr ;
	m_vReserveSitDownObject.push_back(pPlayer) ;
}

uint8_t ISitableRoom::getEmptySeatCount()
{
	uint8_t nCount = 0 ;
	for ( uint8_t nIdx = 0 ; nIdx < m_nSeatCnt ; ++nIdx )
	{
		if ( m_vSitdownPlayers[nIdx] == nullptr )
		{
			++nCount ;
		}
	}
	return nCount ;
}

ISitableRoomPlayer* ISitableRoom::getPlayerByIdx(uint8_t nIdx )
{
	assert(nIdx < getSeatCount() && "invalid player idx");
	if ( nIdx >= getSeatCount() )
	{
		return nullptr ;
	}
	return m_vSitdownPlayers[nIdx] ;
}

bool ISitableRoom::isSeatIdxEmpty( uint8_t nIdx )
{
	assert(nIdx < getSeatCount() && "invalid player idx");
	if ( nIdx >= getSeatCount() )
	{
		return false ;
	}
	return m_vSitdownPlayers[nIdx] == nullptr ;
}

uint8_t ISitableRoom::getSitDownPlayerCount()
{
	return getSeatCount() - getEmptySeatCount() ;
}

uint8_t ISitableRoom::getSeatCount()
{
	return m_nSeatCnt ;
}

ISitableRoomPlayer* ISitableRoom::getReuseSitableRoomPlayerObject()
{
	LIST_SITDOWN_PLAYERS::iterator iter = m_vReserveSitDownObject.begin() ;
	if ( iter != m_vReserveSitDownObject.end() )
	{
		ISitableRoomPlayer* p = *iter ;
		m_vReserveSitDownObject.erase(iter) ;
		return p ;
	}
	return doCreateSitableRoomPlayer();
}

uint8_t ISitableRoom::getPlayerCntWithState( uint32_t nState )
{
	uint8_t nCount = 0 ;
	for ( uint8_t nIdx = 0 ; nIdx < m_nSeatCnt ; ++nIdx )
	{
		if ( m_vSitdownPlayers[nIdx] && m_vSitdownPlayers[nIdx]->isHaveState(nState) )
		{
			++nCount ;
		}
	}
	return nCount ;
}

ISitableRoomPlayer* ISitableRoom::getSitdownPlayerBySessionID(uint32_t nSessionID)
{
	for ( uint8_t nIdx = 0 ; nIdx < m_nSeatCnt ; ++nIdx )
	{
		if ( m_vSitdownPlayers[nIdx] && m_vSitdownPlayers[nIdx]->getSessionID() == nSessionID )
		{
			return m_vSitdownPlayers[nIdx] ;
		}
	}
	return nullptr ;
}

bool ISitableRoom::onMessage( stMsg* prealMsg , eMsgPort eSenderPort , uint32_t nPlayerSessionID )
{
	if ( IRoom::onMessage(prealMsg,eSenderPort,nPlayerSessionID) )
	{
		return true ;
	}

	switch ( prealMsg->usMsgType )
	{
	case MSG_REQUEST_ROOM_RANK:
		{
			std::map<uint32_t,stRoomRankEntry> vWillSend ;
			sortRoomRankItem();
			// add 15 player into list ;
			LIST_ROOM_RANK_ITEM::iterator iter = getSortRankItemListBegin();
			for ( uint8_t nIdx = 0 ; iter != getSortRankItemListEnd() && nIdx < 15; ++iter,++nIdx )
			{
				stRoomRankItem* pItem = (*iter) ;
				stRoomRankEntry entry ;
				entry.nOffset = pItem->nOffset ;
				entry.nUserUID = pItem->nUserUID ;
				vWillSend[pItem->nUserUID] = entry ;
			}

			// keep all sit down player are in willSend
			for ( uint8_t nIdx = 0 ; nIdx < m_nSeatCnt ; ++nIdx )
			{
				auto sitDownPlayer = m_vSitdownPlayers[nIdx] ;
				if ( m_vSitdownPlayers[nIdx] && m_vSitdownPlayers[nIdx]->getUserUID() != 0 )
				{
					auto Iter = vWillSend.find(sitDownPlayer->getUserUID()) ;
					if ( Iter == vWillSend.end() )
					{
						stRoomRankEntry item ;
						item.nOffset = sitDownPlayer->getCoin() - sitDownPlayer->getInitCoin() ;
						item.nUserUID = sitDownPlayer->getUserUID() ;
						vWillSend[item.nUserUID] = item ;
					}
					else
					{
						vWillSend[sitDownPlayer->getUserUID()].nOffset += (sitDownPlayer->getCoin() - sitDownPlayer->getInitCoin());
					}
				}
			}

			// send room info to player ;
			stMsgRequestRoomRankRet msgRet ;
			msgRet.nCnt = vWillSend.size() ;
			CAutoBuffer msgBuffer(sizeof(msgRet) + msgRet.nCnt * sizeof(stRoomRankEntry));
			msgBuffer.addContent(&msgRet,sizeof(msgRet));
			for ( auto& itemSendPlayer : vWillSend )
			{
				msgBuffer.addContent(&itemSendPlayer.second,sizeof(stRoomRankEntry));
			}
			sendMsgToPlayer((stMsg*)msgBuffer.getBufferPtr(),msgBuffer.getContentSize(),nPlayerSessionID) ;
		}
		break;
	default:
		return false;
	}
	return true ;
}