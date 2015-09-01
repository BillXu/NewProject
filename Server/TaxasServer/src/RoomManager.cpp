#include "RoomManager.h"
#include "TaxasServerApp.h"
#include "LogManager.h"
#include "TaxasRoom.h"
#include "ServerMessageDefine.h"
#include "RoomConfig.h"
#include "AutoBuffer.h"
CRoomManager::CRoomManager()
{

}

CRoomManager::~CRoomManager()
{

}

bool CRoomManager::Init()
{
	stBaseRoomConfig* pconfig = CTaxasServerApp::SharedGameServerApp()->GetConfigMgr()->GetRoomConfig(eRoom_TexasPoker,0);
	if ( pconfig )
	{
		CTaxasRoom* pRoom = new CTaxasRoom ;
		pRoom->Init(m_vRooms.size()+1,(stTaxasRoomConfig*)pconfig) ;
		m_vRooms[pRoom->GetRoomID()] = pRoom ;
		pRoom->setRoomName("System");
		pRoom->setRoomDesc("I want you !");
	}
	return true ;
}

bool CRoomManager::OnMsg( stMsg* prealMsg , eMsgPort eSenderPort , uint32_t nSessionID )
{
	if ( eSenderPort != ID_MSG_PORT_CLIENT )
	{
		if ( OnMsgFromOtherSvr(prealMsg,eSenderPort,nSessionID) )
		{
			return true ;
		}
	}

	if ( prealMsg->usMsgType <= MSG_TP_BEGIN || MSG_TP_END <= prealMsg->usMsgType )
	{
		CLogMgr::SharedLogMgr()->ErrorLog("why this msg send here msg = %d ",prealMsg->usMsgType ) ;
		return false ;
	}

	if ( MSG_TP_ENTER_ROOM == prealMsg->usMsgType )
	{
		stMsgTaxasEnterRoom* pRel = (stMsgTaxasEnterRoom*)prealMsg ;
		CTaxasRoom* pRoom = GetRoomByID(pRel->nRoomID) ;
		if ( !pRoom )
		{
			CLogMgr::SharedLogMgr()->ErrorLog("can not find room id = %d",pRel->nRoomID );
			stMsgTaxasEnterRoomRet msgBack ;
			msgBack.nRet = 1 ;
			CTaxasServerApp::SharedGameServerApp()->sendMsg(nSessionID,(char*)&msgBack,sizeof(msgBack)) ;
			return true ;
		}

		if ( pRoom->IsPlayerInRoomWithSessionID(nSessionID) )
		{
			stMsgTaxasEnterRoomRet msgBack ;
			msgBack.nRet = 2 ;
			CTaxasServerApp::SharedGameServerApp()->sendMsg(nSessionID,(char*)&msgBack,sizeof(msgBack)) ;
			return true ;
		}
		
		stMsgRequestTaxasPlayerData msg ;
		msg.nRoomID = pRel->nRoomID ;
		SendMsg(&msg,sizeof(msg),nSessionID ) ;
		CLogMgr::SharedLogMgr()->PrintLog("rquest player data for room ");
		return true ;
	}

	// msg give to room process 
	stMsgToRoom* pRoomMsg = (stMsgToRoom*)prealMsg;
	CTaxasRoom* pRoom = GetRoomByID(pRoomMsg->nRoomID) ;
	if ( pRoom == NULL )
	{
		CLogMgr::SharedLogMgr()->ErrorLog("can not find room to process id = %d ,from = %d",prealMsg->usMsgType,eSenderPort ) ;
		return  false ;
	}

	return pRoom->OnMessage(prealMsg,eSenderPort,nSessionID) ;
}

bool CRoomManager::OnMsgFromOtherSvr( stMsg* prealMsg , eMsgPort eSenderPort , uint32_t nRoomID )
{
	if ( MSG_CROSS_SERVER_REQUEST == prealMsg->usMsgType )
	{
		stMsgCrossServerRequest* pRet = (stMsgCrossServerRequest*)prealMsg ;

		Json::Value* pJsValue = nullptr ;
		Json::Value rootValue ;
		if ( pRet->nJsonsLen )
		{
			Json::Reader reader;
			char* pstr = ((char*)&pRet->nJsonsLen) + sizeof(pRet->nJsonsLen) ;
			reader.parse(pstr,pstr + pRet->nJsonsLen,rootValue,false);
			pJsValue = &rootValue ;
		}

		if ( onCrossServerRequest(pRet,eSenderPort,pJsValue) == false )
		{
			CTaxasRoom* pRoom = GetRoomByID(pRet->nTargetID);
			assert(pRoom&&"this request no one to process or target id error");
			return pRoom->onCrossServerRequest(pRet,eSenderPort,pJsValue);
		}

		return true ;
	}

	if ( MSG_CROSS_SERVER_REQUEST_RET == prealMsg->usMsgType )
	{
		stMsgCrossServerRequestRet* pRet = (stMsgCrossServerRequestRet*)prealMsg ;
		Json::Value* pJsValue = nullptr ;
		Json::Value rootValue ;
		if ( pRet->nJsonsLen )
		{
			Json::Reader reader;
			char* pstr = ((char*)&pRet->nJsonsLen) + sizeof(pRet->nJsonsLen) ;
			reader.parse(pstr,pstr + pRet->nJsonsLen,rootValue,false);
			pJsValue = &rootValue ;
		}

		if ( onCrossServerRequestRet(pRet,pJsValue) == false )
		{
			CTaxasRoom* pRoom = GetRoomByID(pRet->nTargetID);
			assert(pRoom&&"this request no one to process or target id error");
			return pRoom->onCrossServerRequestRet(pRet,pJsValue);
		}
		return true ;
	}

	if ( MSG_TP_REQUEST_PLAYER_DATA == prealMsg->usMsgType )
	{
		if ( eSenderPort != ID_MSG_PORT_DATA )
		{
			CLogMgr::SharedLogMgr()->ErrorLog("why this msg not from data server ? ") ;
			return true ;
		}

		stMsgRequestTaxasPlayerDataRet* pRet = (stMsgRequestTaxasPlayerDataRet*)prealMsg ;
		if ( pRet->nRet )
		{
			stMsgTaxasEnterRoomRet msgBack ;
			msgBack.nRet = pRet->nRet ;
			CTaxasServerApp::SharedGameServerApp()->sendMsg(pRet->tData.nSessionID,(char*)&msgBack,sizeof(msgBack)) ;
			CLogMgr::SharedLogMgr()->SystemLog("invalid session id = %d can not get player data ret = %d ", pRet->tData.nSessionID,pRet->nRet ) ;
			return true ;
		}

		CTaxasRoom* pRoom = GetRoomByID(nRoomID) ;
		if ( pRoom == NULL )
		{
			CLogMgr::SharedLogMgr()->ErrorLog("why room is null server error room id = %d",nRoomID ) ;

			stMsgTaxasEnterRoomRet msgBack ;
			msgBack.nRet = 3;
			CTaxasServerApp::SharedGameServerApp()->sendMsg(pRet->tData.nSessionID,(char*)&msgBack,sizeof(msgBack)) ;
			return true ;
		}

		stTaxasInRoomPeerDataExten* pInRoomPlayer = pRoom->GetInRoomPlayerDataByUID(pRet->tData.nUserUID) ;
		if ( pInRoomPlayer )
		{
			memcpy(pInRoomPlayer,&pRet->tData,sizeof(pRet->tData));
			CLogMgr::SharedLogMgr()->PrintLog("uid = %d data in this room so need not do addPlayer Action",pRet->tData.nUserUID);
		}
		else
		{
			stTaxasInRoomPeerDataExten tDataExten ;
			memset(&tDataExten,0,sizeof(tDataExten));
			memcpy(&tDataExten,&pRet->tData,sizeof(pRet->tData));
			//tDataExten.nCoinInRoom = 0 ;
			//tDataExten.nStateFlag = eRoomPeer_StandUp ;
			pRoom->AddPlayer(tDataExten);
			CLogMgr::SharedLogMgr()->PrintLog("recive player data , do joined to room");
		}
		pRoom->SendRoomInfoToPlayer(pRet->tData.nSessionID);
		return true;
	}

	if ( MSG_TP_ORDER_LEAVE == prealMsg->usMsgType && eSenderPort == ID_MSG_PORT_DATA )
	{
		stMsgOrderTaxasPlayerLeave* pRet = (stMsgOrderTaxasPlayerLeave*)prealMsg ;
		CTaxasRoom* pRoom = GetRoomByID(pRet->nRoomID) ;
		if ( pRoom == NULL || pRoom->IsPlayerInRoomWithSessionID(pRet->nSessionID) == false )
		{
			CLogMgr::SharedLogMgr()->ErrorLog("why not the room id = %d is null can not process , or you not in target room msg = %d, session id = %d",pRet->nRoomID,MSG_TP_ORDER_LEAVE,pRet->nSessionID)  ;

			// still inform data svr , help it reset room id , then player can enter room agian
			stMsgOrderTaxasPlayerLeaveRet msgLeave ;
			msgLeave.nRet = 1 ;
			msgLeave.nUserUID = pRet->nUserUID;
			CTaxasServerApp::SharedGameServerApp()->sendMsg(pRet->nSessionID,(char*)&msgLeave,sizeof(msgLeave)) ;
			return true ;
		}
		pRoom->OnMessage(prealMsg,eSenderPort,pRet->nSessionID) ;
		CLogMgr::SharedLogMgr()->PrintLog("order player leave uid %d",pRet->nSessionID);
		return true ;
	}

	if ( MSG_READ_TAXAS_ROOM_INFO == prealMsg->usMsgType )
	{
		stMsgReadTaxasRoomInfoRet* pRet = (stMsgReadTaxasRoomInfoRet*)prealMsg ;
		stBaseRoomConfig* pRoomConfig = CTaxasServerApp::SharedGameServerApp()->GetConfigMgr()->GetRoomConfig(eRoom_TexasPoker,pRet->nConfigID);
		assert(pRoomConfig&&"why config is null");
		CTaxasRoom* pRoom = new CTaxasRoom ;
		pRoom->Init( pRet->nRoomID,(stTaxasRoomConfig*)pRoomConfig) ;
		m_vRooms[pRoom->GetRoomID()] = pRoom ;
		pRoom->setOwnerUID(pRet->nRoomOwnerUID);
		pRoom->setRoomName(pRet->vRoomName);
		pRoom->setRoomDesc(pRet->vRoomDesc);
		pRoom->setDeadTime(pRet->nDeadTime);
		pRoom->setProfit(pRet->nRoomProfit);
		pRoom->setAvataID(pRet->nAvataID);
		pRoom->setCreateTime(pRet->nCreateTime);
		pRoom->setInformSieral(pRet->nInformSerial);
		if ( pRet->nInformLen )
		{
			CAutoBuffer auBufo (pRet->nInformLen + 1 );
			auBufo.addContent( ((char*)&pRet->nInformLen) + sizeof(pRet->nInformLen),pRet->nInformLen);
			pRoom->setRoomInform(auBufo.getBufferPtr()) ;
		}
	}

	if ( MSG_READ_TAXAS_ROOM_PLAYERS == prealMsg->usMsgType )
	{
		stMsgToRoom* pRoomMsg = (stMsgToRoom*)prealMsg;
		CTaxasRoom* pRoom = GetRoomByID(pRoomMsg->nRoomID) ;
		if ( pRoom == NULL )
		{
			CLogMgr::SharedLogMgr()->ErrorLog("can not find room to process id = %d ,from = %d",prealMsg->usMsgType,eSenderPort ) ;
			return  false ;
		}

		return pRoom->OnMessage(prealMsg,eSenderPort,0) ;
	}

	return false ;
}

CTaxasRoom*CRoomManager::GetRoomByID(uint32_t nRoomID )
{
	MAP_ID_ROOM::iterator iter = m_vRooms.find(nRoomID) ;
	if ( iter != m_vRooms.end() )
	{
		return iter->second ;
	}
	return NULL ;
}

void CRoomManager::SendMsg(stMsg* pmsg, uint32_t nLen , uint32_t nSessionID )
{
	CTaxasServerApp::SharedGameServerApp()->sendMsg(nSessionID,(char*)pmsg,nLen) ;
}

bool CRoomManager::onCrossServerRequest(stMsgCrossServerRequest* pRequest , eMsgPort eSenderPort,Json::Value* vJsValue)
{
	if ( eCrossSvrReq_CreateTaxasRoom == pRequest->nRequestType )
	{
		uint16_t nConfigID = pRequest->vArg[0];
		assert(vJsValue&&"must not be null");
		std::string strName = (*vJsValue)["roonName"].asCString();

		stMsgCrossServerRequestRet msgRet ;
		msgRet.cSysIdentifer = eSenderPort ;
		msgRet.nReqOrigID = pRequest->nTargetID ;
		msgRet.nTargetID = pRequest->nReqOrigID ;
		msgRet.nRequestType = pRequest->nRequestType ;
		msgRet.nRequestSubType = pRequest->nRequestSubType ;
		msgRet.nRet = 0 ;
		msgRet.vArg[0] = 0 ;

		stBaseRoomConfig* pRoomConfig = CTaxasServerApp::SharedGameServerApp()->GetConfigMgr()->GetRoomConfig(eRoom_TexasPoker,nConfigID);
		if ( pRoomConfig == nullptr )
		{
			msgRet.nRet = 1;
			SendMsg(&msgRet,sizeof(msgRet),msgRet.nTargetID);
			return true ;
		}

		CTaxasRoom* pRoom = new CTaxasRoom ;
		pRoom->Init( m_vRooms.size() + 1,(stTaxasRoomConfig*)pRoomConfig) ;
		m_vRooms[pRoom->GetRoomID()] = pRoom ;
		pRoom->onCreateByPlayer(pRequest->nReqOrigID);
		pRoom->setRoomName(strName.c_str());
		pRoom->setRoomDesc("I want you !");
		msgRet.vArg[0] = pRoom->GetRoomID() ;
		SendMsg(&msgRet,sizeof(msgRet),msgRet.nTargetID);

		stMsgSaveCreateTaxasRoomInfo msgCreateInfo ;
		msgCreateInfo.nCreateTime = time(nullptr);
		msgCreateInfo.nConfigID = nConfigID ;
		msgCreateInfo.nRoomID = pRoom->GetRoomID();
		msgCreateInfo.nRoomOwnerUID = pRequest->nReqOrigID ;
		SendMsg(&msgCreateInfo,sizeof(msgCreateInfo),pRoom->GetRoomID());

		pRoom->forceDirytInfo();
		pRoom->saveUpdateRoomInfo();
		return true ;
	}
	return false ;
}

bool CRoomManager::onCrossServerRequestRet(stMsgCrossServerRequestRet* pResult,Json::Value* vJsValue)
{
	return false ;
}

void CRoomManager::onConnectedToSvr()
{
	if ( m_vRooms.empty() )
	{
		stMsgReadTaxasRoomInfo msg ;
		SendMsg(&msg,sizeof(msg),0) ;
		CLogMgr::SharedLogMgr()->ErrorLog("request taxas rooms");
	}
}