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
}

void CPlayerTaxas::Init()
{
	IPlayerComponent::Init();
	m_eType = ePlayerComponent_PlayerTaxas ;
	m_nCurTaxasRoomID = 0 ;
	m_bDirty = false ;
	memset(&m_tData,0,sizeof(m_tData));
}

bool CPlayerTaxas::OnMessage( stMsg* pMessage , eMsgPort eSenderPort)
{
	if ( IPlayerComponent::OnMessage(pMessage,eSenderPort) )
	{
		return true ;
	}

	switch (pMessage->usMsgType)
	{
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
			CLogMgr::SharedLogMgr()->ErrorLog("create room pls kou qian, bu neng chongfu , wan cheng yi ge , then next ");
			
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
			vArg["roonName"] = GetPlayer()->GetBaseData()->GetPlayerName();
			CON_REQ_MSG_JSON(msgReq,vArg,autoBuf) ;
			CGameServerApp::SharedGameServerApp()->sendMsg(msgReq.nReqOrigID,autoBuf.getBufferPtr(),autoBuf.getContentSize()) ;
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
		msgBack.nRoomID = pResult->vArg[0];
		SendMsg(&msgBack,sizeof(msgBack)) ;
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
	msgSavePokerData.nPlayTimes = m_tData.nPlayTimes ;
	msgSavePokerData.nSingleWinMost = m_tData.nSingleWinMost ;
	msgSavePokerData.nWinTimes = m_tData.nWinTimes ;
	memcpy(msgSavePokerData.vMaxCards,m_tData.vMaxCards,sizeof(m_tData.vMaxCards));
	msgSavePokerData.nUserUID = GetPlayer()->GetUserUID() ;
	SendMsg((stMsgSavePlayerMoney*)&msgSavePokerData,sizeof(msgSavePokerData)) ;
}
