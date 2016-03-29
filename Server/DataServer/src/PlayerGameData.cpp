#include "PlayerGameData.h"
#include "Player.h"
#include "LogManager.h"
#include "GameServerApp.h"
#include "PlayerBaseData.h"
#include <json/json.h>
#include "AutoBuffer.h"
#include "TaxasPokerPeerCard.h"
#include "RoomConfig.h"
void CPlayerGameData::Reset()
{
	IPlayerComponent::Reset();
	m_nStateInRoomID = 0;
	m_nStateInRoomType = eRoom_Max;
	m_nSubRoomIdx = 0 ;
	memset(&m_vData,0,sizeof(m_vData));
	for (auto& r : m_vMyOwnRooms )
	{
		r.clear() ;
	}

	stMsgReadPlayerTaxasData msgr ;
	msgr.nUserUID = GetPlayer()->GetUserUID() ;
	SendMsg(&msgr,sizeof(msgr)) ;

	stMsgReadMyOwnTaxasRooms msgReq ;
	msgReq.nUserUID = GetPlayer()->GetUserUID();
	SendMsg(&msgReq,sizeof(msgReq)) ;

	// niu  niu 
	stMsgReadMyOwnRooms msgReqt ;
	msgReqt.nUserUID = GetPlayer()->GetUserUID();
	SendMsg(&msgReqt,sizeof(msgReqt)) ;

	stMsgReadPlayerNiuNiuData msg ;
	msg.nUserUID = GetPlayer()->GetUserUID() ;
	SendMsg(&msg,sizeof(msg)) ;
	CLogMgr::SharedLogMgr()->PrintLog("requesting player taxas data for uid = %d",msg.nUserUID);
}

void CPlayerGameData::Init()
{
	IPlayerComponent::Init();
	m_eType = ePlayerComponent_PlayerGameData ;
	m_nStateInRoomID = 0;
	m_nStateInRoomType = eRoom_Max;
	memset(&m_vData,0,sizeof(m_vData));
	for (auto& r : m_vMyOwnRooms )
	{
		r.clear() ;
	}

	stMsgReadMyOwnTaxasRooms msgReqr ;
	msgReqr.nUserUID = GetPlayer()->GetUserUID();
	SendMsg(&msgReqr,sizeof(msgReqr)) ;

	// niu  niu 
	stMsgReadMyOwnRooms msgReq ;
	msgReq.nUserUID = GetPlayer()->GetUserUID();
	SendMsg(&msgReq,sizeof(msgReq)) ;

	stMsgReadPlayerNiuNiuData msg ;
	msg.nUserUID = GetPlayer()->GetUserUID() ;
	SendMsg(&msg,sizeof(msg)) ;
	CLogMgr::SharedLogMgr()->PrintLog("requesting player niuniu data for uid = %d",msg.nUserUID);

	stMsgReadPlayerTaxasData msgt ;
	msgt.nUserUID = GetPlayer()->GetUserUID() ;
	SendMsg(&msgt,sizeof(msgt)) ;
	CLogMgr::SharedLogMgr()->PrintLog("requesting player taxas data for uid = %d",msg.nUserUID);
}

bool CPlayerGameData::OnMessage( stMsg* pMessage , eMsgPort eSenderPort)
{
	if ( IPlayerComponent::OnMessage(pMessage,eSenderPort) )
	{
		return true ;
	}
	switch ( pMessage->usMsgType )
	{
	case MSG_PLAYER_ENTER_ROOM:
		{
			stMsgPlayerEnterRoom* pRet = (stMsgPlayerEnterRoom*)pMessage ;
			if ( isNotInAnyRoom() )
			{
				stMsgSvrEnterRoom msgEnter ;
				msgEnter.cSysIdentifer = GetPlayer()->getMsgPortByRoomType(pRet->nRoomGameType) ;
				if ( msgEnter.cSysIdentifer == ID_MSG_PORT_NONE )
				{
					stMsgPlayerEnterRoomRet msgRet ;
					msgRet.nRet = 6;
					SendMsg(&msgRet,sizeof(msgRet)) ;
					CLogMgr::SharedLogMgr()->ErrorLog("player uid = %d enter game , can not find game port type = %d ",GetPlayer()->GetUserUID(), pRet->nRoomGameType ) ;
					break;
				}

				msgEnter.nGameType = pRet->nRoomGameType ;
				msgEnter.nRoomID = pRet->nRoomID ;
				msgEnter.nSubIdx = pRet->nSubIdx ;
				msgEnter.tPlayerData.isRegisted = GetPlayer()->GetBaseData()->isPlayerRegistered() ;
				msgEnter.tPlayerData.nCoin = GetPlayer()->GetBaseData()->getCoin() ;
				msgEnter.tPlayerData.nUserSessionID = GetPlayer()->GetSessionID() ;
				msgEnter.tPlayerData.nUserUID = GetPlayer()->GetUserUID() ;
				msgEnter.tPlayerData.nNewPlayerHaloWeight = GetPlayer()->GetBaseData()->getNewPlayerHaloWeight() ;
				msgEnter.tPlayerData.nPlayerType = GetPlayer()->GetBaseData()->getPlayerType();
				CGameServerApp::SharedGameServerApp()->sendMsg(msgEnter.tPlayerData.nUserSessionID,(char*)&msgEnter,sizeof(msgEnter)) ;

				m_nStateInRoomID = pRet->nRoomID;
				m_nStateInRoomType = pRet->nRoomGameType;
				CLogMgr::SharedLogMgr()->PrintLog("player uid = %d enter to enter room id = %d , type = %d coin = %u", GetPlayer()->GetUserUID(), m_nStateInRoomID, m_nStateInRoomType,msgEnter.tPlayerData.nCoin ) ;
			}
			else
			{
				stMsgPlayerEnterRoomRet msgRet ;
				msgRet.nRet = 1;
				SendMsg(&msgRet,sizeof(msgRet)) ;
				CLogMgr::SharedLogMgr()->PrintLog("player uid = %d already in room type = %d , id = %d ", GetPlayer()->GetUserUID() , m_nStateInRoomType,m_nStateInRoomID ) ;
			}
		}
		break;
	case MSG_SVR_ENTER_ROOM:
		{
			stMsgSvrEnterRoomRet* pRet = (stMsgSvrEnterRoomRet*)pMessage ;
			stMsgPlayerEnterRoomRet msgRet ;
			msgRet.nRet = pRet->nRet;
			SendMsg(&msgRet,sizeof(msgRet)) ;

			if ( msgRet.nRet )  // enter room failed ;
			{
				m_nStateInRoomID = 0;
				m_nStateInRoomType = eRoom_Max;
				m_nSubRoomIdx = 0 ;
				CLogMgr::SharedLogMgr()->PrintLog("player enter room failed ret = %d uid = %d",msgRet.nRet,GetPlayer()->GetUserUID()) ;
			}
			else
			{
				m_nStateInRoomID = pRet->nRoomID;
				m_nStateInRoomType = pRet->nGameType;
				m_nSubRoomIdx = (uint8_t)pRet->nSubIdx ;
				CLogMgr::SharedLogMgr()->PrintLog("player do enter oom id = %d , type = %d uid = %d subIdx = %u",m_nStateInRoomID,m_nStateInRoomType,GetPlayer()->GetUserUID(),m_nSubRoomIdx) ;
			}
		}
		break;
	case MSG_SVR_DO_LEAVE_ROOM:
		{
			m_nStateInRoomID = 0;
			m_nStateInRoomType = eRoom_Max;
			stMsgSvrDoLeaveRoom* pRet = (stMsgSvrDoLeaveRoom*)pMessage ;
			CLogMgr::SharedLogMgr()->PrintLog("uid = %d leave room coin = %u , back coin = %lld, temp coin = %u",GetPlayer()->GetUserUID(),GetPlayer()->GetBaseData()->getCoin(),pRet->nCoin,GetPlayer()->GetBaseData()->getTempCoin() ) ;
			GetPlayer()->GetBaseData()->setCoin(pRet->nCoin + GetPlayer()->GetBaseData()->getTempCoin()) ;
			GetPlayer()->GetBaseData()->setTempCoin(0) ;
			
			m_vData[pRet->nGameType].nPlayTimes += pRet->nPlayerTimes ;
			m_vData[pRet->nGameType].nWinTimes += pRet->nWinTimes ;
			if ( m_vData[pRet->nGameType].nSingleWinMost < pRet->nSingleWinMost )
			{
				m_vData[pRet->nGameType].nSingleWinMost = pRet->nSingleWinMost ;
			}

			if ( pRet->nPlayerTimes != 0 )
			{
				m_vData[pRet->nGameType].bDirty = true ;
			}

			// decrease halo weight 
			if ( GetPlayer()->GetBaseData()->getNewPlayerHaloWeight() >= pRet->nPlayerTimes )
			{
				GetPlayer()->GetBaseData()->setNewPlayerHalo(GetPlayer()->GetBaseData()->getNewPlayerHaloWeight() - pRet->nPlayerTimes );
			}
			else
			{
				GetPlayer()->GetBaseData()->setNewPlayerHalo(0);
			}

			CLogMgr::SharedLogMgr()->PrintLog("uid = %d do leave room final coin = %u",GetPlayer()->GetUserUID(), GetPlayer()->GetBaseData()->getCoin()) ;
			stMsg msg ;
			msg.usMsgType = MSG_PLAYER_UPDATE_MONEY ;
			GetPlayer()->GetBaseData()->OnMessage(&msg,ID_MSG_PORT_CLIENT) ;
		}
		break;
	case MSG_SVR_DELAYED_LEAVE_ROOM:
		{
			stMsgSvrDelayedLeaveRoom* pRet = (stMsgSvrDelayedLeaveRoom*)pMessage ;
			if ( isNotInAnyRoom() )
			{
				GetPlayer()->GetBaseData()->setCoin( pRet->nCoin + GetPlayer()->GetBaseData()->getCoin() ) ;
				CLogMgr::SharedLogMgr()->PrintLog("player not enter other room just uid = %d add coin = %lld, final = %u,",GetPlayer()->GetUserUID(),pRet->nCoin,GetPlayer()->GetBaseData()->getCoin()) ;
			}
			else
			{
				GetPlayer()->GetBaseData()->setTempCoin(GetPlayer()->GetBaseData()->getTempCoin() + pRet->nCoin) ;
				CLogMgr::SharedLogMgr()->PrintLog("player enter other room so uid = %d add temp = %lld, final = %u,",GetPlayer()->GetUserUID(),pRet->nCoin,GetPlayer()->GetBaseData()->getTempCoin(),GetPlayer()->GetBaseData()->getCoin() ) ;
			}

			m_vData[pRet->nGameType].nPlayTimes += pRet->nPlayerTimes ;
			m_vData[pRet->nGameType].nWinTimes += pRet->nWinTimes ;
			if ( m_vData[pRet->nGameType].nSingleWinMost < pRet->nSingleWinMost )
			{
				m_vData[pRet->nGameType].nSingleWinMost = pRet->nSingleWinMost ;
			}

			if ( pRet->nPlayerTimes != 0 )
			{
				m_vData[pRet->nGameType].bDirty = true ;
			}

			CLogMgr::SharedLogMgr()->PrintLog("uid = %d delay leave room coin = %u",GetPlayer()->GetUserUID(), GetPlayer()->GetBaseData()->getCoin()) ;
			stMsg msg ;
			msg.usMsgType = MSG_PLAYER_UPDATE_MONEY ;
			GetPlayer()->GetBaseData()->OnMessage(&msg,ID_MSG_PORT_CLIENT) ;
		}
		break;
	case MSG_READ_PLAYER_NIUNIU_DATA:
		{
			stMsgReadPlayerNiuNiuDataRet* pRet = (stMsgReadPlayerNiuNiuDataRet*)pMessage ;
			auto& gdata = m_vData[eRoom_NiuNiu] ;
			gdata.bDirty = false ;
			memcpy(&gdata,&pRet->tData,sizeof(pRet->tData)) ;
		}
		break;
	case MSG_READ_PLAYER_TAXAS_DATA:
		{
			stMsgReadPlayerTaxasDataRet* pRet = (stMsgReadPlayerTaxasDataRet*)pMessage ;
			if ( pRet->nRet )
			{
				CLogMgr::SharedLogMgr()->ErrorLog("uid = %d read taxas data error",pRet->nUserUID);
				return true ;
			}

			auto& gdata = m_vData[eRoom_TexasPoker] ;
			gdata.bDirty = false ;
			memcpy(&gdata,&pRet->tData,sizeof(pRet->tData)) ;
			CLogMgr::SharedLogMgr()->PrintLog("uid taxas data  , owner rooms = %d",m_vMyOwnRooms[eRoom_TexasPoker].size());
			sendGameDataToClient();
		}
		break;
	case MSG_REQUEST_MY_OWN_ROOMS:
		{
			stMsgRequestMyOwnRooms* pRet = (stMsgRequestMyOwnRooms*)pMessage ;
			if ( pRet->nRoomType >= eRoom_Max )
			{
				return false;
			}

			stMsgRequestMyOwnRoomsRet msgRet ;
			msgRet.nRoomType = pRet->nRoomType ;
			msgRet.nCnt = m_vMyOwnRooms[pRet->nRoomType].size() ;
			if ( msgRet.nCnt == 0 )
			{
				SendMsg(&msgRet,sizeof(msgRet)) ;
				return true ;
			}

			CAutoBuffer autoBuffer(sizeof(msgRet) + sizeof(uint32_t)* msgRet.nCnt);
			autoBuffer.addContent((char*)&msgRet,sizeof(msgRet)) ;
			MAP_ID_MYROOW::iterator iter = m_vMyOwnRooms[pRet->nRoomType].begin() ;
			for ( ; iter != m_vMyOwnRooms[pRet->nRoomType].end() ; ++iter )
			{
				uint32_t n = iter->first ;
				autoBuffer.addContent((char*)&n,sizeof(uint32_t));
			}
			SendMsg((stMsg*)autoBuffer.getBufferPtr(),autoBuffer.getContentSize()) ;
		}
		break;
	case MSG_READ_MY_OWN_ROOMS:
		{
			stMsgReadMyOwnRoomsRet* pRet = (stMsgReadMyOwnRoomsRet*)pMessage ;
			if ( pRet->nRoomType >= eRoom_Max )
			{
				return false;
			}
			stMyOwnRoom* pRoomPtr = (stMyOwnRoom*)((char*)pMessage + sizeof(stMsgReadMyOwnRoomsRet));
			while ( pRet->nCnt-- )
			{
				m_vMyOwnRooms[pRet->nRoomType].insert(MAP_ID_MYROOW::value_type(pRoomPtr->nRoomID,*pRoomPtr));
				++pRoomPtr ;
			}
			CLogMgr::SharedLogMgr()->PrintLog("uid = %d ,read own creator room" , GetPlayer()->GetUserUID() ) ;
		}
		break;
	//case MSG_ADD_RENT_TIME:
	//	{
	//		stMsgAddRoomRentTime* pRet = (stMsgAddRoomRentTime*)pMessage ;
	//		CLogMgr::SharedLogMgr()->ErrorLog("MSG_TP_ADD_RENT_TIME check room id , and kou qian  do not forget ");

	//		stMsgAddRoomRentTimeRet msgBack ;
	//		msgBack.nAddDays = pRet->nAddDays ;
	//		msgBack.nRet = 0 ;
	//		msgBack.nRoomID = pRet->nRoomID ;
	//		msgBack.nRoomType = pRet->nRoomType ;

	//		uint16_t nRoomConfigID = 0 ;
	//		if ( eRoom_Max > pRet->nRoomType )
	//		{
	//			nRoomConfigID = getMyOwnRoomConfig((eRoomType)pRet->nRoomType,pRet->nRoomID) ;
	//		}
	//		else
	//		{
	//			msgBack.nRet = 3 ;
	//			SendMsg(&msgBack,sizeof(msgBack)) ;
	//			return true ;
	//		}

	//		CRoomConfigMgr* pConfigMgr = (CRoomConfigMgr*)CGameServerApp::SharedGameServerApp()->GetConfigMgr()->GetConfig(CConfigManager::eConfig_Room);
	//		stBaseRoomConfig* pRoomConfig = pConfigMgr->GetConfigByConfigID(nRoomConfigID) ;
	//		if ( pRoomConfig == nullptr )
	//		{
	//			msgBack.nRet = 1 ;
	//			SendMsg(&msgBack,sizeof(msgBack)) ;
	//			return true ;
	//		}

	//		if ( GetPlayer()->GetBaseData()->getCoin() < pRet->nAddDays * pRoomConfig->nRentFeePerDay )
	//		{
	//			msgBack.nRet = 2 ;
	//			SendMsg(&msgBack,sizeof(msgBack)) ;
	//			return true ;
	//		}

	//		GetPlayer()->GetBaseData()->decressMoney(pRet->nAddDays * pRoomConfig->nRentFeePerDay);

	//		stMsgCrossServerRequest msgRoomProfitReq ;
	//		msgRoomProfitReq.cSysIdentifer = CPlayer::getMsgPortByRoomType(pRet->nRoomType) ;
	//		msgRoomProfitReq.nReqOrigID = GetPlayer()->GetUserUID() ;
	//		msgRoomProfitReq.nTargetID = pRet->nRoomID ;
	//		msgRoomProfitReq.nRequestType = eCrossSvrReq_AddRentTime ;
	//		msgRoomProfitReq.nRequestSubType = eCrossSvrReqSub_Default ;
	//		msgRoomProfitReq.vArg[0] = pRet->nAddDays ;
	//		msgRoomProfitReq.vArg[1] = pRet->nRoomType ;
	//		msgRoomProfitReq.vArg[2] = pRet->nAddDays * pRoomConfig->nRentFeePerDay ;
	//		SendMsg(&msgRoomProfitReq,sizeof(msgRoomProfitReq)) ;
	//	}
	//	break;
	/*case MSG_CACULATE_ROOM_PROFILE:
		{
			stMsgCaculateTaxasRoomProfit* pRet = (stMsgCaculateTaxasRoomProfit*)pMessage ;

			stMsgCaculateTaxasRoomProfitRet msgBack ;
			msgBack.nRoomType = pRet->nRoomType ;
			msgBack.bDiamond = false ;
			msgBack.nProfitMoney = 0 ;
			msgBack.nRet = 0 ;

			msgBack.nRoomID = pRet->nRoomID ;
			bool bIsOwnRoom = false ;
			if ( eRoom_Max > pRet->nRoomType )
			{
				bIsOwnRoom = isRoomIDMyOwn((eRoomType)pRet->nRoomType,pRet->nRoomID) ;
			}
			else
			{
				msgBack.nRet = 2 ;
				SendMsg(&msgBack,sizeof(msgBack)) ;
				return true ;
			}

			if ( !bIsOwnRoom )
			{
				msgBack.nRet = 1 ;
				SendMsg(&msgBack,sizeof(msgBack)) ;
				return true ;
			}

			stMsgCrossServerRequest msgRoomProfitReq ;
			msgRoomProfitReq.cSysIdentifer = CPlayer::getMsgPortByRoomType(pRet->nRoomType) ;
			msgRoomProfitReq.nReqOrigID = GetPlayer()->GetUserUID() ;
			msgRoomProfitReq.nTargetID = pRet->nRoomID ;
			msgRoomProfitReq.nRequestType = eCrossSvrReq_RoomProfit ;
			msgRoomProfitReq.nRequestSubType = eCrossSvrReqSub_Default ;
			SendMsg(&msgRoomProfitReq,sizeof(msgRoomProfitReq)) ;
			return true;
		}
		break;*/
	case MSG_DELETE_ROOM:
		{
			stMsgDeleteRoomRet msgBack ;
			stMsgDeleteRoom* pRet = (stMsgDeleteRoom*)pMessage ;
			msgBack.nRoomID = pRet->nRoomID ;
			msgBack.nRoomType = pRet->nRoomType ;
			bool bDeleted = false ;
			if ( eRoom_Max > pRet->nRoomType )
			{
				bDeleted = deleteOwnRoom((eRoomType)pRet->nRoomType,pRet->nRoomID) ;
			}
			else
			{
				msgBack.nRet = 2 ;
				SendMsg(&msgBack,sizeof(msgBack)) ;
				return true ;
			}

			if ( !bDeleted )
			{
				msgBack.nRet = 1 ;
				SendMsg(&msgBack,sizeof(msgBack)) ;
				return true ;
			}

			// do delete room ;
			stMsgCrossServerRequest msgReq ;
			msgReq.nJsonsLen = 0 ;
			msgReq.cSysIdentifer = CPlayer::getMsgPortByRoomType(pRet->nRoomType) ;
			msgReq.nReqOrigID = GetPlayer()->GetUserUID() ;
			msgReq.nRequestSubType = eCrossSvrReqSub_Default;
			msgReq.nRequestType = eCrossSvrReq_DeleteRoom ;
			msgReq.nTargetID = 0 ;
			memset(msgReq.vArg,0,sizeof(msgReq.vArg)) ;
			msgReq.vArg[0] = pRet->nRoomType ;
			msgReq.vArg[1] = pRet->nRoomID ;
			CGameServerApp::SharedGameServerApp()->sendMsg(msgReq.nReqOrigID, (char*)&msgReq,sizeof(msgReq)) ;
			CLogMgr::SharedLogMgr()->PrintLog("uid = %d , delete room = %d , type = %d",GetPlayer()->GetUserUID(),pRet->nRoomID,pRet->nRoomType) ;

			msgBack.nRet = 0 ;
			SendMsg(&msgBack,sizeof(msgBack)) ;
		}
		break;
	case MSG_CREATE_ROOM:
		{
			stMsgCreateRoom* pRet = (stMsgCreateRoom*)pMessage ;

			stMsgCreateRoomRet msgBack ;
			msgBack.nRoomID = 0 ;
			msgBack.nRoomType = pRet->nRoomType ;
			msgBack.nFinalCoin = GetPlayer()->GetBaseData()->getCoin() ;

			CRoomConfigMgr* pConfigMgr = (CRoomConfigMgr*)CGameServerApp::SharedGameServerApp()->GetConfigMgr()->GetConfig(CConfigManager::eConfig_Room);
			stTaxasRoomConfig* pRoomConfig = (stTaxasRoomConfig*)pConfigMgr->GetConfigByConfigID(pRet->nConfigID) ;
			if ( pRoomConfig == nullptr )
			{
				msgBack.nRet = 1 ;
				msgBack.nRoomID = 0 ;
				SendMsg(&msgBack,sizeof(msgBack)) ;
				return true ;
			}

			// check if create room count reach limit ;
			bool bReachLimit = false ;
			if ( eRoom_Max > pRet->nRoomType )
			{
				bReachLimit = isCreateRoomCntReachLimit((eRoomType)pRet->nRoomType) ;
			}
			else
			{
				CLogMgr::SharedLogMgr()->ErrorLog("add my own room , unknown room type = %d , uid = %d",msgBack.nRoomType,GetPlayer()->GetUserUID()) ;
				msgBack.nRet = 1 ;
				msgBack.nRoomID = 0 ;
				SendMsg(&msgBack,sizeof(msgBack)) ;
				return true ;
			}

			if ( bReachLimit && GetPlayer()->GetUserUID() != MATCH_MGR_UID )
			{
				msgBack.nRet = 5 ;
				msgBack.nRoomID = 0 ;
				SendMsg(&msgBack,sizeof(msgBack)) ;
				return true ;
			}

			// check coin weather engough 
			if ( GetPlayer()->GetBaseData()->getCoin() < pRoomConfig->nRentFeePerDay * pRet->nMinites )
			{
				msgBack.nRet = 4 ;
				msgBack.nRoomID = 0 ;
				SendMsg(&msgBack,sizeof(msgBack)) ;
				return true ;
			}

			stMsgCrossServerRequest msgReq ;
			msgReq.cSysIdentifer = CPlayer::getMsgPortByRoomType(pRet->nRoomType) ;
			if ( msgReq.cSysIdentifer == ID_MSG_PORT_NONE )
			{
				msgBack.nRet = 6 ;
				SendMsg(&msgBack,sizeof(msgBack)) ;
				CLogMgr::SharedLogMgr()->ErrorLog("crate room but , unknown room type = %d , uid = %d",pRet->nRoomType, GetPlayer()->GetUserUID() ) ;
				return true ;
			}

			GetPlayer()->GetBaseData()->decressMoney(pRoomConfig->nRentFeePerDay * pRet->nMinites );

			msgReq.nReqOrigID = GetPlayer()->GetUserUID() ;
			msgReq.nRequestSubType = eCrossSvrReqSub_Default;
			msgReq.nRequestType = eCrossSvrReq_CreateRoom ;
			msgReq.nTargetID = 0 ;
			msgReq.vArg[0] = pRet->nConfigID ;
			msgReq.vArg[1] = pRet->nMinites ;
			msgReq.vArg[2] = pRet->nRoomType ;
			pRet->vRoomName[MAX_LEN_ROOM_NAME-1] = 0 ;

			Json::Value vArg ;
			vArg["roonName"] = pRet->vRoomName;
			if (msgReq.nReqOrigID == MATCH_MGR_UID )
			{
				vArg["subRoomCnt"] = 8 ;
			}
			CON_REQ_MSG_JSON(msgReq,vArg,autoBuf) ;
			CGameServerApp::SharedGameServerApp()->sendMsg(msgReq.nReqOrigID,autoBuf.getBufferPtr(),autoBuf.getContentSize()) ;
		}
		break;
	default:
		return false;
	}
	return true ;
}

bool CPlayerGameData::onCrossServerRequest(stMsgCrossServerRequest* pRequest , eMsgPort eSenderPort,Json::Value* vJsValue)
{
	if ( IPlayerComponent::onCrossServerRequest(pRequest,eSenderPort,vJsValue) )
	{
		return true ;
	}

	switch ( pRequest->nRequestType )
	{
	case eCrossSvrReq_LeaveRoomRet:
		{
			m_nStateInRoomType = eRoom_Max ;
			m_nStateInRoomID = 0 ;
			GetPlayer()->GetBaseData()->setCoin(GetPlayer()->GetBaseData()->getTempCoin() + GetPlayer()->GetBaseData()->getCoin()) ;
			GetPlayer()->GetBaseData()->setTempCoin(0) ;
			CLogMgr::SharedLogMgr()->ErrorLog("uid = %d leave room state error ",GetPlayer()->GetUserUID() ) ;
		}
		break;
	default:
		return false ;
	}
	return true ;
}

bool CPlayerGameData::onCrossServerRequestRet(stMsgCrossServerRequestRet* pResult,Json::Value* vJsValue )
{
	if ( IPlayerComponent::onCrossServerRequestRet(pResult,vJsValue) )
	{
		return true ;
	}

	if ( eCrossSvrReq_CreateRoom == pResult->nRequestType  )
	{
		stMsgCreateRoomRet msgBack ;
		msgBack.nRet = pResult->nRet ;
		msgBack.nRoomID = pResult->vArg[1];
		msgBack.nRoomType = pResult->vArg[2] ;
		msgBack.nFinalCoin = GetPlayer()->GetBaseData()->GetAllCoin() ;
		if ( pResult->nRet == 0 )
		{
			if ( eRoom_Max > msgBack.nRoomType )
			{
				addOwnRoom((eRoomType)msgBack.nRoomType,msgBack.nRoomID,pResult->vArg[0]) ;
			}
			else
			{
				CLogMgr::SharedLogMgr()->ErrorLog("add my own room , unknown room type = %d , uid = %d",msgBack.nRoomType,GetPlayer()->GetUserUID()) ;
			}
			CLogMgr::SharedLogMgr()->PrintLog("uid = %d , create room id = %d , config id = %d", GetPlayer()->GetUserUID(),msgBack.nRoomID,(int32_t)pResult->vArg[0] ) ;
		}
		else
		{
			CLogMgr::SharedLogMgr()->PrintLog("result create failed give back coin uid = %d",GetPlayer()->GetUserUID());

			CRoomConfigMgr* pConfigMgr = (CRoomConfigMgr*)CGameServerApp::SharedGameServerApp()->GetConfigMgr()->GetConfig(CConfigManager::eConfig_Room);

			stTaxasRoomConfig* pRoomConfig = (stTaxasRoomConfig*)pConfigMgr->GetConfigByConfigID(pResult->vArg[0]) ;
			if ( pRoomConfig == nullptr )
			{
				CLogMgr::SharedLogMgr()->ErrorLog("fuck arument error must fix now , room config id , can not find") ;
				return true ;
			}

			GetPlayer()->GetBaseData()->AddMoney( pRoomConfig->nRentFeePerDay *  pResult->vArg[3]);
			msgBack.nFinalCoin = GetPlayer()->GetBaseData()->getCoin() ;
		}
		SendMsg(&msgBack,sizeof(msgBack)) ;
		return true ;
	}

	if ( eCrossSvrReq_RoomProfit == pResult->nRequestType )
	{
		stMsgCaculateRoomProfitRet msgBack ;
		msgBack.nRoomID = pResult->nReqOrigID ;
		msgBack.bDiamond = !pResult->vArg[0] ;
		msgBack.nProfitMoney = pResult->vArg[1] ;
		msgBack.nRoomType = pResult->vArg[2] ;
		msgBack.nRet = pResult->nRet ? 3 : 0 ;
		SendMsg(&msgBack,sizeof(msgBack)) ;
		if ( msgBack.nRet )
		{
			return true ;
		}
		GetPlayer()->GetBaseData()->AddMoney(msgBack.nProfitMoney,msgBack.bDiamond);
		// save log 
		stMsgSaveLog msgLog ;
		memset(msgLog.vArg,0,sizeof(msgLog.vArg));
		msgLog.nJsonExtnerLen = 0 ;
		msgLog.nLogType = eLog_AddMoney ;
		msgLog.nTargetID = GetPlayer()->GetUserUID() ;
		msgLog.vArg[0] = !msgBack.bDiamond ;
		msgLog.vArg[1] = msgBack.nProfitMoney;
		msgLog.vArg[2] = GetPlayer()->GetBaseData()->GetData()->nCoin;
		msgLog.vArg[3] = GetPlayer()->GetBaseData()->GetData()->nDiamoned ;
		msgLog.vArg[4] = eCrossSvrReq_RoomProfit ;
		msgLog.vArg[5] = pResult->nReqOrigID ;
		CGameServerApp::SharedGameServerApp()->sendMsg(pResult->nReqOrigID,(char*)&msgLog,sizeof(msgLog));

		CLogMgr::SharedLogMgr()->PrintLog("uid = %d get profit = %llu",GetPlayer()->GetUserUID(),msgBack.nProfitMoney) ;
		return true ;
	}

	if ( eCrossSvrReq_AddRentTime == pResult->nRequestType )
	{
		stMsgAddRoomRentTimeRet msgRet ;
		msgRet.nRet = pResult->nRet ? 4 : 0  ;
		msgRet.nAddDays = pResult->vArg[0] ;
		msgRet.nRoomID = pResult->nReqOrigID ;
		msgRet.nRoomType = pResult->vArg[1] ;
		SendMsg(&msgRet,sizeof(msgRet)) ;
		if ( msgRet.nRet )
		{
			GetPlayer()->GetBaseData()->AddMoney(pResult->vArg[2]) ;
		}
		CLogMgr::SharedLogMgr()->PrintLog("uid = %d add rent time = %d",GetPlayer()->GetUserUID(),msgRet.nAddDays) ;
		return true ;
	}
	return false ;
}

void CPlayerGameData::OnPlayerDisconnect()
{
	IPlayerComponent::OnPlayerDisconnect();
	if ( isNotInAnyRoom() == false )
	{
		stMsgCrossServerRequest msgEnter ;
		msgEnter.cSysIdentifer = GetPlayer()->getMsgPortByRoomType(m_nStateInRoomType) ;
		msgEnter.nJsonsLen = 0 ;
		msgEnter.nReqOrigID = GetPlayer()->GetUserUID();
		msgEnter.nRequestSubType = eCrossSvrReqSub_Default ;
		msgEnter.nRequestType = eCrossSvrReq_ApplyLeaveRoom ;
		msgEnter.nTargetID = m_nStateInRoomID ;
		msgEnter.vArg[0] = m_nStateInRoomID ;
		msgEnter.vArg[1] = GetPlayer()->GetSessionID() ;
		msgEnter.vArg[2] = m_nStateInRoomType ;
		SendMsg(&msgEnter,sizeof(msgEnter)) ;
		CLogMgr::SharedLogMgr()->PrintLog("uid = %d disconnected , apply to leave room id = %d ",GetPlayer()->GetUserUID(),m_nStateInRoomID) ;
		GetPlayer()->delayDelete();
	}
}

void CPlayerGameData::OnOtherWillLogined()
{
	IPlayerComponent::OnOtherWillLogined();
	if ( isNotInAnyRoom() == false )
	{
		stMsgCrossServerRequest msgEnter ;
		msgEnter.cSysIdentifer = GetPlayer()->getMsgPortByRoomType(m_nStateInRoomType) ;
		msgEnter.nJsonsLen = 0 ;
		msgEnter.nReqOrigID = GetPlayer()->GetUserUID();
		msgEnter.nRequestSubType = eCrossSvrReqSub_Default ;
		msgEnter.nRequestType = eCrossSvrReq_ApplyLeaveRoom ;
		msgEnter.nTargetID = m_nStateInRoomID ;
		msgEnter.vArg[0] = m_nStateInRoomID ;
		msgEnter.vArg[1] = GetPlayer()->GetSessionID() ;
		SendMsg(&msgEnter,sizeof(msgEnter)) ;

		CLogMgr::SharedLogMgr()->PrintLog("uid = %d other device login , apply to leave room id = %d ",GetPlayer()->GetUserUID(),m_nStateInRoomID) ;
	}
}

void CPlayerGameData::TimerSave()
{
	for ( uint8_t nIdx = eRoom_None; nIdx < eRoom_Max ; ++nIdx )
	{
		auto& gameData = m_vData[nIdx] ;
		if ( gameData.bDirty == false )
		{
			continue;
		}

		gameData.bDirty = false ;
		
		stMsgSavePlayerGameData msgSave ;
		msgSave.nGameType = nIdx ;
		msgSave.nUserUID = GetPlayer()->GetUserUID() ;
		memcpy((char*)&msgSave.tData,&gameData,sizeof(msgSave.tData));
		SendMsg(&msgSave,sizeof(msgSave)) ;
	}
}

void CPlayerGameData::addOwnRoom(eRoomType eType , uint32_t nRoomID , uint16_t nConfigID )
{
	if ( eType >= eRoom_Max )
	{
		return ;
	}
	stMyOwnRoom myroom ;
	myroom.nRoomID = nRoomID ;
	m_vMyOwnRooms[eType].insert(MAP_ID_MYROOW::value_type(myroom.nRoomID,myroom));
}

bool CPlayerGameData::isCreateRoomCntReachLimit(eRoomType eType)
{
	if ( GetPlayer()->GetUserUID() == MATCH_MGR_UID )
	{
		return false ;
	}

	if ( eType >= eRoom_Max )
	{
		return true;
	}

	return m_vMyOwnRooms[eType].size() >= 1 ;
}

bool CPlayerGameData::deleteOwnRoom(eRoomType eType , uint32_t nRoomID )
{
	if ( eType >= eRoom_Max )
	{
		return false;
	}

	auto iter = m_vMyOwnRooms[eType].find(nRoomID) ;
	if ( iter != m_vMyOwnRooms[eType].end() )
	{
		m_vMyOwnRooms[eType].erase(iter) ;
		return true ;
	}
	return false ;
}

//uint16_t CPlayerGameData::getMyOwnRoomConfig( eRoomType eType , uint32_t nRoomID ) 
//{
//	if ( eType >= eRoom_Max )
//	{
//		return 0;
//	}
//
//	MAP_ID_MYROOW::iterator iter = m_vMyOwnRooms[eType].find(nRoomID) ;
//	if ( iter != m_vMyOwnRooms[eType].end() )
//	{
//		return iter->second.nConfigID ;
//	}
//	return 0 ;
//}

bool CPlayerGameData::isRoomIDMyOwn(eRoomType eType , uint32_t nRoomID)
{
	if ( eType >= eRoom_Max )
	{
		return false;
	}
	MAP_ID_MYROOW::iterator iter = m_vMyOwnRooms[eType].find(nRoomID) ;
	return iter != m_vMyOwnRooms[eType].end() ;
}

void CPlayerGameData::sendGameDataToClient()
{
	// send niuniu data ;
	stMsgPlayerBaseDataNiuNiu msg ;
	memcpy(&msg.tNiuNiuData,&m_vData[eRoom_NiuNiu],sizeof(msg.tNiuNiuData));
	SendMsg(&msg,sizeof(msg)) ;

	// send taxas data 
	stMsgPlayerBaseDataTaxas msgT ;
	memcpy(&msgT.tTaxasData,&m_vData[eRoom_TexasPoker],sizeof(msgT.tTaxasData));
	SendMsg(&msgT,sizeof(msgT)) ;
}
