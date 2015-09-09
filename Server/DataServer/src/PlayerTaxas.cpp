#include "PlayerTaxas.h"
#include "Player.h"
#include "LogManager.h"
#include "GameServerApp.h"
#include "PlayerBaseData.h"
#include <json/json.h>
#include "AutoBuffer.h"
void CPlayerTaxas::Reset()
{
	IPlayerComponent::Reset();
	m_nCurTaxasRoomID = 0 ;
	m_bDirty = false ;
	memset(&m_tData,0,sizeof(m_tData));
	m_vMyOwnRooms.clear();
	m_vFollowedRooms.clear();

	stMsgReadPlayerTaxasData msg ;
	msg.nUserUID = GetPlayer()->GetUserUID() ;
	SendMsg(&msg,sizeof(msg)) ;
}

void CPlayerTaxas::Init()
{
	IPlayerComponent::Init();
	m_eType = ePlayerComponent_PlayerTaxas ;
	m_nCurTaxasRoomID = 0 ;
	m_bDirty = false ;
	memset(&m_tData,0,sizeof(m_tData));
	m_vMyOwnRooms.clear();
	m_vFollowedRooms.clear();

	stMsgReadPlayerTaxasData msg ;
	msg.nUserUID = GetPlayer()->GetUserUID() ;
	SendMsg(&msg,sizeof(msg)) ;
	CLogMgr::SharedLogMgr()->PrintLog("requesting player taxas data for uid = %d",msg.nUserUID);
}

bool CPlayerTaxas::OnMessage( stMsg* pMessage , eMsgPort eSenderPort)
{
	if ( IPlayerComponent::OnMessage(pMessage,eSenderPort) )
	{
		return true ;
	}

	switch (pMessage->usMsgType)
	{
	case MSG_READ_PLAYER_TAXAS_DATA:
		{
			stMsgReadPlayerTaxasDataRet* pRet = (stMsgReadPlayerTaxasDataRet*)pMessage ;
			if ( pRet->nRet )
			{
				CLogMgr::SharedLogMgr()->ErrorLog("uid = %d read taxas data error",pRet->nUserUID);
				return true ;
			}

			memcpy(&m_tData,&pRet->tData,sizeof(m_tData));
			char* pBuffer = (char*)pRet ;
			pBuffer += sizeof(stMsgReadPlayerTaxasDataRet);
			if ( pRet->nFollowedRoomsStrLen > 0 )
			{
				Json::Reader reader ;
				Json::Value arrayValue ;
				reader.parse(pBuffer,pBuffer + pRet->nFollowedRoomsStrLen,arrayValue,false);
				for ( uint16_t nIdx = 0 ; nIdx < arrayValue.size(); ++nIdx )
				{
					m_vFollowedRooms.insert(arrayValue[nIdx].asUInt());
				}
			}

			pBuffer += pRet->nFollowedRoomsStrLen ;
			
			if ( pRet->nMyOwnRoomsStrLen > 0 )
			{
				Json::Reader reader ;
				Json::Value arrayValue ;
				reader.parse(pBuffer,pBuffer + pRet->nMyOwnRoomsStrLen,arrayValue,false);
				for ( uint16_t nIdx = 0 ; nIdx < arrayValue.size(); ++nIdx )
				{
					m_vMyOwnRooms.insert(arrayValue[nIdx].asUInt());
				}
			}
			
			CLogMgr::SharedLogMgr()->PrintLog("uid taxas data followed rooms = %d , owner rooms = %d",m_vFollowedRooms.size(),m_vMyOwnRooms.size());
		}
		break;
	case MSG_TP_REQUEST_PLAYER_DATA:
		{
			stMsgRequestTaxasPlayerData* pData = (stMsgRequestTaxasPlayerData*)pMessage;
			stMsgRequestTaxasPlayerDataRet msgBack ;
			msgBack.nRet = 0 ;
			msgBack.tData.nSessionID = GetPlayer()->GetSessionID() ;
			if ( m_nCurTaxasRoomID )
			{
				msgBack.nRet = 2 ;
				CGameServerApp::SharedGameServerApp()->sendMsg(pData->nRoomID,(char*)&msgBack,sizeof(msgBack)) ;
				CLogMgr::SharedLogMgr()->ErrorLog("can not enter room already in other room id = %d  session id = %d",m_nCurTaxasRoomID,GetPlayer()->GetSessionID() ) ;

				// may have error  order leave 
				stMsgOrderTaxasPlayerLeave msg ;
				msg.nRoomID = m_nCurTaxasRoomID ;
				msg.nSessionID = GetPlayer()->GetSessionID();
				msg.nUserUID = GetPlayer()->GetUserUID();
				CGameServerApp::SharedGameServerApp()->sendMsg(m_nCurTaxasRoomID,(char*)&msg,sizeof(msg)) ;
				return true ;
			}

			m_nCurTaxasRoomID = pData->nRoomID ;
			CPlayer* pPlayer = GetPlayer();
			msgBack.tData.nPhotoID = pPlayer->GetBaseData()->GetPhotoID();
			memset(msgBack.tData.cName,0,sizeof(msgBack.tData.cName) );
			sprintf_s(msgBack.tData.cName,sizeof(msgBack.tData.cName),"%s",pPlayer->GetBaseData()->GetPlayerName()) ;
			msgBack.tData.nSex = pPlayer->GetBaseData()->GetSex();
			msgBack.tData.nUserUID = pPlayer->GetUserUID();
			msgBack.tData.nVipLevel = pPlayer->GetBaseData()->GetVipLevel() ;
			CGameServerApp::SharedGameServerApp()->sendMsg(m_nCurTaxasRoomID,(char*)&msgBack,sizeof(msgBack)) ;
		}
		break;
	case MSG_TP_SYNC_PLAYER_DATA:
		{
			stMsgSyncTaxasPlayerData* pRet = (stMsgSyncTaxasPlayerData*)pMessage ;
			m_tData.nPlayTimes = pRet->nPlayTimes ;
			m_tData.nSingleWinMost = pRet->nSingleWinMost ;
			m_tData.nWinTimes = pRet->nWinTimes ;
			CLogMgr::SharedLogMgr()->PrintLog("be told uid = %d sys player data ",GetPlayer()->GetUserUID());
			m_bDirty = true ;
		}
		break;
	case MSG_TP_ORDER_LEAVE:
		{
			stMsgOrderTaxasPlayerLeaveRet* pRet = (stMsgOrderTaxasPlayerLeaveRet*)pMessage ;
			// if success , we will recived inform leave , if failed just recieved this msg 
			if ( pRet->nRet )
			{
				CLogMgr::SharedLogMgr()->ErrorLog("order leave failed uid = %d",GetPlayer()->GetUserUID());
			}
		}
		break;
	case MSG_TP_INFORM_LEAVE:
		{
			m_nCurTaxasRoomID = 0 ;
			CLogMgr::SharedLogMgr()->PrintLog("be told uid = %d leave taxas room ",GetPlayer()->GetUserUID());
		}
		break;
	case MSG_TP_CREATE_ROOM:
		{
			stMsgCreateTaxasRoomRet msgBack ;
			stMsgCreateTaxasRoom* pRet = (stMsgCreateTaxasRoom*)pMessage ;
			//stBaseRoomConfig* pRoomConfig = CTaxasServerApp::SharedGameServerApp()->GetConfigMgr()->GetRoomConfig(eRoom_TexasPoker,pRet->nConfigID);
			CLogMgr::SharedLogMgr()->ErrorLog("create room pls kou qian, bu neng chongfu , wan cheng yi ge , then next var[1] var[2]");
			
			//if ( pRoomConfig == nullptr )
			//{
			//	msgBack.nRet = 1 ;
			//	msgBack.nRoomID = 0 ;
			//	SendMsg(&msgBack,sizeof(msgBack),nSessionID) ;
			//	return true ;
			//}
			//else
			//{
			//	stMsgPlayerRequestCoin msgReqMoney ;
			//	msgReqMoney.bIsDiamond = false ;
			//	msgReqMoney.nAtLeast = 0 ;
			//	msgReqMoney.nWantMoney = pRoomConfig->nCreateFee ;
			//	msgReqMoney.nSessionID = nSessionID ;
			//	msgReqMoney.nReqType = eReqMoney_CreateRoom;
			//	msgReqMoney.nUserUID = 1 ;
			//	msgReqMoney.nBackArg[0] = nSessionID ;
			//	msgReqMoney.nBackArg[1] = pRet->nConfigID ;
			//	SendMsg(&msgReqMoney,sizeof(msgReqMoney),nSessionID) ;
			//}
			stMsgCrossServerRequest msgReq ;
			msgReq.cSysIdentifer = ID_MSG_PORT_TAXAS ;
			msgReq.nReqOrigID = GetPlayer()->GetUserUID() ;
			msgReq.nRequestSubType = eCrossSvrReqSub_Default;
			msgReq.nRequestType = eCrossSvrReq_CreateTaxasRoom ;
			msgReq.nTargetID = 0 ;
			msgReq.vArg[0] = pRet->nConfigID ;
			
			Json::Value vArg ;
			vArg["roonName"] = pRet->vRoomName;
			CON_REQ_MSG_JSON(msgReq,vArg,autoBuf) ;
			CGameServerApp::SharedGameServerApp()->sendMsg(msgReq.nReqOrigID,autoBuf.getBufferPtr(),autoBuf.getContentSize()) ;
		}
		break;
	case MSG_REQUEST_MY_FOLLOW_ROOMS:
		{
			stMsgRequestMyFollowRoomsRet msgRet ;
			msgRet.nCnt = m_vFollowedRooms.size() ;
			if ( msgRet.nCnt == 0 )
			{
				SendMsg(&msgRet,sizeof(msgRet)) ;
				return true ;
			}

			CAutoBuffer autoBuffer(sizeof(msgRet) + sizeof(uint32_t)* msgRet.nCnt);
			autoBuffer.addContent((char*)&msgRet,sizeof(msgRet)) ;
			SET_ROOM_ID::iterator iter = m_vFollowedRooms.begin() ;
			for ( ; iter != m_vFollowedRooms.end() ; ++iter )
			{
				uint32_t n = *iter ;
				autoBuffer.addContent((char*)&n,sizeof(uint32_t));
			}
			SendMsg((stMsg*)autoBuffer.getBufferPtr(),autoBuffer.getContentSize()) ;
		}
		break;
	case MSG_REQUEST_MY_OWN_ROOMS:
		{
			stMsgRequestMyOwnRoomsRet msgRet ;
			msgRet.nCnt = m_vMyOwnRooms.size() ;
			if ( msgRet.nCnt == 0 )
			{
				SendMsg(&msgRet,sizeof(msgRet)) ;
				return true ;
			}

			CAutoBuffer autoBuffer(sizeof(msgRet) + sizeof(uint32_t)* msgRet.nCnt);
			autoBuffer.addContent((char*)&msgRet,sizeof(msgRet)) ;
			SET_ROOM_ID::iterator iter = m_vMyOwnRooms.begin() ;
			for ( ; iter != m_vMyOwnRooms.end() ; ++iter )
			{
				uint32_t n = *iter ;
				autoBuffer.addContent((char*)&n,sizeof(uint32_t));
			}
			SendMsg((stMsg*)autoBuffer.getBufferPtr(),autoBuffer.getContentSize()) ;
		}
		break;
	default:
		return false;
	}

	return true ;
}

bool CPlayerTaxas::onCrossServerRequest(stMsgCrossServerRequest* pRequest , eMsgPort eSenderPort,Json::Value* vJsValue )
{
	if ( IPlayerComponent::onCrossServerRequest(pRequest,eSenderPort,vJsValue) )
	{
		return true ;
	}
	return false ;
}

bool CPlayerTaxas::onCrossServerRequestRet(stMsgCrossServerRequestRet* pResult,Json::Value* vJsValue )
{
	if ( IPlayerComponent::onCrossServerRequestRet(pResult,vJsValue) )
	{
		return true ;
	}

	if ( eCrossSvrReq_CreateTaxasRoom == pResult->nRequestType  )
	{
		stMsgCreateTaxasRoomRet msgBack ;
		msgBack.nRet = pResult->nRet ;
		msgBack.nRoomID = pResult->vArg[1];
		SendMsg(&msgBack,sizeof(msgBack)) ;
		if ( pResult->nRet == 0 )
		{
			m_vMyOwnRooms.insert(msgBack.nRoomID);
			m_bDirty = true ;
		}
		else
		{
			CLogMgr::SharedLogMgr()->ErrorLog("create failed give back coin uid = %d",GetPlayer()->GetUserUID());
		}
		
		CLogMgr::SharedLogMgr()->PrintLog("uid = %d create room ret = %d",GetPlayer()->GetSessionID(),msgBack.nRet);
		return true ;
	}
	return false ;
}

void CPlayerTaxas::OnPlayerDisconnect()
{
	IPlayerComponent::OnPlayerDisconnect();
	if ( m_nCurTaxasRoomID != 0 )  // order to leave 
	{
		stMsgOrderTaxasPlayerLeave msgLeave ;
		msgLeave.nRoomID = m_nCurTaxasRoomID ;
		msgLeave.nSessionID = GetPlayer()->GetSessionID();
		msgLeave.nUserUID = GetPlayer()->GetUserUID();
		CGameServerApp::SharedGameServerApp()->sendMsg(m_nCurTaxasRoomID,(char*)&msgLeave,sizeof(msgLeave) ) ;
		m_nCurTaxasRoomID = 0 ;
	}
	TimerSave();
}

void CPlayerTaxas::OnOtherWillLogined()
{
	IPlayerComponent::OnOtherWillLogined();
	if ( m_nCurTaxasRoomID != 0 )  // order to leave 
	{
		stMsgOrderTaxasPlayerLeave msgLeave ;
		msgLeave.nRoomID = m_nCurTaxasRoomID ;
		msgLeave.nSessionID = GetPlayer()->GetSessionID();
		msgLeave.nUserUID = GetPlayer()->GetUserUID();
		CGameServerApp::SharedGameServerApp()->sendMsg(m_nCurTaxasRoomID,(char*)&msgLeave,sizeof(msgLeave) ) ;
		m_nCurTaxasRoomID = 0 ;
	}
}

void CPlayerTaxas::TimerSave()
{
	IPlayerComponent::TimerSave();
	if ( !m_bDirty )
	{
		return ;
	}
	m_bDirty = false ;

	stMsgSavePlayerTaxaPokerData msgSavePokerData ;
	msgSavePokerData.nUserUID = GetPlayer()->GetUserUID() ;
	memcpy(&msgSavePokerData.tData,&m_tData,sizeof(m_tData));
	msgSavePokerData.nUserUID = GetPlayer()->GetUserUID() ;
	msgSavePokerData.nFollowedRoomsStrLen = 0 ;
	msgSavePokerData.nMyOwnRoomsStrLen = 0 ;
	if ( m_vFollowedRooms.empty() && m_vMyOwnRooms.empty() )
	{
		SendMsg((stMsgSavePlayerMoney*)&msgSavePokerData,sizeof(msgSavePokerData)) ;
		return ;
	}

	std::string strFollow = "";
	std::string strMyOwn = "";
	if ( !m_vFollowedRooms.empty() )
	{
		Json::Value followArray ;
		Json::StyledWriter writeFollow ;

		SET_ROOM_ID::iterator iter = m_vFollowedRooms.begin() ;
		uint16_t nIdx = 0 ;
		for ( ; iter != m_vFollowedRooms.end(); ++iter ,++nIdx)
		{
			followArray[nIdx] = *iter ;
		}
		strFollow = writeFollow.write(followArray) ;
	}

	if ( !m_vMyOwnRooms.empty() )
	{
		Json::Value ownArray ;
		Json::StyledWriter writeOwn ;

		SET_ROOM_ID::iterator iter = m_vMyOwnRooms.begin() ;
		uint16_t nIdx = 0 ;
		for ( ; iter != m_vMyOwnRooms.end(); ++iter ,++nIdx)
		{
			ownArray[nIdx] = *iter ;
		}
		strMyOwn = writeOwn.write(ownArray) ;
	}
	msgSavePokerData.nFollowedRoomsStrLen = strlen(strFollow.c_str()) ;
	msgSavePokerData.nMyOwnRoomsStrLen = strlen(strMyOwn.c_str()) ;

	CAutoBuffer writeBuffer(sizeof(msgSavePokerData) + msgSavePokerData.nFollowedRoomsStrLen + msgSavePokerData.nMyOwnRoomsStrLen ) ;
	writeBuffer.addContent((char*)&msgSavePokerData,sizeof(msgSavePokerData));
	if ( msgSavePokerData.nFollowedRoomsStrLen > 0 )
	{
		writeBuffer.addContent(strFollow.c_str(),msgSavePokerData.nFollowedRoomsStrLen) ;
	}

	if ( msgSavePokerData.nMyOwnRoomsStrLen > 0 )
	{
		writeBuffer.addContent(strMyOwn.c_str(),msgSavePokerData.nMyOwnRoomsStrLen) ;
	}

	CGameServerApp::SharedGameServerApp()->sendMsg(GetPlayer()->GetSessionID(),writeBuffer.getBufferPtr(),writeBuffer.getContentSize());
	CLogMgr::SharedLogMgr()->PrintLog("time save taxas data for uid = %d",GetPlayer()->GetUserUID());
}
