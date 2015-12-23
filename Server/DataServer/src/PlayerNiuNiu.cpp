#include "PlayerNiuNiu.h"
#include "NiuNiuMessageDefine.h"
#include "Player.h"
#include "PlayerBaseData.h"
#include "LogManager.h"
void CPlayerNiuNiu::Reset()
{
	IPlayerComponent::Reset();
	m_nCurRoomID = 0 ;
}

void CPlayerNiuNiu::Init()
{
	IPlayerComponent::Init();
	m_nCurRoomID = 0 ;
}

bool CPlayerNiuNiu::OnMessage( stMsg* pMessage , eMsgPort eSenderPort)
{
	if ( IPlayerComponent::OnMessage(pMessage,eSenderPort) )
	{
		return true ;
	}
	switch ( pMessage->usMsgType )
	{
	case MSG_NN_ENTER_ROOM:
		{
			stMsgNNEnterRoom* pEnter = (stMsgNNEnterRoom*)pMessage ;
			stMsgNNEnterRoomRet msgBack ;
			if ( 0 && GetPlayer()->isNotInAnyRoom() == false && m_nCurRoomID != 0 )
			{
				msgBack.nRet = 2 ;
				CLogMgr::SharedLogMgr()->ErrorLog("check the cur room id ");
				SendMsg(&msgBack,sizeof(msgBack)) ;
				return true ;
			}

			stMsgCrossServerRequest msgEnter ;
			msgEnter.cSysIdentifer = ID_MSG_PORT_NIU_NIU ;
			msgEnter.nJsonsLen = 0 ;
			msgEnter.nReqOrigID = GetPlayer()->GetUserUID();
			msgEnter.nRequestSubType = eCrossSvrReqSub_Default ;
			msgEnter.nRequestType = eCrossSvrReq_EnterRoom ;
			msgEnter.nTargetID = pEnter->nRoomID ;
			msgEnter.vArg[0] = GetPlayer()->GetSessionID() ;
			msgEnter.vArg[1] = pEnter->nRoomID ;
			msgEnter.vArg[2] = GetPlayer()->GetBaseData()->GetAllCoin();
			SendMsg(&msgEnter,sizeof(msgEnter)) ;

			m_nCurRoomID = pEnter->nRoomID ;
			CLogMgr::SharedLogMgr()->PrintLog("uid = %d try to enter niuniu room id = %d",GetPlayer()->GetUserUID(),m_nCurRoomID) ;
		}
		break;
	default:
		return false;
	}
	return true ;
}

bool CPlayerNiuNiu::onCrossServerRequest(stMsgCrossServerRequest* pRequest , eMsgPort eSenderPort,Json::Value* vJsValue)
{
	if ( IPlayerComponent::onCrossServerRequest(pRequest,eSenderPort,vJsValue) )
	{
		return true ;
	}

	switch ( pRequest->nRequestType )
	{
	case eCrossSvrReq_LeaveRoomRet:
		{
			if ( pRequest->vArg[0] == eRoom_NiuNiu )
			{
				m_nCurRoomID = 0 ;
				return true ;
			}
			else
			{
				return false;
			}
		}
		break;
	default:
		return false ;
	}
	return true ;
}

bool CPlayerNiuNiu::onCrossServerRequestRet(stMsgCrossServerRequestRet* pResult,Json::Value* vJsValue )
{
	if ( IPlayerComponent::onCrossServerRequestRet(pResult,vJsValue) )
	{
		return true ;
	}

	switch ( pResult->nRequestType )
	{
	case eCrossSvrReq_EnterRoom:
		{
			if ( pResult->vArg[1] == eRoom_NiuNiu )
			{
				if ( pResult->nRet )
				{
					stMsgNNEnterRoomRet msgBack ;
					msgBack.nRet = 2 ;
					SendMsg(&msgBack,sizeof(msgBack)) ;
					CLogMgr::SharedLogMgr()->PrintLog("enter niunniu room failed , cann't find room id = %d , uid = %d",m_nCurRoomID,GetPlayer()->GetUserUID()) ;
					m_nCurRoomID = 0 ;
					return true ;
				}
				else
				{
					CLogMgr::SharedLogMgr()->PrintLog("enter niuniu room success uid = %d",GetPlayer()->GetUserUID()) ;
				}
			}
			else
			{
				return false ; 
			}
		}
		break;
	default:
		return false;
	}
	return true ;
}

void CPlayerNiuNiu::OnPlayerDisconnect()
{
	if ( m_nCurRoomID )
	{
		stMsgCrossServerRequest msgEnter ;
		msgEnter.cSysIdentifer = ID_MSG_PORT_NIU_NIU ;
		msgEnter.nJsonsLen = 0 ;
		msgEnter.nReqOrigID = GetPlayer()->GetUserUID();
		msgEnter.nRequestSubType = eCrossSvrReqSub_Default ;
		msgEnter.nRequestType = eCrossSvrReq_ApplyLeaveRoom ;
		msgEnter.nTargetID = m_nCurRoomID ;
		msgEnter.vArg[0] = m_nCurRoomID ;
		msgEnter.vArg[1] = GetPlayer()->GetSessionID() ;
		SendMsg(&msgEnter,sizeof(msgEnter)) ;
	}
}

void CPlayerNiuNiu::OnOtherWillLogined()
{
	if ( m_nCurRoomID )
	{
		stMsgCrossServerRequest msgEnter ;
		msgEnter.cSysIdentifer = ID_MSG_PORT_NIU_NIU ;
		msgEnter.nJsonsLen = 0 ;
		msgEnter.nReqOrigID = GetPlayer()->GetUserUID();
		msgEnter.nRequestSubType = eCrossSvrReqSub_Default ;
		msgEnter.nRequestType = eCrossSvrReq_ApplyLeaveRoom ;
		msgEnter.nTargetID = m_nCurRoomID ;
		msgEnter.vArg[0] = m_nCurRoomID ;
		msgEnter.vArg[1] = GetPlayer()->GetSessionID() ;
		SendMsg(&msgEnter,sizeof(msgEnter)) ;
	}
}

void CPlayerNiuNiu::TimerSave()
{

}

void CPlayerNiuNiu::OnReactive(uint32_t nSessionID )
{

}

void CPlayerNiuNiu::OnOtherDoLogined()
{

}