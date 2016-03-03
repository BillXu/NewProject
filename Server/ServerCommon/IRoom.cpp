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
#include "ServerStringTable.h"
#include <json/json.h>
#define TIME_SECONDS_PER_DAY (60*60*24)
#define TIME_SAVE_ROOM_INFO 60*30
#define TIME_DURING_ROOM_CLOSE 60*30
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
	memset(m_vRoomName,0,sizeof(m_vRoomName)) ;
	m_nTotalProfit = 0;
	m_nChatRoomID = 0;
	m_nConfigID = 0;
	m_bDirySorted = false ;

	m_bIsDelte = false ;

	m_bRoomInfoDiry = false ;

	memset(&m_stLimitConition,0,sizeof(m_stLimitConition)) ;
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

	MAP_UID_STAND_PLAYER::iterator iter = m_vInRoomPlayers.begin() ;
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

	removeAllRankItemPlayer();
}

void IRoom::removeAllRankItemPlayer()
{
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

void IRoom::onRoomClosed()
{
	m_nOpenTime = time(nullptr) + TIME_DURING_ROOM_CLOSE ; // half an hour later reopen ;
	m_bRoomInfoDiry = true;
	CLogMgr::SharedLogMgr()->SystemLog("room id = %d closed",getRoomID());

	// tong ji guanjun 
	uint32_t nChampionUID = 0;
	sortRoomRankItem();
	auto champ = getSortRankItemListBegin();
	stRoomRankItem* pPlayerChampion = nullptr ;
	if ( champ != getSortRankItemListEnd() )
	{
		pPlayerChampion = *champ ;
		nChampionUID = pPlayerChampion->nUserUID ;
	}

	if ( nChampionUID == 0 )
	{
		return ;
	}

	// save log ;
	stMsgSaveLog msgLog ;
	msgLog.nJsonExtnerLen = 0 ;
	msgLog.nLogType = eLog_MatchResult ;
	msgLog.nTargetID = getRoomID() ;
	memset(msgLog.vArg,0,sizeof(msgLog.vArg)) ;
	msgLog.vArg[0] = getRoomType() ;
	msgLog.vArg[1] = nChampionUID ;
	if ( pPlayerChampion != nullptr )
	{
		msgLog.vArg[2] = pPlayerChampion->nGameOffset ;
	}
	msgLog.vArg[3] = getProfit() ;
	sendMsgToPlayer(&msgLog,sizeof(msgLog),getRoomID()) ;
	if ( pPlayerChampion == nullptr )
	{
		CLogMgr::SharedLogMgr()->ErrorLog("room id = %d , type = %d , have no champion",getRoomID(),getRoomType()) ;
		return ;
	}

	// send reward inform ;
	stMsgCrossServerRequest msgReq ;
	msgReq.cSysIdentifer = ID_MSG_PORT_DATA ;
	msgReq.nReqOrigID = getRoomID();
	msgReq.nTargetID = nChampionUID;
	msgReq.nRequestType = eCrossSvrReq_Inform ;
	msgReq.nRequestSubType = eCrossSvrReqSub_Default ;

	std::string str = CServerStringTable::getInstance()->getStringByID(1) ;
	char pBuffer[200] = {0} ;
	sprintf_s(pBuffer,sizeof(pBuffer),str.c_str(),getRoomName()) ;
	msgReq.nJsonsLen = strlen(pBuffer) ;
	CAutoBuffer aub(sizeof(msgReq) + msgReq.nJsonsLen );
	aub.addContent(&msgReq,sizeof(msgReq)) ;
	aub.addContent(pBuffer,msgReq.nJsonsLen) ;
	sendMsgToPlayer((stMsg*)aub.getBufferPtr(),aub.getContentSize(),0) ;
}

void IRoom::onRoomOpened()
{
	CLogMgr::SharedLogMgr()->SystemLog("room id = %d opened",getRoomID());
	// reset room profit ;
	addTotoalProfit(getProfit());
	setProfit(0);
	// reset all history in db;
	stMsgRemoveRoomPlayer msgPlayer ;
	msgPlayer.nRoomID = getRoomID();
	msgPlayer.nRoomType = getRoomType();
	sendMsgToPlayer(&msgPlayer,sizeof(msgPlayer),0) ;

	// remove all histroy 
	removeAllRankItemPlayer();
}

void IRoom::onRoomWillDoDelete()
{
	LIST_STAND_PLAYER vAllInRoomPlayers ;
	auto bGin = m_vInRoomPlayers.begin() ;
	while (bGin != m_vInRoomPlayers.end())
	{
		vAllInRoomPlayers.push_back(bGin->second) ;
		++bGin ;
	}

	for ( auto& p : vAllInRoomPlayers )
	{
		onPlayerWillLeaveRoom(p) ;
		playerDoLeaveRoom(p);
	}
}

bool IRoom::init(stBaseRoomConfig* pConfig, uint32_t nRoomID, Json::Value& vJsValue )
{
	m_nRoomID = nRoomID ;
	m_nConfigID = pConfig->nConfigID ;
	m_nDeskFree = pConfig->nDeskFee ;
	m_nCreateTime = time(nullptr) ;
	m_nChatRoomID = 0;
	m_nCurProfit = 0 ;
	m_nTotalProfit = 0 ;
	m_nDeadTime = 0 ;
	m_nOpenTime = m_nCreateTime ;
	m_isOmitNewPlayerHalo = false ;
	m_nRoomOwnerUID = 0 ;

	memset(&m_stLimitConition,0,sizeof(m_stLimitConition)) ;
	
	if ( !vJsValue["ownerUID"].isNull() )
	{
		m_nRoomOwnerUID = vJsValue["ownerUID"].asUInt();
	}
	
	if (  !vJsValue["openTime"].isNull()  )
	{
		m_nOpenTime = vJsValue["openTime"].asUInt() ;
	}
	
	m_nDuringTime = 30*60 ;
	if ( !vJsValue["duringTime"].isNull() )
	{
		m_nDuringTime = vJsValue["duringTime"].asUInt() ;
	}
	
	if ( !vJsValue["isOmitHalo"].isNull() )
	{
		m_isOmitNewPlayerHalo = vJsValue["isOmitHalo"].asBool() ;
	}

	if ( !vJsValue["deadTime"].isNull() )
	{
		m_nDeadTime = vJsValue["deadTime"].asUInt() ;
		if ( m_nDeadTime < ( m_nOpenTime + m_nDuringTime ) )
		{
			m_nDeadTime = ( m_nOpenTime + m_nDuringTime );
		}
	}

	memset(m_vRoomName,0,sizeof(m_vRoomName));
	if ( !vJsValue["name"].isNull() )
	{
		sprintf_s(m_vRoomName,sizeof(m_vRoomName),"%s",vJsValue["name"].asCString()) ;
	}

	prepareState();
	return true ;
}

void IRoom::prepareState()
{
	// create room state ;
	IRoomState* vState[] = {
		new IRoomStateWaitPlayerJoin(),new IRoomStateClosed(),new IRoomStateDidGameOver(),
		new IRoomStateDead()
	};
	for ( uint8_t nIdx = 0 ; nIdx < sizeof(vState) / sizeof(IRoomState*); ++nIdx )
	{
		addRoomState(vState[nIdx]) ;
	}
	
	setInitState(vState[0]);
}

void IRoom::serializationFromDB(uint32_t nRoomID , Json::Value& vJsValue )
{
	m_nRoomID = nRoomID ;
	m_strRewardDesc = vJsValue["rewardDesc"].asCString();
	memset(m_vRoomName,0,sizeof(m_vRoomName));
	sprintf_s(m_vRoomName,sizeof(m_vRoomName),"%s",vJsValue["name"].asCString()) ;
	m_nRoomOwnerUID = vJsValue["ownerUID"].asUInt();
	m_nCreateTime = vJsValue["createTime"].asUInt() ;
	m_nDeadTime = vJsValue["deadTime"].asUInt() ;
	m_nOpenTime = vJsValue["openTime"].asUInt() ;
	m_nDuringTime = vJsValue["duringTime"].asUInt() ;
	m_nCurProfit = vJsValue["curProfit"].asUInt() ;
	m_nTotalProfit = vJsValue["totalProfit"].asUInt() ;
	m_nChatRoomID = vJsValue["chatId"].asUInt() ;
	m_nConfigID = vJsValue["configID"].asUInt();
	m_isOmitNewPlayerHalo = vJsValue["isOmitHalo"].asBool() ;
	m_nDeskFree = vJsValue["deskFee"].asUInt() ;

	stMsgReadRoomPlayer msgReadRoomPlayerData ;
	msgReadRoomPlayerData.nRoomID = nRoomID ;
	msgReadRoomPlayerData.nRoomType = getRoomType() ;
	sendMsgToPlayer(&msgReadRoomPlayerData,sizeof(msgReadRoomPlayerData),nRoomID) ;
	CLogMgr::SharedLogMgr()->PrintLog("read room player data uid = %d",nRoomID ) ;
	prepareState();
}

void IRoom::serializationToDB( bool bIsNewCreate )
{
	CLogMgr::SharedLogMgr()->ErrorLog("test stage do not store room info");
	return ;
	Json::StyledWriter jsWrite ;
	Json::Value vValue ;
	willSerializtionToDB(vValue) ;
	std::string strJson = jsWrite.write(vValue);

	stMsgSaveUpdateRoomInfo msgSave ;
	msgSave.bIsNewCreate = bIsNewCreate ;
	msgSave.nRoomType = getRoomType() ;
	msgSave.nRoomID = getRoomID() ;
	msgSave.nJsonLen = strJson.size() ;
	msgSave.nRoomOwnerUID = m_nRoomOwnerUID ;

	CAutoBuffer autoBuffer(sizeof(msgSave) + msgSave.nJsonLen);
	autoBuffer.addContent((char*)&msgSave,sizeof(msgSave)) ;
	autoBuffer.addContent(strJson.c_str(),msgSave.nJsonLen) ;
	sendMsgToPlayer((stMsg*)autoBuffer.getBufferPtr(),autoBuffer.getContentSize(),0) ;

	m_bRoomInfoDiry = false ;
}

void IRoom::willSerializtionToDB(Json::Value& vOutJsValue)
{
	vOutJsValue["roomType"] = getRoomType() ;
	vOutJsValue["name"] = getRoomName() ;
	vOutJsValue["rewardDesc"] = getRewardDesc() ;
	vOutJsValue["ownerUID"] = getOwnerUID() ; 
	vOutJsValue["createTime"] = getCreateTime();
	vOutJsValue["deadTime"] = getDeadTime() ;
	vOutJsValue["openTime"] = getOpenTime();
	vOutJsValue["duringTime"] = m_nDuringTime ;
	vOutJsValue["curProfit"] = (uint32_t)m_nCurProfit ;
	vOutJsValue["totalProfit"] = (uint32_t) m_nTotalProfit ;
	vOutJsValue["chatId"] = m_nChatRoomID ;
	vOutJsValue["configID"] = getConfigID() ;
	vOutJsValue["isOmitHalo"] = m_isOmitNewPlayerHalo ;
	vOutJsValue["deskFee"] = m_nDeskFree ;
}

uint8_t IRoom::canPlayerEnterRoom( stEnterRoomData* pEnterRoomPlayer )  // return 0 means ok ;
{
	stStandPlayer* pp = getPlayerByUserUID(pEnterRoomPlayer->nUserUID);
	if ( pp )
	{
		CLogMgr::SharedLogMgr()->ErrorLog("player uid = %d , already in this room, can not enter twice",pEnterRoomPlayer->nUserUID) ;
		return 1;
	}

	if ( m_stLimitConition.isRegisted )
	{
		if ( pEnterRoomPlayer->isRegisted == false )
		{
			CLogMgr::SharedLogMgr()->PrintLog("player is visitor , son can not enter room , uid = %d",pEnterRoomPlayer->nUserUID);
			return 2 ; // not register player  can not enter ;
		}
	}

	if ( m_stLimitConition.nCoinLowLimit > pEnterRoomPlayer->nCoin )
	{
		CLogMgr::SharedLogMgr()->PrintLog("player coin is too few so can not enter room , uid = %d",pEnterRoomPlayer->nUserUID);
		return 3 ; // player coin is too few ;
	}

	if ( m_stLimitConition.nCoinUpLimit && m_stLimitConition.nCoinUpLimit < pEnterRoomPlayer->nCoin )
	{
		CLogMgr::SharedLogMgr()->PrintLog("player coin is too many, so can not enter room , uid = %d",pEnterRoomPlayer->nUserUID);
		return 4 ; // player coin is too many ;
	}
	return 0 ;
}

void IRoom::onPlayerEnterRoom(stEnterRoomData* pEnterRoomPlayer )
{
	stStandPlayer* pp = getPlayerByUserUID(pEnterRoomPlayer->nUserUID);
	stStandPlayer * pStandPlayer = nullptr ;
	if ( pp )
	{
		CLogMgr::SharedLogMgr()->ErrorLog("player uid = %d , already in this room, can not enter twice, data svr crashed ?",pEnterRoomPlayer->nUserUID) ;
		pStandPlayer = pp ;
	}
	else
	{
		pStandPlayer = new stStandPlayer ;
		memset(pStandPlayer,0,sizeof(stStandPlayer));
	}

	memcpy(pStandPlayer,pEnterRoomPlayer,sizeof(stEnterRoomData));
	addRoomPlayer(pStandPlayer) ;

	sendRoomInfoToPlayer(pEnterRoomPlayer->nUserSessionID) ;
}

void IRoom::onPlayerWillLeaveRoom(stStandPlayer* pPlayer )
{
	CLogMgr::SharedLogMgr()->PrintLog("player uid = %d , will leave room process this function",pPlayer->nUserUID);
}

void IRoom::playerDoLeaveRoom( stStandPlayer* pp )
{
	// send msg to data svr tell player leave room ;
	if ( pp )
	{
		stMsgSvrDoLeaveRoom msgdoLeave ;
		msgdoLeave.nCoin = pp->nCoin ;
		msgdoLeave.nGameType = getRoomType() ;
		msgdoLeave.nRoomID = getRoomID() ;
		msgdoLeave.nNewPlayerHaloWeight = pp->nNewPlayerHaloWeight ;
		msgdoLeave.nUserUID = pp->nUserUID ;
		msgdoLeave.nWinTimes = pp->nWinTimes ;
		msgdoLeave.nPlayerTimes = pp->nPlayerTimes ;
		msgdoLeave.nSingleWinMost = pp->nSingleWinMost ;
		sendMsgToPlayer(&msgdoLeave,sizeof(msgdoLeave),pp->nUserSessionID) ;

		removePlayer(pp);
		CLogMgr::SharedLogMgr()->PrintLog("uid = %d , do leave this room ",msgdoLeave.nUserUID ) ;
	}
	else
	{
		CLogMgr::SharedLogMgr()->ErrorLog("player , not in this room can not do leave room" ) ;
	}
	
}

uint32_t IRoom::getRoomID()
{
	return m_nRoomID ;
}

void IRoom::update(float fDelta)
{
	m_pCurRoomState->update(fDelta);
}

bool IRoom::addRoomPlayer(stStandPlayer* pPlayer )
{
	assert(pPlayer&&"pPlayer is null") ;
	if ( isPlayerInRoom(pPlayer) )
	{
		return false;
	}
	m_vInRoomPlayers[pPlayer->nUserUID] = pPlayer ;
	return true ;
}

void IRoom::removePlayer(stStandPlayer* pPlayer )
{
	STAND_PLAYER_ITER iter = m_vInRoomPlayers.find(pPlayer->nUserUID) ;
	if ( iter != m_vInRoomPlayers.end() )
	{
		m_vReseverPlayerObjects.push_back(pPlayer);
		m_vInRoomPlayers.erase(iter) ;
	}
}

IRoom::stStandPlayer* IRoom::getPlayerByUserUID(uint32_t nUserUID )
{
	STAND_PLAYER_ITER iter = m_vInRoomPlayers.find(nUserUID) ;
	if ( iter != m_vInRoomPlayers.end() )
	{
		return iter->second ;
	}
	return nullptr ;
}

IRoom::stStandPlayer* IRoom::getPlayerBySessionID(uint32_t nSessionID )
{
	STAND_PLAYER_ITER iter = m_vInRoomPlayers.begin();
	for ( ; iter != m_vInRoomPlayers.end(); ++iter )
	{
		if ( iter->second->nUserSessionID == nSessionID )
		{
			return iter->second ;
		}
	}
	return nullptr ;
}

bool IRoom::isPlayerInRoom(stStandPlayer* pPlayer )
{
	return isPlayerInRoomWithUserUID(pPlayer->nUserUID);
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

IRoom::STAND_PLAYER_ITER IRoom::beginIterForPlayers()
{
	return m_vInRoomPlayers.begin() ;
}

IRoom::STAND_PLAYER_ITER IRoom::endIterForPlayers()
{
	return m_vInRoomPlayers.end() ;
}

void IRoom::updatePlayerOffset(uint32_t nUserUID , int64_t nOffsetGame, int64_t nOtherOffset )
{
	if ( nOffsetGame == 0 && nOtherOffset == 0 )
	{
		return ;
	}

	m_bDirySorted = true ;

	auto targ = m_vRoomRankHistroy.find(nUserUID) ;
	if ( targ != m_vRoomRankHistroy.end() )
	{
		targ->second->nGameOffset += nOffsetGame ;
		targ->second->nOtherOffset += nOtherOffset ;
		targ->second->bIsDiryt = true ;
		CLogMgr::SharedLogMgr()->PrintLog("uid = %d update offset = %lld , final = %lld",nUserUID,nOffsetGame,targ->second->nGameOffset) ;
		return ;
	}
	stRoomRankItem* p = new stRoomRankItem ;
	p->nUserUID = nUserUID ;
	p->nGameOffset = nOffsetGame ;
	p->nOtherOffset = nOtherOffset ;
	p->bIsDiryt = false ;
	m_vRoomRankHistroy[p->nUserUID] = p ;
	m_vSortedRankItems.push_back(p) ;

	CLogMgr::SharedLogMgr()->PrintLog("uid = %d update offset = %lld , final = %lld",nUserUID,nOffsetGame,nOffsetGame) ;
	// send msg to db  add this peer ;
	stMsgSaveRoomPlayer msgSave ;
	msgSave.isUpdate = false ;
	msgSave.nRoomID = getRoomID() ;
	msgSave.nRoomType = getRoomType() ;
	msgSave.savePlayer.nGameOffset = nOffsetGame ;
	msgSave.savePlayer.nOtherOffset = nOtherOffset;
	msgSave.savePlayer.nUserUID = p->nUserUID ;
	sendMsgToPlayer(&msgSave,sizeof(msgSave),getRoomID());
	return ;
}

bool sortFuncRankItem(IRoom::stRoomRankItem* pLeft , IRoom::stRoomRankItem* pRight )
{
	return (pLeft->nGameOffset + pLeft->nOtherOffset) >= (pRight->nGameOffset + pRight->nOtherOffset) ;
}

void IRoom::sortRoomRankItem()
{
	if ( m_bDirySorted && m_vSortedRankItems.size() >= 2 )
	{
		m_vSortedRankItems.sort(sortFuncRankItem);
	}
	m_bDirySorted = false ;
}

void IRoom::sendRoomMsg( stMsg* pmsg , uint16_t nLen )
{
	STAND_PLAYER_ITER iter = m_vInRoomPlayers.begin() ;
	for ( ; iter != m_vInRoomPlayers.end() ; ++iter )
	{
		sendMsgToPlayer(pmsg,nLen,iter->second->nUserSessionID) ;
	}
}

bool IRoom::onCrossServerRequest(stMsgCrossServerRequest* pRequest , eMsgPort eSenderPort,Json::Value* vJsValue )
{
	switch ( pRequest->nRequestType )
	{
	case eCrossSvrReq_RoomProfit:
		{
			stMsgCrossServerRequestRet msgRet ;
			msgRet.cSysIdentifer = ID_MSG_PORT_DATA ;
			msgRet.nReqOrigID = pRequest->nTargetID ;
			msgRet.nTargetID = pRequest->nReqOrigID ;
			msgRet.nRequestType = pRequest->nRequestType ;
			msgRet.nRequestSubType = pRequest->nRequestSubType ;
			msgRet.nRet = 0 ;
			msgRet.vArg[0] = true ;
			msgRet.vArg[1] = m_nCurProfit ;
			msgRet.vArg[2] = eRoom_TexasPoker ;
			m_nTotalProfit += m_nCurProfit ;
			m_nCurProfit = 0 ;
			sendMsgToPlayer(&msgRet,sizeof(msgRet),msgRet.nTargetID ) ;
		}
		break;
	default:
		return false;
	}
	return true ;
}

bool IRoom::onMessage( stMsg* prealMsg , eMsgPort eSenderPort , uint32_t nPlayerSessionID )
{
	if ( m_pCurRoomState && m_pCurRoomState->onMessage(prealMsg,eSenderPort,nPlayerSessionID) )
	{
		return true ;
	}

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
				p->nGameOffset = pp->nGameOffset ;
				p->nOtherOffset = pp->nOtherOffset ;
				m_vRoomRankHistroy[p->nUserUID] = p ;
				m_vSortedRankItems.push_back(p) ;
				++pp;
			}
		}
		break;
	case MSG_PLAYER_LEAVE_ROOM:
		{
			stMsgPlayerLeaveRoomRet msg ;
			stStandPlayer* pp = getPlayerBySessionID(nPlayerSessionID) ;
			if ( pp )
			{
				onPlayerWillLeaveRoom(pp) ;
				playerDoLeaveRoom(pp);
				msg.nRet = 0 ;
			}
			else
			{
				msg.nRet = 1 ;
				CLogMgr::SharedLogMgr()->ErrorLog("session id not in this room how to leave session id = %d",nPlayerSessionID) ;
			}
			sendMsgToPlayer(&msg,sizeof(msg),nPlayerSessionID) ;
		}
		break;
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
				entry.nGameOffset = pItem->nGameOffset ;
				entry.nOtherOffset = pItem->nOtherOffset ;
				entry.nUserUID = pItem->nUserUID ;
				vWillSend[pItem->nUserUID] = entry ;
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
			msgSave.savePlayer.nGameOffset = ranData->nGameOffset;
			msgSave.savePlayer.nOtherOffset = ranData->nOtherOffset ;
			msgSave.savePlayer.nUserUID = ranData->nUserUID ;
			sendMsgToPlayer(&msgSave,sizeof(msgSave),getRoomID());
			CLogMgr::SharedLogMgr()->PrintLog("time save room player data room id = %d  , uid = %d",getRoomID(),ranData->nUserUID);
		}
	}

	if ( m_bRoomInfoDiry )
	{
		m_bRoomInfoDiry = false ;
		serializationToDB();
	}

}

void IRoom::goToState(IRoomState* pTargetState )
{
	//assert(pTargetState != m_pCurRoomState && "go to the same state ? " );
	if ( pTargetState == m_pCurRoomState)
	{
		CLogMgr::SharedLogMgr()->SystemLog("go to the same state %d , room id = %d ? ",pTargetState->getStateID(), getRoomID() );
	}
	
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

void IRoom::setOwnerUID(uint32_t nCreatorUID )
{
	m_nRoomOwnerUID = nCreatorUID ;
}

uint32_t IRoom::getOwnerUID()
{
	return m_nRoomOwnerUID ;
}

void IRoom::addLiftTime(uint32_t nMinites )
{
	time_t tNow = time(nullptr) ;
	if ( tNow > m_nDeadTime )
	{
		m_nDeadTime = tNow ;
	}
	m_nDuringTime += 60*nMinites ;
	if ( m_nDeadTime != 0 )
	{
		m_nDeadTime += 60*nMinites ;
	}
	m_bRoomInfoDiry = true ;
}

void IRoom::setDeadTime(uint32_t nDeadTime)
{
	m_nDeadTime = nDeadTime ;
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

void IRoom::setRewardDesc(const char* pRewardDesc )
{
	if ( pRewardDesc == nullptr || strlen(pRewardDesc) >= MAX_LEN_ROOM_DESC )
	{
		CLogMgr::SharedLogMgr()->ErrorLog("too long proom Inform ");
		return ;
	}
	m_strRewardDesc = pRewardDesc ;
	m_bRoomInfoDiry = true ;
}

bool IRoom::isRoomAlive()
{
	if ( m_nDeadTime == 0 )
	{
		return true ;
	}

	return time(NULL) <= m_nDeadTime && m_bIsDelte == false;
}

void IRoom::setProfit(uint64_t nProfit )
{
	m_nCurProfit = nProfit ;
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

void IRoom::setChatRoomID(uint32_t nChatRoomID )
{
	m_nChatRoomID = nChatRoomID ;
	m_bRoomInfoDiry = true ;
}

uint32_t IRoom::getConfigID()
{
	return m_nConfigID ;
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

void IRoom::deleteRoom()
{
	m_bIsDelte = true ;
	m_nDuringTime = 0 ;
	m_nDeadTime = 1 ; // right now 
}

bool IRoom::isDeleteRoom()
{
	return m_bIsDelte ;
}

void IRoom::debugRank()
{
	int64_t nFinal = 0 ;
	CLogMgr::SharedLogMgr()->SystemLog("debug rank room id = %d ",getRoomID());
	m_vSortedRankItems;
	for ( auto& rank : m_vSortedRankItems )
	{
		CLogMgr::SharedLogMgr()->SystemLog("uid = %d, offset = %lld , room id = %d",rank->nUserUID,rank->nGameOffset,getRoomID() ) ;
		nFinal += rank->nGameOffset ;
	}

	if ( nFinal != 0  )
	{
		CLogMgr::SharedLogMgr()->ErrorLog("rank error not equal room id = %d",getRoomID()) ;
	}
}