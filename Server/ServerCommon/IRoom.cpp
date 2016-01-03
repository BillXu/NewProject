#include "IRoom.h"
#include <cassert>
#include "IRoomPlayer.h"
#include "IRoomState.h"
#include <time.h>
#include "AutoBuffer.h"
#include "ServerDefine.h"
#include "ServerMessageDefine.h"
#include "LogManager.h"
#include "RoomConfig.h"
#include <algorithm>
#define TIME_SECONDS_PER_DAY (60*60*24)
#define TIME_SAVE_ROOM_INFO 60*30
IRoom::IRoom()
{
	m_nRoomID = 0 ;
	m_vReseverPlayerObjects.clear();
	m_vInRoomPlayers.clear();
	m_vRoomStates.clear();
	m_pCurRoomState = nullptr ;

	m_nRoomOwnerUID = 0 ;
	m_nCreateTime = 0 ;
	m_nDeadTime = 0;
	m_nAvataID = 0 ;
	memset(m_vRoomName,0,sizeof(m_vRoomName)) ;
	m_strRoomInForm = "nll n";
	m_nInformSerial = 0;
	m_nRoomProfit = 0;
	m_nTotalProfit = 0;
	m_nChatRoomID = 0;
	m_nConfigID = 0;
	m_bDirySorted = false ;

	m_bRoomInfoDiry = false ;
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

	auto mapIter = m_vRoomRankHistroy.begin() ;
	for ( ; mapIter != m_vRoomRankHistroy.end() ; ++mapIter )
	{
		if ( mapIter->second )
		{
			delete mapIter->second ;
			mapIter->second = nullptr ;
		}
	}
	m_vRoomRankHistroy.clear() ;
	m_vSortedRankItems.clear() ;
}

bool IRoom::init(stBaseRoomConfig* pConfig, uint32_t nRoomID )
{
	m_nRoomID = nRoomID ;
	m_nConfigID = pConfig->nConfigID ;
	
	stMsgReadRoomPlayer msgReadRoomPlayerData ;
	msgReadRoomPlayerData.nRoomID = nRoomID ;
	msgReadRoomPlayerData.nRoomType = getRoomType() ;
	sendMsgToPlayer(&msgReadRoomPlayerData,sizeof(msgReadRoomPlayerData),nRoomID) ;
	CLogMgr::SharedLogMgr()->PrintLog("read room player data uid = %d",nRoomID ) ;
	m_bRoomInfoDiry = false ;
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

void IRoom::updatePlayerOffset(uint32_t nUserUID , int64_t nOffsetThisOnce )
{
	if ( nOffsetThisOnce == 0 )
	{
		return ;
	}

	m_bDirySorted = true ;

	auto targ = m_vRoomRankHistroy.find(nUserUID) ;
	if ( targ != m_vRoomRankHistroy.end() )
	{
		targ->second->nOffset += nOffsetThisOnce ;
		targ->second->bIsDiryt = true ;
		return ;
	}
	stRoomRankItem* p = new stRoomRankItem ;
	p->nUserUID = nUserUID ;
	p->nOffset = nOffsetThisOnce ;
	p->bIsDiryt = false ;
	p->nWinTimes = 0 ;
	p->nPlayerTimes = 0 ;
	m_vRoomRankHistroy[p->nUserUID] = p ;
	m_vSortedRankItems.push_back(p) ;

	// send msg to db  add this peer ;
	stMsgSaveRoomPlayer msgSave ;
	msgSave.isUpdate = false ;
	msgSave.nRoomID = getRoomID() ;
	msgSave.nRoomType = getRoomType() ;
	msgSave.savePlayer.nOffset = nOffsetThisOnce ;
	msgSave.savePlayer.nUserUID = p->nUserUID ;
	msgSave.savePlayer.nPlayerTimes = p->nPlayerTimes ;
	msgSave.savePlayer.nWinTimes = p->nWinTimes ;
	sendMsgToPlayer(&msgSave,sizeof(msgSave),getRoomID());
	return ;
}

bool sortFuncRankItem(IRoom::stRoomRankItem* pLeft , IRoom::stRoomRankItem* pRight )
{
	return pLeft->nOffset >= pRight->nOffset ;
}

void IRoom::sortRoomRankItem()
{
	if ( m_bDirySorted )
	{
		m_vSortedRankItems.sort(sortFuncRankItem);
	}
	m_bDirySorted = false ;
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
	switch ( prealMsg->usMsgType )
	{
	case MSG_READ_ROOM_PLAYER:
		{
			stMsgReadRoomPlayerRet* pRet = (stMsgReadRoomPlayerRet*)prealMsg ; 
			CLogMgr::SharedLogMgr()->PrintLog("room id = %d recive room player data cnt = %d",getRoomID(),pRet->nCnt) ;
			stSaveRoomPlayerEntry* pp = (stSaveRoomPlayerEntry*)(((char*)prealMsg) + sizeof(stMsgReadRoomPlayerRet));
			m_bDirySorted = true ;
			while ( pRet->nCnt-- )
			{
				stRoomRankItem* p = new stRoomRankItem ;
				p->bIsDiryt = false ;
				p->nUserUID = pp->nUserUID ;
				p->nOffset = pp->nOffset ;
				p->nPlayerTimes = pp->nPlayerTimes ;
				p->nWinTimes = pp->nWinTimes ;
				m_vRoomRankHistroy[p->nUserUID] = p ;
				m_vSortedRankItems.push_back(p) ;
				++pp;
			}
		}
		break;
	default:
		if ( m_pCurRoomState->onMessage(prealMsg,eSenderPort,nPlayerSessionID) )
		{
			return true ;
		}
		return false ;
	}

	return true ;
}

void IRoom::onTimeSave( bool bRightNow )
{
	// save room player data 
	for ( auto& ranData : m_vSortedRankItems )
	{
		if ( ranData->bIsDiryt )
		{
			ranData->bIsDiryt = false ;

			stMsgSaveRoomPlayer msgSave ;
			msgSave.isUpdate = true ;
			msgSave.nRoomID = getRoomID() ;
			msgSave.nRoomType = getRoomType() ;
			msgSave.savePlayer.nOffset = ranData->nOffset ;
			msgSave.savePlayer.nUserUID = ranData->nUserUID ;
			msgSave.savePlayer.nPlayerTimes = ranData->nPlayerTimes ;
			msgSave.savePlayer.nWinTimes = ranData->nWinTimes ;
			sendMsgToPlayer(&msgSave,sizeof(msgSave),getRoomID());
			CLogMgr::SharedLogMgr()->PrintLog("time save room player data room id = %d  , uid = %d",getRoomID(),ranData->nUserUID);
		}
	}

	if ( m_bRoomInfoDiry )
	{
		m_bRoomInfoDiry = false ;
		stMsgSaveUpdateRoomInfo msgSave ;
		msgSave.nRoomType = getRoomType() ;
		msgSave.nAvataID = m_nAvataID ;
		msgSave.nDeadTime = m_nDeadTime ;
		msgSave.nInformSerial = m_nInformSerial ;
		msgSave.nRoomID = getRoomID() ;
		msgSave.nRoomProfit = m_nRoomProfit ;
		msgSave.nTotalProfit = m_nTotalProfit ;
		memset(msgSave.vRoomName,0,sizeof(msgSave.vRoomName));
		//sprintf(msgSave.vRoomDesc,"%s",m_strRoomDesc.c_str());
		sprintf_s(msgSave.vRoomName,sizeof(msgSave.vRoomName),"%s",m_vRoomName );
		msgSave.nInformLen = strlen(m_strRoomInForm.c_str());
		if ( msgSave.nInformLen == 0 )
		{
			sendMsgToPlayer(&msgSave,sizeof(msgSave),0) ;
			return ;
		}

		CAutoBuffer autoBuffer(sizeof(msgSave) + msgSave.nInformLen);
		autoBuffer.addContent((char*)&msgSave,sizeof(msgSave)) ;
		autoBuffer.addContent(m_strRoomInForm.c_str(),msgSave.nInformLen) ;
		sendMsgToPlayer((stMsg*)autoBuffer.getBufferPtr(),autoBuffer.getContentSize(),0) ;
	}

}

void IRoom::goToState(IRoomState* pTargetState )
{
	assert(pTargetState != m_pCurRoomState && "go to the same state ? " );
	m_pCurRoomState->leaveState() ;
	m_pCurRoomState = pTargetState ;
	m_pCurRoomState->enterState(this) ;

	stMsgRoomEnterNewState msgNewState ;
	msgNewState.m_fStateDuring = m_pCurRoomState->getStateDuring();
	msgNewState.nNewState = m_pCurRoomState->getStateID();
	sendRoomMsg(&msgNewState,sizeof(msgNewState)) ;
}

void IRoom::goToState( uint16_t nStateID )
{
	goToState(getRoomStateByID(nStateID)) ;
}

void IRoom::setInitState(IRoomState* pDefaultState )
{
	m_pCurRoomState = pDefaultState ;
	m_pCurRoomState->enterState(this);
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

void IRoom::onCreateByPlayer(uint32_t nUserUID, uint16_t nRentDays )
{
	setOwnerUID(nUserUID);
	m_nCreateTime = time(nullptr);
	m_nDeadTime = m_nCreateTime + TIME_SECONDS_PER_DAY*nRentDays ;
	m_bRoomInfoDiry = true ;
}

void IRoom::setOwnerUID(uint32_t nCreatorUID )
{
	m_nRoomOwnerUID = nCreatorUID ;
}

uint32_t IRoom::getOwnerUID()
{
	return m_nRoomOwnerUID ;
}

void IRoom::addLiftTime(uint32_t nDays )
{
	time_t tNow = time(nullptr) ;
	if ( tNow > m_nDeadTime )
	{
		m_nDeadTime = tNow ;
	}
	m_nDeadTime += TIME_SECONDS_PER_DAY*nDays ;
	m_bRoomInfoDiry = true ;
}

void IRoom::setDeadTime(uint32_t nDeadTime)
{
	m_nDeadTime = nDeadTime ;
	m_bRoomInfoDiry = true ;
}

void IRoom::setAvataID(uint32_t nAvaID )
{
	m_nAvataID = nAvaID ;
	m_bRoomInfoDiry = true ;
}

void IRoom::setRoomName(const char* pRoomName)
{
	if ( pRoomName == nullptr || strlen(pRoomName) >= MAX_LEN_ROOM_NAME )
	{
		CLogMgr::SharedLogMgr()->ErrorLog("too long proom name");
		return ;
	}

	memset(m_vRoomName,0,sizeof(m_vRoomName)) ;
	memcpy_s(m_vRoomName,MAX_LEN_ROOM_NAME,pRoomName,strlen(pRoomName));
	m_bRoomInfoDiry = true ;
}

const char* IRoom::getRoomName()
{
	return m_vRoomName ;
}

void IRoom::setRoomInform(const char* pRoomInform )
{
	if ( pRoomInform == nullptr || strlen(pRoomInform) >= MAX_LEN_ROOM_DESC )
	{
		CLogMgr::SharedLogMgr()->ErrorLog("too long proom Inform ");
		return ;
	}
	m_strRoomInForm = pRoomInform ;
	m_bRoomInfoDiry = true ;
}

bool IRoom::isRoomAlive()
{
	if ( m_nDeadTime == 0 )
	{
		return true ;
	}

	return time(NULL) <= m_nDeadTime ;
}

void IRoom::setProfit(uint64_t nProfit )
{
	m_nRoomProfit = nProfit ;
	m_bRoomInfoDiry = true ;
}

void IRoom::setCreateTime(uint32_t nTime)
{
	m_nCreateTime = nTime ;
	m_bRoomInfoDiry = true ;
}

uint32_t IRoom::getCreateTime()
{
	return m_nCreateTime ;
}

void IRoom::setInformSieral(uint32_t nSieaial)
{
	m_nInformSerial = nSieaial ;
	m_bRoomInfoDiry = true ;
}

void IRoom::setChatRoomID(uint64_t nChatRoomID )
{
	m_nChatRoomID = nChatRoomID ;
	m_bRoomInfoDiry = true ;
}

uint32_t IRoom::getConfigID()
{
	return m_nConfigID ;
}

uint32_t IRoom::getDeadTime()
{
	return m_nDeadTime ;
}

void IRoom::sendExpireInform()
{
	// if player requesting coin , do not sync data ;
	stMsgCrossServerRequest msgReq ;
	msgReq.cSysIdentifer = ID_MSG_PORT_DATA ;
	msgReq.nReqOrigID = getRoomID();
	msgReq.nTargetID = m_nRoomOwnerUID;
	msgReq.nRequestType = eCrossSvrReq_Inform ;
	msgReq.nRequestSubType = eCrossSvrReqSub_Default ;

	std::string str = "your room ( id = %d ) have expired !" ;
	char pBuffer[200] = {0} ;
	sprintf_s(pBuffer,sizeof(pBuffer),str.c_str(),getRoomID()) ;
	msgReq.nJsonsLen = strlen(pBuffer) ;
	CAutoBuffer aub(sizeof(msgReq) + msgReq.nJsonsLen );
	aub.addContent(&msgReq,sizeof(msgReq)) ;
	aub.addContent(pBuffer,msgReq.nJsonsLen) ;
	sendMsgToPlayer((stMsg*)aub.getBufferPtr(),aub.getContentSize(),0) ;
}