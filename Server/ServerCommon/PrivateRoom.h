#pragma once
#include "IRoomInterface.h"
#include "ISitableRoom.h"
#include "IRoomDelegate.h"
#include <cassert>
#include "ISitableRoomPlayer.h"
struct stBuyInEntry
{
	uint32_t nBuyInCoin ;
	time_t tBuyInTime ;
};

struct stPrivateRoomPlayerItem
{
	uint32_t nUserUID ;
	uint32_t nToTalCoin ;
	uint32_t nCoinInRoom ;
	uint32_t nToTalBuyIn ;
	std::vector<stBuyInEntry> vBuyInRecord ;

	bool isDirty ;

	stPrivateRoomPlayerItem(uint32_t nPlayerUID, uint32_t nAllCoin )
	{
		nUserUID = nPlayerUID ;
		nToTalCoin = nAllCoin ;
		nCoinInRoom = 0 ;
		nToTalBuyIn = 0 ;
		vBuyInRecord.clear() ;
		isDirty = false ;
	}

	void toJsvalue(Json::Value& jsValue )
	{
		jsValue["nUserUID"] = nUserUID ;
		jsValue["nToTalCoin"] = nToTalCoin ;
		jsValue["nToTalBuyIn"] = nToTalBuyIn ;
		jsValue["nCoinInRoom"] = nCoinInRoom ;

		Json::Value arrayBuEntry ;
		for ( auto& ref  : vBuyInRecord )
		{
			Json::Value entry ;
			entry["nBuyInCoin"] = ref.nBuyInCoin ;
			entry["tBuyInTime"] = (uint32_t)ref.tBuyInTime ;
			arrayBuEntry[arrayBuEntry.size()] = entry ;
		}

		if ( arrayBuEntry.empty() == false )
		{
			jsValue["vBuyInRecord"] = arrayBuEntry ;
		}
	}

	void fromjsValue( Json::Value& jsValue )
	{
		nUserUID = jsValue["nUserUID"].asUInt();
		nToTalCoin = jsValue["nToTalCoin"].asUInt();
		nToTalBuyIn = jsValue["nToTalBuyIn"].asUInt();
		nCoinInRoom = jsValue["nCoinInRoom"].asUInt();

		if ( jsValue["vBuyInRecord"].isNull() )
		{
			return ;
		}

		Json::Value vJsBuyRecorder  = jsValue["vBuyInRecord"] ;
		for ( uint16_t nIdx = 0 ; nIdx < vJsBuyRecorder.size() ; ++nIdx )
		{
			Json::Value jsEntry = vJsBuyRecorder[nIdx] ;

			stBuyInEntry entry ;
			entry.tBuyInTime = jsEntry["tBuyInTime"].asUInt() ;
			entry.nBuyInCoin = jsEntry["nBuyInCoin"].asUInt() ;
			vBuyInRecord.push_back(entry) ;
		}
	}
	
	bool buyIn(uint32_t nCoin )
	{
		if ( nToTalCoin < nCoin )
		{
			return false ;
		}

		nToTalCoin -= nCoin ;
		nToTalBuyIn += nCoin ;

		stBuyInEntry tEntry  ;
		tEntry.nBuyInCoin = nCoin ;
		tEntry.tBuyInTime = time(nullptr) ;
		vBuyInRecord.push_back(tEntry) ;
		isDirty = true ;
		return true ;
	}
};
template<class T >
class CPrivateRoom
	:public IRoomInterface
	,public IRoomDelegate
{
public:
	typedef T REAL_ROOM ;
	typedef T* REAL_ROOM_PTR;
public:
	CPrivateRoom();
	~CPrivateRoom();

	// interface 
	bool onFirstBeCreated(IRoomManager* pRoomMgr,stBaseRoomConfig* pConfig, uint32_t nRoomID, Json::Value& vJsValue )override;
	void serializationFromDB(IRoomManager* pRoomMgr,stBaseRoomConfig* pConfig,uint32_t nRoomID , Json::Value& vJsValue ) override;
	void serializationToDB() override;

	uint8_t canPlayerEnterRoom( stEnterRoomData* pEnterRoomPlayer )override;  // return 0 means ok ;
	void onPlayerEnterRoom(stEnterRoomData* pEnterRoomPlayer,int8_t& nSubIdx ) override;
	bool onPlayerApplyLeaveRoom(uint32_t nUserUID )override;
	void roomItemDetailVisitor(Json::Value& vOutJsValue) override;
	uint32_t getRoomID()override;
	uint8_t getRoomType() override ;
	void update(float fDelta) override;
	void onTimeSave() override;
	bool onMessage( stMsg* prealMsg , eMsgPort eSenderPort , uint32_t nPlayerSessionID )override;
	bool isDeleteRoom()override;
	void deleteRoom()override ;
	uint32_t getOwnerUID()override;

	// delegate msg ;
	uint8_t canPlayerEnterRoom( IRoom* pRoom,stEnterRoomData* pEnterRoomPlayer )override;  // return 0 means ok ;
	bool isRoomShouldClose( IRoom* pRoom)override;
	bool isOmitNewPlayerHalo(IRoom* pRoom )override;
	void onRankPlayerChanged( uint32_t nUID , uint16_t nPreIdx , uint16_t nCurIdx )override;
	bool isPlayerLoseReachMax( IRoom* pRoom, uint32_t nUserUID )override;
	bool onPlayerWillDoLeaveRoom(IRoom* pRoom , IRoom::stStandPlayer* pPlayer )override;
	bool onDelayPlayerWillLeaveRoom(IRoom* pRoom , ISitableRoomPlayer* pPlayer )override;

	// self 
	uint32_t getRoomState(){ return m_eState ; }
	time_t getCloseTime(){ return m_tCloseTime ;}
	bool isRoomClosed();
	void sendRoomInfo(uint32_t nSessionID );
	stPrivateRoomPlayerItem* getPlayerByUID(uint32_t nUserUID )
	{
		auto iter = m_mapPrivateRoomPlayers.find(nUserUID) ;
		if ( iter != m_mapPrivateRoomPlayers.end() )
		{
			return iter->second ;
		}
		return nullptr ;
	}
protected:
	IRoomManager* m_pRoomMgr ;
	uint32_t m_nRoomID ;
	uint32_t m_nOwnerUID ;
	time_t m_tCloseTime ;
	time_t m_tCreateTime ;
	time_t m_nDeadTime ;
	uint32_t m_nDuringSeconds ;
	eRoomState m_eState ;
	stSitableRoomConfig* m_pConfig ;
	REAL_ROOM_PTR m_pRoom ;

	bool m_bRoomInfoDiry ; 

	std::map<uint32_t,stPrivateRoomPlayerItem*> m_mapPrivateRoomPlayers ;
};




// implement 
#include "LogManager.h"
#include "RoomConfig.h"
#include "ServerMessageDefine.h"
#include "IRoomManager.h"
#include "SeverUtility.h"
#include "ServerStringTable.h"
#include "IRoomState.h"
#include "RewardConfig.h"

template<class T >
CPrivateRoom<T>::CPrivateRoom()
{
	m_pRoomMgr = nullptr ;
	m_nRoomID = 0 ;
	m_nOwnerUID = 0;
	m_tCloseTime = 0 ;
	m_nDeadTime = 0;
	m_nDuringSeconds = 0 ;
	m_eState = eRoomState_None ;
	m_pConfig = nullptr;
	m_pRoom = nullptr;

	m_bRoomInfoDiry = false; 
}

template<class T >
CPrivateRoom<T>::~CPrivateRoom()
{
	if ( m_pRoom )
	{
		delete m_pRoom ;
		m_pRoom = nullptr ;
	}

	for ( auto ref : m_mapPrivateRoomPlayers )
	{
		delete ref.second ;
		ref.second = nullptr ;
	}
	m_mapPrivateRoomPlayers.clear() ;
}

// interface 
template<class T >
bool CPrivateRoom<T>::onFirstBeCreated(IRoomManager* pRoomMgr,stBaseRoomConfig* pConfig, uint32_t nRoomID, Json::Value& vJsValue )
{
	time_t tNow = time(nullptr) ;
	m_pConfig = (stSitableRoomConfig*)pConfig ;
	m_nRoomID = nRoomID ;
	m_nDuringSeconds = 2 * 60;
	m_tCreateTime = tNow ;
	m_eState = eRoomState_Opening ;
	m_pRoomMgr = pRoomMgr ;
	m_bRoomInfoDiry = true ;
	if ( vJsValue["duringTime"].isNull() == false )
	{
		m_nDuringSeconds = vJsValue["duringTime"].asUInt() ;
		CLogMgr::SharedLogMgr()->PrintLog("create private room duiring is = %u",m_nDuringSeconds) ;
	}
	else
	{
		CLogMgr::SharedLogMgr()->ErrorLog("create private room duiring is null ?") ;
	}

	if ( vJsValue["ownerUID"].isNull() == false )
	{
		m_nOwnerUID = vJsValue["ownerUID"].asUInt() ;
		CLogMgr::SharedLogMgr()->PrintLog("create private room ownerUID is = %u",m_nOwnerUID) ;
	}
	else
	{
		CLogMgr::SharedLogMgr()->ErrorLog("create private room ownerUID is null ?") ;
	}

	m_tCloseTime = tNow + (time_t)m_nDuringSeconds ;
	m_nDeadTime = m_tCloseTime + 1 ; 

	vJsValue["parentRoomID"] = getRoomID() ;

	m_pRoom = new REAL_ROOM ;
	m_pRoom->onFirstBeCreated(pRoomMgr,pConfig,0 ,vJsValue);
	pRoomMgr->reqeustChatRoomID(m_pRoom);
	m_pRoom->setDelegate(this);

	CLogMgr::SharedLogMgr()->PrintLog("create 1 private room") ;
	return true ;
}

template<class T >
void CPrivateRoom<T>::serializationFromDB(IRoomManager* pRoomMgr,stBaseRoomConfig* pConfig,uint32_t nRoomID , Json::Value& vJsValue )
{
	m_bRoomInfoDiry = false ;
	m_nRoomID = nRoomID ;
	m_pConfig = (stSitableRoomConfig*)pConfig ;
	m_pRoomMgr = pRoomMgr ;
	m_eState = (eRoomState)vJsValue["state"].asUInt();
	m_tCloseTime = (time_t)vJsValue["closeTime"].asUInt();
	m_nDeadTime = m_tCloseTime + 1;
	m_nDuringSeconds = vJsValue["duringTime"].asUInt();
	
	m_nOwnerUID = vJsValue["ownerUID"].asUInt();
	m_tCreateTime = (time_t)vJsValue["createTime"].asUInt() ;

	vJsValue["parentRoomID"] = getRoomID() ;

	Json::Value subRoom = vJsValue["subRoom"];
	m_pRoom = new REAL_ROOM ;
	m_pRoom->serializationFromDB(pRoomMgr,pConfig,0,subRoom);
	m_pRoom->setDelegate(this);

	// read rank data 
	stMsgReadRoomPlayer msgRead ;
	msgRead.nRoomID = getRoomID();
	msgRead.nRoomType = getRoomType() ;
	msgRead.nTermNumber = 0 ;
	m_pRoomMgr->sendMsg(&msgRead,sizeof(msgRead),getRoomID()) ;

	// read private room data 
	stMsgReadPrivateRoomPlayer msgReadPrivate ;
	msgReadPrivate.nRoomID = getRoomID() ;
	msgReadPrivate.nRoomType = getRoomType() ;
	m_pRoomMgr->sendMsg(&msgReadPrivate,sizeof(msgReadPrivate),getRoomID()) ;

	CLogMgr::SharedLogMgr()->PrintLog("read room id = %u ternm = %u rank player",getRoomID(),0) ;
}

template<class T >
void CPrivateRoom<T>::serializationToDB()
{
	Json::StyledWriter jsWrite ;
	Json::Value vValue ;

	vValue["state"] = m_eState ;
	vValue["duringTime"] = m_nDuringSeconds ;
	vValue["closeTime"] = (uint32_t)m_tCloseTime ;
	vValue["ownerUID"] = getOwnerUID() ;
	//vValue["deadTime"] = (uint32_t)m_nDeadTime ;
	vValue["createTime"] = (uint32_t)m_tCreateTime ;

	Json::Value subRoom ;
	m_pRoom->willSerializtionToDB(subRoom);
	vValue["subRoom"] = subRoom ;

	std::string strJson = jsWrite.write(vValue);

	stMsgSaveUpdateRoomInfo msgSave ;
	msgSave.nRoomType = getRoomType() ;
	msgSave.nRoomID = getRoomID() ;
	msgSave.nJsonLen = strJson.size() ;
	msgSave.nConfigID = m_pConfig->nConfigID;
	msgSave.nRoomOwnerUID = getOwnerUID() ;

	CAutoBuffer autoBuffer(sizeof(msgSave) + msgSave.nJsonLen);
	autoBuffer.addContent((char*)&msgSave,sizeof(msgSave)) ;
	autoBuffer.addContent(strJson.c_str(),msgSave.nJsonLen) ;
	m_pRoomMgr->sendMsg((stMsg*)autoBuffer.getBufferPtr(),autoBuffer.getContentSize(),0) ;
}

template<class T >
uint8_t CPrivateRoom<T>::canPlayerEnterRoom( stEnterRoomData* pEnterRoomPlayer )  // return 0 means ok ;
{
	if ( getRoomState() != eRoomState_Opening )
	{
		return 7 ;  // room not open 
	}

	if ( m_pRoom )
	{
		return m_pRoom->canPlayerEnterRoom(pEnterRoomPlayer);
	}
	return 7 ;
}

template<class T >
void CPrivateRoom<T>::onPlayerEnterRoom(stEnterRoomData* pEnterRoomPlayer,int8_t& nSubIdx )
{
	nSubIdx = 0 ;
	if ( nullptr == m_pRoom )
	{
		return ;
	}
	
	stPrivateRoomPlayerItem* pPlayerItem = nullptr ;
	auto iter = m_mapPrivateRoomPlayers.find(pEnterRoomPlayer->nUserUID) ;
	if ( iter == m_mapPrivateRoomPlayers.end() )
	{
		pPlayerItem = new stPrivateRoomPlayerItem(pEnterRoomPlayer->nUserUID,pEnterRoomPlayer->nCoin) ;
		m_mapPrivateRoomPlayers[pPlayerItem->nUserUID] = pPlayerItem ;
	}
	else
	{
		pPlayerItem = iter->second ;
	}

	pPlayerItem->nToTalCoin = pEnterRoomPlayer->nCoin ;

	stEnterRoomData refEnterData ;
	memcpy(&refEnterData,pEnterRoomPlayer,sizeof(stEnterRoomData));
	refEnterData.nCoin = pPlayerItem->nCoinInRoom ;
	m_pRoom->onPlayerEnterRoom(&refEnterData,nSubIdx) ;
	sendRoomInfo(pEnterRoomPlayer->nUserSessionID);
	CLogMgr::SharedLogMgr()->PrintLog("uid = %u , enter room id = %u , subIdx = %u inRoom coin = %u , total coin = %u",pEnterRoomPlayer->nUserUID, getRoomID(),0,pPlayerItem->nCoinInRoom,pPlayerItem->nToTalCoin) ;

	// apns tell creator have player enter room 
	CSendPushNotification::getInstance()->reset() ;
	CSendPushNotification::getInstance()->addTarget(getOwnerUID()) ;
	CSendPushNotification::getInstance()->setContent(CServerStringTable::getInstance()->getStringByID(11),1);
	auto abf = CSendPushNotification::getInstance()->getNoticeMsgBuffer() ;
	if ( abf )
	{
		m_pRoomMgr->sendMsg((stMsg*)abf->getBufferPtr(),abf->getContentSize(),getRoomID()) ;
	}
	CLogMgr::SharedLogMgr()->PrintLog("send have player enter room  apns uid = %u",getOwnerUID()) ;
}

template<class T >
bool CPrivateRoom<T>::onPlayerApplyLeaveRoom(uint32_t nUserUID )
{
	if ( m_pRoom )
	{
		return m_pRoom->onPlayerApplyLeaveRoom(nUserUID) ;
	}
	return false ;
}

template<class T >
void CPrivateRoom<T>::roomItemDetailVisitor(Json::Value& vOutJsValue)
{
	vOutJsValue["configID"] = m_pConfig->nConfigID ;
	vOutJsValue["closeTime"] = (uint32_t)m_tCloseTime ;
	vOutJsValue["state"] = (uint32_t)getRoomState() ;
	vOutJsValue["createTime"] = (uint32_t)m_tCreateTime ;
	vOutJsValue["curCnt"] = m_pRoom->getSitDownPlayerCount();
}

template<class T >
uint32_t CPrivateRoom<T>::getRoomID()
{
	return m_nRoomID ;
}

template<class T >
uint8_t CPrivateRoom<T>::getRoomType()
{
	if ( m_pRoom )
	{
		return m_pRoom->getRoomType();
	}
	return eRoom_None ;
}

template<class T >
bool CPrivateRoom<T>::isRoomClosed()
{
	if ( m_pRoom )
	{
		return eRoomState_Close == m_pRoom->getCurRoomState()->getStateID();
	}
	return false ;
}

template<class T >
void CPrivateRoom<T>::update(float fDelta)
{
	if ( m_pRoom )
	{
		m_pRoom->update(fDelta);
	}
	// update room state 
	switch ( getRoomState())
	{
	case eRoomState_Opening:
		{
			time_t tNow = time(nullptr);
			if ( tNow >= getCloseTime() )
			{
				m_eState = eRoomState_WillClose ;
				m_bRoomInfoDiry = true ;
				CLogMgr::SharedLogMgr()->PrintLog("uid = %d change do will close",getRoomID() );
			}
		}
		break;
	case eRoomState_WillClose:
		{
			if ( isRoomClosed() )
			{
				m_eState = eRoomState_Close ;
				CLogMgr::SharedLogMgr()->PrintLog("uid = %d change do close",getRoomID() );
			}
		}
		break;
	case eRoomState_Close:
		{
			time_t tNow = time(nullptr);
			if ( tNow > m_nDeadTime )
			{
				m_eState = eRoomState_WillDead ;
				m_bRoomInfoDiry = true ;
			}
		}
		break;
	case eRoomState_WillDead:
		{
			if ( isRoomClosed() )
			{
				CLogMgr::SharedLogMgr()->PrintLog("uid = %d change do dead, prepare recorders ",getRoomID() );
				m_eState = eRoomState_Dead ;
				m_bRoomInfoDiry = true ;
				if ( m_pRoom )
				{
					m_pRoom->forcePlayersLeaveRoom();
					m_pRoomMgr->deleteRoomChatID(m_pRoom->getChatRoomID()) ;
				}

				// sync player coin  ;
				stPrivateRoomRecorder* pRecoder = new stPrivateRoomRecorder ;
				pRecoder->nConfigID = m_pConfig->nConfigID ;
				pRecoder->nCreaterUID = getOwnerUID() ;
				pRecoder->nRoomID = getRoomID() ;
				pRecoder->nTime = time(nullptr) ;
				pRecoder->nDuringSeconds = m_nDuringSeconds ;
				
				// produce room bill info ;
				sortRoomRankItem();
				// reset apns target 
				CSendPushNotification::getInstance()->reset() ;

				stMsgSyncPrivateRoomResult msgResult ;
				msgResult.nRoomID = getRoomID() ;
				msgResult.nRoomType = getRoomType() ;
				msgResult.nCreatorUID = getOwnerUID() ;
				msgResult.nDuringTimeSeconds = m_nDuringSeconds ;
				
				LIST_ROOM_RANK_ITEM::iterator iter = m_vSortedRankItems.begin();
				for ( uint16_t nIdx = 0 ; iter != m_vSortedRankItems.end(); ++iter, ++nIdx )
				{
					// add target to apns 
					CSendPushNotification::getInstance()->addTarget((*iter)->nUserUID);

					// push rank 5 to send to client ;
					stRoomRankItem* pItem = (*iter) ;
					auto pP = getPlayerByUID(pItem->nUserUID);
					if ( pP == nullptr )
					{
						continue;
					}
					Json::Value vValue ;
					vValue["uid"] = pP->nUserUID ;
					vValue["buyIn"] = pP->nToTalBuyIn ;
					vValue["offset"] = pItem->nGameOffset ;
					pRecoder->playerDetail[nIdx] = vValue ;

					msgResult.nOffset = pItem->nGameOffset ;
					msgResult.nTargetPlayerUID = pItem->nUserUID ;
					msgResult.nFinalCoin = pP->nCoinInRoom ;
					msgResult.nBuyIn = pP->nToTalBuyIn ;
					msgResult.nConfigID = m_pConfig->nConfigID ;
					
					m_pRoomMgr->sendMsg(&msgResult,sizeof(msgResult),0);
				}

				if ( m_vSortedRankItems.empty() || pRecoder->playerDetail.isNull())
				{
					delete pRecoder ; 
					pRecoder = nullptr ;
					CLogMgr::SharedLogMgr()->ErrorLog("private room log is null id = %u",getRoomID()) ;
				}
				else 
				{
					m_pRoomMgr->addPrivateRoomRecorder(pRecoder) ;
				}
				


				// send apns tell close 
				CSendPushNotification::getInstance()->setContent(CServerStringTable::getInstance()->getStringByID(10),1);
				auto abf = CSendPushNotification::getInstance()->getNoticeMsgBuffer() ;
				if ( abf )
				{
					m_pRoomMgr->sendMsg((stMsg*)abf->getBufferPtr(),abf->getContentSize(),getRoomID()) ;
				}
				CLogMgr::SharedLogMgr()->PrintLog("send close end apns uid = %u",getRoomID()) ;
			}
		}
		break;
	case eRoomState_Dead:
	case eRoomState_None:
		{

		}
		break;
	default:
		CLogMgr::SharedLogMgr()->ErrorLog("unknonw room state = %u room id = %u",getRoomState(),getRoomID()) ;
		break;
	}
}

template<class T >
void CPrivateRoom<T>::onTimeSave()
{
	CLogMgr::SharedLogMgr()->PrintLog("time save room info room id = %u",getRoomID());
	// save room rank ;
	stMsgSaveRoomPlayer msgSave ;
	msgSave.nRoomID = getRoomID() ;
	msgSave.nRoomType = getRoomType() ;
	msgSave.nTermNumber = 0 ;
	for ( auto pp : m_vSortedRankItems )
	{
		if ( !pp->bIsDiryt )
		{
			continue;
		}
		pp->bIsDiryt = false ;

		msgSave.savePlayer.nUserUID = pp->nUserUID ;
		msgSave.savePlayer.nGameOffset = pp->nGameOffset ;
		msgSave.savePlayer.nOtherOffset = pp->nOtherOffset ;
		m_pRoomMgr->sendMsg(&msgSave,sizeof(msgSave),0) ;
		CLogMgr::SharedLogMgr()->PrintLog("update rank uid = %u , offset = %d",pp->nUserUID,pp->nGameOffset) ;
	}

	if ( m_bRoomInfoDiry )
	{
		serializationToDB();
		m_bRoomInfoDiry = false ;
	}

	// save private room player 

	CAutoBuffer auBuffer( sizeof( stMsgSavePrivateRoomPlayer ) + 200 );
	for ( auto refPrivatePlayer : m_mapPrivateRoomPlayers )
	{
		if ( refPrivatePlayer.second->isDirty == false )
		{
			continue;
		}

		refPrivatePlayer.second->isDirty = true ;

		Json::Value jsValue ;
		refPrivatePlayer.second->toJsvalue(jsValue);

		Json::StyledWriter jsWrite ;
		std::string str = jsWrite.write(jsValue) ;

		stMsgSavePrivateRoomPlayer msgSavePrivatePlayer ;
		msgSavePrivatePlayer.nJsonLen = str.size() ;
		msgSavePrivatePlayer.nRoomID = getRoomID() ;
		msgSavePrivatePlayer.nRoomType = getRoomType() ;
		msgSavePrivatePlayer.nUserUID = refPrivatePlayer.second->nUserUID ;

		auBuffer.clearBuffer() ;
		auBuffer.addContent(&msgSavePrivatePlayer,sizeof(msgSavePrivatePlayer)) ;
		auBuffer.addContent(str.c_str(),msgSavePrivatePlayer.nJsonLen) ;
		m_pRoomMgr->sendMsg((stMsg*)auBuffer.getBufferPtr(),auBuffer.getContentSize(),0) ;
	}
}

template<class T >
bool CPrivateRoom<T>::onMessage( stMsg* prealMsg , eMsgPort eSenderPort , uint32_t nPlayerSessionID )
{
	switch ( prealMsg->usMsgType )
	{
	case MSG_READ_PRIVATE_ROOM_PLAYER:
		{
			stMsgReadPrivateRoomPlayerRet* pRet = (stMsgReadPrivateRoomPlayerRet*)prealMsg ;
			char* pBuffer = (char*)pRet ;
			pBuffer += sizeof(stMsgReadPrivateRoomPlayerRet);
			Json::Reader jsReader ;
			Json::Value jsValue ;
			jsReader.parse(pBuffer,pBuffer + pRet->nJsonLen,jsValue,false) ;
			stPrivateRoomPlayerItem* pItem = new stPrivateRoomPlayerItem(0,0);
			pItem->fromjsValue(jsValue) ;
			m_mapPrivateRoomPlayers[pItem->nUserUID] = pItem ;
		}
		break ;
	case MSG_READ_ROOM_PLAYER:
		{
			stMsgReadRoomPlayerRet* pRet = (stMsgReadRoomPlayerRet*)prealMsg ; 
			if ( pRet->nTermNumber != 0 )
			{
				CLogMgr::SharedLogMgr()->SystemLog("recieved last wrong term player data , skip id  room id = %u",getRoomID()) ;
				break; 
			}

			CLogMgr::SharedLogMgr()->PrintLog("room id = %d recive room player data cnt = %d",getRoomID(),pRet->nCnt) ;
			stSaveRoomPlayerEntry* pp = (stSaveRoomPlayerEntry*)(((char*)prealMsg) + sizeof(stMsgReadRoomPlayerRet));
			while ( pRet->nCnt-- )
			{
				stRoomRankItem* p = new stRoomRankItem ;
				p->bIsDiryt = false ;
				p->nUserUID = pp->nUserUID ;
				p->nGameOffset = pp->nGameOffset ;
				p->nOtherOffset = pp->nOtherOffset ;
				 // current term rank item ;
				m_vRoomRankHistroy[p->nUserUID] = p ;
				m_vSortedRankItems.push_back(p) ;
				++pp;
			}
		}
		break;
	case MSG_REQUEST_ROOM_RANK:
		{
			uint32_t nUserID = 0 ;
			if ( m_pRoom )
			{
				auto pp = m_pRoom->getPlayerBySessionID(nPlayerSessionID) ;
				if ( pp )
				{
					nUserID = pp->nUserUID ;
				}
			}

			std::map<uint32_t,stRoomRankEntry> vWillSend ;
			sortRoomRankItem();
			int16_t nSelfRankIdx = -1 ;
			uint16_t nSendRankCnt = 80 ;
			int16_t nMaxCheckCount = 80 ;
			// add 5 player into list ;
			LIST_ROOM_RANK_ITEM::iterator iter = m_vSortedRankItems.begin();
			for ( uint8_t nIdx = 0 ; iter != m_vSortedRankItems.end(); ++iter,++nIdx )
			{
				// push rank 5 to send to client ;
				stRoomRankItem* pItem = (*iter) ;
				if ( pItem->nUserUID == nUserID )
				{
					nSelfRankIdx = nIdx ;
				}

				if ( nIdx < nSendRankCnt || pItem->nUserUID == nUserID  )
				{
					if ( getPlayerByUID(pItem->nUserUID) == nullptr )
					{
						continue; 
					}

					stRoomRankEntry entry ;
					entry.nGameOffset = pItem->nGameOffset ;
					entry.nOtherOffset = getPlayerByUID(pItem->nUserUID)->nToTalBuyIn;
					entry.nUserUID = pItem->nUserUID ;
					vWillSend[pItem->nUserUID] = entry ;
				}
				else
				{
					if ( nSelfRankIdx != -1 || nUserID == 0 )  // find self or not in this room ;
					{
						break;
					}
				}

				--nMaxCheckCount ;
				if ( nMaxCheckCount <= 0 )
				{
					break;
				}
			}

			CLogMgr::SharedLogMgr()->PrintLog("uid = %d request rank room id = %u",nUserID,getRoomID());
			// send room info to player ;
			stMsgRequestRoomRankRet msgRet ;
			msgRet.nCnt = vWillSend.size() ;
			msgRet.nSelfRankIdx = nSelfRankIdx ;
			CAutoBuffer msgBuffer(sizeof(msgRet) + msgRet.nCnt * sizeof(stRoomRankEntry));
			msgBuffer.addContent(&msgRet,sizeof(msgRet));
			for ( auto& itemSendPlayer : vWillSend )
			{
				msgBuffer.addContent(&itemSendPlayer.second,sizeof(stRoomRankEntry));
				CLogMgr::SharedLogMgr()->PrintLog("room id = %u rank player uid = %u offset = %d",getRoomID(),itemSendPlayer.second.nUserUID,itemSendPlayer.second.nGameOffset);
			}
			m_pRoomMgr->sendMsg((stMsg*)msgBuffer.getBufferPtr(),msgBuffer.getContentSize(),nPlayerSessionID) ;
		}
		break;
	case MSG_REQUEST_ROOM_INFO:
		{
			stMsgToRoom* pRet = (stMsgToRoom*)prealMsg ;
			sendRoomInfo(nPlayerSessionID);
		}
		break;
	case MSG_PLAYER_REBUY:
		{
			stMsgPlayerReBuyInRet msgBack ;
			msgBack.nRet = 0 ;
			stMsgPlayerReBuyIn* pRet = (stMsgPlayerReBuyIn*)prealMsg ;
			auto stStandPlayer = m_pRoom->getPlayerBySessionID(nPlayerSessionID);
			assert( stStandPlayer && "not enter room how to rebuy ?" );
			ISitableRoomPlayer* stiDownPlayer = m_pRoom->getSitdownPlayerBySessionID(nPlayerSessionID);

			auto iter = m_mapPrivateRoomPlayers.find(stStandPlayer->nUserUID) ;
			
			assert(iter != m_mapPrivateRoomPlayers.end() && "not at entry player how to rebuy ?" );
			stPrivateRoomPlayerItem* pPrivatePlayer = iter->second ;
			
			uint32_t nCurInRoomCoin = stStandPlayer->nCoin ;
			if ( stiDownPlayer )
			{
				assert("when sit down must take in all " && nCurInRoomCoin == 0 );
				nCurInRoomCoin += stiDownPlayer->getCoin();
			}

			msgBack.nBuyInCoin = pRet->nBuyInCoin ;
			msgBack.nFinalCoin = nCurInRoomCoin ;
			// process my ru 
			if ( pPrivatePlayer->nToTalCoin < pRet->nBuyInCoin )
			{
				CLogMgr::SharedLogMgr()->ErrorLog("coin not enought") ;
				msgBack.nRet = 1 ;
				m_pRoom->sendMsgToPlayer(&msgBack,sizeof(msgBack),nPlayerSessionID);
				return true ;
			}

			if ( nCurInRoomCoin + pRet->nBuyInCoin > m_pConfig->nBaseTakeIn * 5 || ( pRet->nBuyInCoin % m_pConfig->nBaseTakeIn != 0 ) )
			{
				CLogMgr::SharedLogMgr()->ErrorLog("buy in too many coin");
				msgBack.nRet = 1 ;
				m_pRoom->sendMsgToPlayer(&msgBack,sizeof(msgBack),nPlayerSessionID);
				return true ;
			}
			
			pPrivatePlayer->buyIn(pRet->nBuyInCoin) ;

			msgBack.nRet = 0 ;
			if ( stiDownPlayer )
			{
				stiDownPlayer->setCoin(stiDownPlayer->getCoin() + pRet->nBuyInCoin ) ;

				stMsgPrivateRoomReBuyIn msgRoom ;
				msgRoom.nBuyInCoin = pRet->nBuyInCoin ;
				msgRoom.nFinalCoin = stiDownPlayer->getCoin() ;
				msgRoom.nIdx = stiDownPlayer->getIdx() ;
				m_pRoom->sendRoomMsg(&msgRoom,sizeof(msgRoom)) ;
				CLogMgr::SharedLogMgr()->PrintLog("uid = %u ,sitdown coin = %u , stand coin = %u , totalBuyin = %u , newBuyin = %u ",stStandPlayer->nUserUID,stiDownPlayer->getCoin(),stStandPlayer->nCoin,pPrivatePlayer->nToTalBuyIn,pRet->nBuyInCoin) ;
			}
			else
			{
				stStandPlayer->nCoin += pRet->nBuyInCoin ;
			}

			msgBack.nFinalCoin = nCurInRoomCoin + pRet->nBuyInCoin ;
			m_pRoom->sendMsgToPlayer(&msgBack,sizeof(msgBack),nPlayerSessionID);
			CLogMgr::SharedLogMgr()->PrintLog("uid = %u  , stand coin = %u , totalBuyin = %u , newBuyin = %u ",stStandPlayer->nUserUID,stStandPlayer->nCoin,pPrivatePlayer->nToTalBuyIn,pRet->nBuyInCoin) ;
		}
		break;
	default:
		{
			if ( m_pRoom )
			{
				return m_pRoom->onMessage(prealMsg,eSenderPort,nPlayerSessionID) ;
			}
			else
			{
				 
			}
		}
		return false ;
	}

	return true ;
}

template<class T >
bool CPrivateRoom<T>::isDeleteRoom()
{
	return m_eState == eRoomState_Dead ;
}

template<class T >
void CPrivateRoom<T>::deleteRoom()
{
	m_eState = eRoomState_WillDead ;
}

template<class T >
uint32_t CPrivateRoom<T>::getOwnerUID()
{
	return m_nOwnerUID ;
}

// delegate msg ;
template<class T >
uint8_t CPrivateRoom<T>::canPlayerEnterRoom( IRoom* pRoom,stEnterRoomData* pEnterRoomPlayer )  // return 0 means ok ;
{
	return 0 ;
}

template<class T >
bool CPrivateRoom<T>::isRoomShouldClose( IRoom* pRoom)
{
	return eRoomState_Opening != m_eState ;
}

template<class T >
bool CPrivateRoom<T>::isOmitNewPlayerHalo(IRoom* pRoom )
{
	return true ;
}

template<class T >
void CPrivateRoom<T>::onRankPlayerChanged( uint32_t nUID , uint16_t nPreIdx , uint16_t nCurIdx )
{
	return ;
}

template<class T >
bool CPrivateRoom<T>::isPlayerLoseReachMax( IRoom* pRoom, uint32_t nUserUID )
{
	return false ;
}

template<class T >
bool CPrivateRoom<T>::onPlayerWillDoLeaveRoom(IRoom* pRoom , IRoom::stStandPlayer* pPlayer )
{
	auto pRoomPlayer = getPlayerByUID(pPlayer->nUserUID) ;
	pRoomPlayer->nCoinInRoom = pPlayer->nCoin ;
	pRoomPlayer->isDirty = true ;

	pPlayer->nCoin = pRoomPlayer->nToTalCoin ;
	IRoomDelegate::onPlayerWillDoLeaveRoom(pRoom,pPlayer) ;
	pRoomPlayer->nToTalCoin = 0 ;
	CLogMgr::SharedLogMgr()->PrintLog("uid = %u leave private room sys coin = %u , coin in this room = %u",pPlayer->nUserUID,pPlayer->nCoin, pRoomPlayer->nCoinInRoom ) ;
	return true ;
}

template<class T >
bool CPrivateRoom<T>::onDelayPlayerWillLeaveRoom(IRoom* pRoom , ISitableRoomPlayer* pPlayer )
{
	auto pRoomPlayer = getPlayerByUID(pPlayer->getUserUID()) ;
	pRoomPlayer->nCoinInRoom += pPlayer->getCoin() ;
	pRoomPlayer->isDirty = true ;

	pPlayer->setCoin( 0 );
	IRoomDelegate::onDelayPlayerWillLeaveRoom(pRoom,pPlayer) ;

	CLogMgr::SharedLogMgr()->PrintLog("uid = %u leave DELAY private room sys coin = %u , coin in this room = %u",pPlayer->getUserUID(),pPlayer->getCoin(), pRoomPlayer->nCoinInRoom ) ;
	return true ;
}

template<class T >
void CPrivateRoom<T>::sendRoomInfo(uint32_t nSessionID )
{
	typename REAL_ROOM_PTR pRoom = m_pRoom ;
	if ( pRoom == nullptr )
	{
		CLogMgr::SharedLogMgr()->ErrorLog("session id = %u requesiont room info room id = %u, subIdx = %d is null",nSessionID , getRoomID(),0) ;
		return ;
	}

	stMsgRoomInfo msgInfo ;
	msgInfo.eCurRoomState = pRoom->getCurRoomState()->getStateID() ;
	msgInfo.fChouShuiRate = m_pConfig->fDividFeeRate ;
	msgInfo.nChatRoomID = pRoom->getChatRoomID() ;
	msgInfo.nCloseTime = (uint32_t)getCloseTime() ;
	msgInfo.nDeskFee =  m_pConfig->nDeskFee ;
	msgInfo.nMaxSeat = (uint8_t)pRoom->getSeatCount();
	msgInfo.nRoomID = getRoomID() ;
	msgInfo.nRoomType = getRoomType() ;
	msgInfo.nSubIdx = pRoom->getRoomID() ;

	 
	auto iPlayer = pRoom->getPlayerBySessionID(nSessionID);
	auto iter = m_mapPrivateRoomPlayers.find(iPlayer->nUserUID) ;
	assert(iter != m_mapPrivateRoomPlayers.end() && "why this is null ?" );
	Json::Value jsPrivate ;
	jsPrivate["selfCoin"] = iter->second->nCoinInRoom ;
	jsPrivate["baseTakeIn"] = m_pConfig->nBaseTakeIn;

	Json::StyledWriter wr ;
	Json::Value vOut ;
	vOut["private"] = jsPrivate ;

	pRoom->roomInfoVisitor(vOut);
	std::string str = wr.write(vOut) ;
	msgInfo.nJsonLen = str.size() ;
	CAutoBuffer sBuf(sizeof(msgInfo) + msgInfo.nJsonLen );
	sBuf.addContent(&msgInfo,sizeof(msgInfo)) ;
	sBuf.addContent(str.c_str(),msgInfo.nJsonLen) ;
	m_pRoomMgr->sendMsg((stMsg*)sBuf.getBufferPtr(),sBuf.getContentSize(),nSessionID) ;
	CLogMgr::SharedLogMgr()->PrintLog("send room info to session id = %u js:%s",nSessionID, str.c_str()) ;

	pRoom->sendRoomPlayersInfo(nSessionID);
}