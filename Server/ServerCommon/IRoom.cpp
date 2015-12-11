#include "IRoom.h"
#include <cassert>
#include "IRoomPlayer.h"
#include "IRoomState.h"
IRoom::IRoom()
{
	m_nRoomID = 0 ;
	m_vReseverPlayerObjects.clear();
	m_vInRoomPlayers.clear();
	m_vRoomStates.clear();
	m_pCurRoomState = nullptr ;
}

IRoom::~IRoom()
{
	for ( auto pPlayer : m_vReseverPlayerObjects )
	{
			if ( pPlayer )
			{
				delete pPlayer ;
				pPlayer = nullptr ;
			}
	}
	m_vReseverPlayerObjects.clear() ;

	MAP_UID_ROOM_PLAYER::iterator iter = m_vInRoomPlayers.begin() ;
	for ( ; iter != m_vInRoomPlayers.end(); ++iter )
	{
		if ( iter->second )
		{
			delete iter->second ;
			iter->second = nullptr ;
		}
	}
	m_vInRoomPlayers.clear() ;

	MAP_ID_ROOM_STATE::iterator idState = m_vRoomStates.begin() ;
	for ( ; idState != m_vRoomStates.end() ; ++idState )
	{
		delete idState->second ;
		idState->second = nullptr ;
	}
	m_vRoomStates.clear() ;
}

bool IRoom::init(stBaseRoomConfig* pConfig, uint32_t nRoomID )
{
	m_nRoomID = nRoomID ;
	return true ;
}

uint32_t IRoom::getRoomID()
{
	return m_nRoomID ;
}

void IRoom::update(float fDelta)
{
	m_pCurRoomState->update(fDelta);
}

bool IRoom::addRoomPlayer(IRoomPlayer* pPlayer )
{
	assert(pPlayer&&"pPlayer is null") ;
	if ( isPlayerInRoom(pPlayer) )
	{
		return false;
	}
	m_vInRoomPlayers[pPlayer->getUserUID()] = pPlayer ;
	return true ;
}

void IRoom::removePlayer(IRoomPlayer* pPlayer )
{
	PLAYER_ITER iter = m_vInRoomPlayers.find(pPlayer->getUserUID()) ;
	if ( iter != m_vInRoomPlayers.end() )
	{
		m_vReseverPlayerObjects.push_back(pPlayer);
		m_vInRoomPlayers.erase(iter) ;
	}
}

IRoomPlayer* IRoom::getPlayerByUserUID(uint32_t nUserUID )
{
	PLAYER_ITER iter = m_vInRoomPlayers.find(nUserUID) ;
	if ( iter != m_vInRoomPlayers.end() )
	{
		return iter->second ;
	}
	return nullptr ;
}

IRoomPlayer* IRoom::getPlayerBySessionID(uint32_t nSessionID )
{
	PLAYER_ITER iter = m_vInRoomPlayers.begin();
	for ( ; iter != m_vInRoomPlayers.end(); ++iter )
	{
		if ( iter->second->getSessionID() == nSessionID )
		{
			return iter->second ;
		}
	}
	return nullptr ;
}

bool IRoom::isPlayerInRoom(IRoomPlayer* pPlayer )
{
	return isPlayerInRoomWithUserUID(pPlayer->getUserUID());
}

bool IRoom::isPlayerInRoomWithSessionID(uint32_t nSessioID )
{
	return getPlayerBySessionID(nSessioID) != nullptr ;
}

bool IRoom::isPlayerInRoomWithUserUID(uint32_t nUserUID )
{
	return getPlayerByUserUID(nUserUID) != nullptr ;
}

uint16_t IRoom::getPlayerCount()
{
	return m_vInRoomPlayers.size() ;
}

IRoomPlayer* IRoom::getReusePlayerObject()
{
	LIST_ROOM_PLAYER::iterator iter = m_vReseverPlayerObjects.begin() ;
	if ( iter != m_vReseverPlayerObjects.end() )
	{
		IRoomPlayer* p = *iter ;
		m_vReseverPlayerObjects.erase(iter) ;
		return p ;
	}
	
	return doCreateRoomPlayerObject();
}

IRoom::PLAYER_ITER IRoom::beginIterForPlayers()
{
	return m_vInRoomPlayers.begin() ;
}

IRoom::PLAYER_ITER IRoom::endIterForPlayers()
{
	return m_vInRoomPlayers.end() ;
}

void IRoom::sendRoomMsg( stMsg* pmsg , uint16_t nLen )
{
	PLAYER_ITER iter = m_vInRoomPlayers.begin() ;
	for ( ; iter != m_vInRoomPlayers.end() ; ++iter )
	{
		sendMsgToPlayer(pmsg,nLen,iter->second->getSessionID()) ;
	}
}

bool IRoom::onMessage( stMsg* prealMsg , eMsgPort eSenderPort , uint32_t nPlayerSessionID )
{
	if ( m_pCurRoomState->onMessage(prealMsg,eSenderPort,nPlayerSessionID) )
	{
		return true ;
	}
	return false ;
}

void IRoom::onTimeSave()
{

}

void IRoom::goToState(IRoomState* pTargetState )
{
	assert(pTargetState != m_pCurRoomState && "go to the same state ? " );
	m_pCurRoomState->leaveState() ;
	m_pCurRoomState = pTargetState ;
	m_pCurRoomState->enterState(this) ;
}

void IRoom::goToState( uint16_t nStateID )
{
	goToState(getRoomStateByID(nStateID)) ;
}

void IRoom::setInitState(IRoomState* pDefaultState )
{
	m_pCurRoomState = pDefaultState ;
}

IRoomState* IRoom::getCurRoomState()
{
	return m_pCurRoomState ;
}

IRoomState* IRoom::getRoomStateByID(uint16_t nStateID )
{
	MAP_ID_ROOM_STATE::iterator iter = m_vRoomStates.find(nStateID) ;
	if ( iter != m_vRoomStates.end() )
	{
		return iter->second ;
	}
	return nullptr ;
}

bool IRoom::addRoomState(IRoomState* pRoomState )
{
	assert(getRoomStateByID(pRoomState->getStateID()) == nullptr && "already added this state" ) ;
	m_vRoomStates[pRoomState->getStateID()] = pRoomState ;
	return true ;
}