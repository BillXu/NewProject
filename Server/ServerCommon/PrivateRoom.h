#pragma once
#include "IRoomInterface.h"
#include "ISitableRoom.h"
#include "IRoomDelegate.h"
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

	// self 
	uint32_t getRoomState(){ return m_eState ; }
	time_t getCloseTime(){ return m_tCloseTime ;}
	bool isRoomClosed();
	void sendRoomInfo(uint32_t nSessionID );
protected:
	IRoomManager* m_pRoomMgr ;
	uint32_t m_nRoomID ;
	uint32_t m_nOwnerUID ;
	time_t m_tCloseTime ;
	time_t m_tCreateTime ;
	time_t m_nDeadTime ;
	uint32_t m_nDuringSeconds ;
	eRoomState m_eState ;
	stBaseRoomConfig* m_pConfig ;
	REAL_ROOM_PTR m_pRoom ;

	bool m_bRoomInfoDiry ; 
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
}

// interface 
template<class T >
bool CPrivateRoom<T>::onFirstBeCreated(IRoomManager* pRoomMgr,stBaseRoomConfig* pConfig, uint32_t nRoomID, Json::Value& vJsValue )
{
	time_t tNow = time(nullptr) ;
	m_pConfig = pConfig ;
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
	m_nDeadTime = m_tCloseTime + 60*60*24 ; 

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
	m_pConfig = pConfig ;
	m_pRoomMgr = pRoomMgr ;
	m_eState = (eRoomState)vJsValue["state"].asUInt();
	m_nDeadTime = (time_t)vJsValue["deadTime"].asUInt();
	m_nDuringSeconds = vJsValue["duringTime"].asUInt();
	m_tCloseTime = (time_t)vJsValue["closeTime"].asUInt();
	m_nOwnerUID = vJsValue["ownerUID"].asUInt();
	m_tCreateTime = (time_t)vJsValue["createTime"].asUInt() ;

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
	vValue["deadTime"] = (uint32_t)m_nDeadTime ;
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
	if ( m_pRoom )
	{
		m_pRoom->onPlayerEnterRoom(pEnterRoomPlayer,nSubIdx) ;
		sendRoomInfo(pEnterRoomPlayer->nUserSessionID);
		m_pRoom->sendRoomPlayersInfo(pEnterRoomPlayer->nUserSessionID);
		CLogMgr::SharedLogMgr()->PrintLog("uid = %u , enter room id = %u , subIdx = %u",pEnterRoomPlayer->nUserUID, getRoomID(),0) ;
	}
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
		m_pRoom->getRoomType();
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
				CLogMgr::SharedLogMgr()->PrintLog("uid = %d change do dead",getRoomID() );
				m_eState = eRoomState_Dead ;
				m_bRoomInfoDiry = true ;
				if ( m_pRoom )
				{
					m_pRoom->forcePlayersLeaveRoom();
					m_pRoomMgr->deleteRoomChatID(m_pRoom->getChatRoomID()) ;
				}
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
}

template<class T >
bool CPrivateRoom<T>::onMessage( stMsg* prealMsg , eMsgPort eSenderPort , uint32_t nPlayerSessionID )
{
	switch ( prealMsg->usMsgType )
	{
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

				if ( nIdx < nSendRankCnt || pItem->nUserUID == nUserID )
				{
					stRoomRankEntry entry ;
					entry.nGameOffset = pItem->nGameOffset ;
					entry.nOtherOffset = pItem->nOtherOffset ;
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

	Json::StyledWriter wr ;
	Json::Value vOut ;
	pRoom->roomInfoVisitor(vOut);
	std::string str = wr.write(vOut) ;
	msgInfo.nJsonLen = str.size() ;
	CAutoBuffer sBuf(sizeof(msgInfo) + msgInfo.nJsonLen );
	sBuf.addContent(&msgInfo,sizeof(msgInfo)) ;
	sBuf.addContent(str.c_str(),msgInfo.nJsonLen) ;
	m_pRoomMgr->sendMsg((stMsg*)sBuf.getBufferPtr(),sBuf.getContentSize(),nSessionID) ;
	CLogMgr::SharedLogMgr()->PrintLog("send room info to session id = %u js:%s",nSessionID, str.c_str()) ;
}