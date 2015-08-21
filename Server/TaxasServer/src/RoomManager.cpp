#include "RoomManager.h"
#include "TaxasServerApp.h"
#include "LogManager.h"
#include "TaxasRoom.h"
#include "ServerMessageDefine.h"
#include "RoomConfig.h"
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
	if ( prealMsg->usMsgType <= MSG_TP_BEGIN || MSG_TP_END <= prealMsg->usMsgType )
	{
		CLogMgr::SharedLogMgr()->ErrorLog("why this msg send here msg = %d ",prealMsg->usMsgType ) ;
		return false ;
	}

	if ( MSG_TP_CREATE_ROOM == prealMsg->usMsgType )
	{
		stMsgCreateTaxasRoomRet msgBack ;
		stMsgCreateTaxasRoom* pRet = (stMsgCreateTaxasRoom*)prealMsg ;
		stBaseRoomConfig* pRoomConfig = CTaxasServerApp::SharedGameServerApp()->GetConfigMgr()->GetRoomConfig(eRoom_TexasPoker,pRet->nConfigID);
		if ( pRoomConfig == nullptr )
		{
			msgBack.nRet = 1 ;
			msgBack.nRoomID = 0 ;
			SendMsg(&msgBack,sizeof(msgBack),nSessionID) ;
			return true ;
		}
		else
		{
			stMsgPlayerRequestCoin msgReqMoney ;
			msgReqMoney.bIsDiamond = false ;
			msgReqMoney.nAtLeast = 0 ;
			msgReqMoney.nWantMoney = pRoomConfig->nCreateFee ;
			msgReqMoney.nSessionID = nSessionID ;
			msgReqMoney.nReqType = eReqMoney_CreateRoom;
			msgReqMoney.nUserUID = 1 ;
			msgReqMoney.nBackArg[0] = nSessionID ;
			msgReqMoney.nBackArg[1] = pRet->nConfigID ;
			SendMsg(&msgReqMoney,sizeof(msgReqMoney),nSessionID) ;
		}
	}

	if ( MSG_REQUEST_MONEY == prealMsg->usMsgType && eSenderPort == ID_MSG_PORT_DATA )
	{
		stMsgPlayerRequestCoinRet* pRet = (stMsgPlayerRequestCoinRet*)prealMsg ;
		if ( eReqMoney_TaxasTakeIn == pRet->nReqType )
		{
			CTaxasRoom* pRoom = GetRoomByID(pRet->nBackArg[0]) ;
			if ( pRoom == NULL )
			{
				CLogMgr::SharedLogMgr()->ErrorLog("Imporssible error , why not the room id = %d is null can not process msg = %d, session id = %d",pRet->nBackArg[0],MSG_REQUEST_MONEY,nSessionID)  ;
				return true ;
			}
			pRoom->OnMessage(prealMsg,eSenderPort,nSessionID) ;
		}
		else if ( eReqMoney_CreateRoom == pRet->nReqType )
		{
			stMsgCreateTaxasRoomRet msgBack ;
			msgBack.nRoomID = 0 ;
			msgBack.nRet = pRet->nRet;
			if ( pRet->nRet == 0 )
			{
				stBaseRoomConfig* pRoomConfig = CTaxasServerApp::SharedGameServerApp()->GetConfigMgr()->GetRoomConfig(eRoom_TexasPoker,pRet->nBackArg[1]);
				CTaxasRoom* pRoom = new CTaxasRoom ;
				pRoom->Init( m_vRooms.size() + 1,(stTaxasRoomConfig*)pRoomConfig) ;
				m_vRooms[pRoom->GetRoomID()] = pRoom ;
				pRoom->onCreateByPlayer(pRet->nUserUID);
				pRoom->setRoomName("HappyPoker");
				pRoom->setRoomDesc("I want you !");
				msgBack.nRoomID = pRoom->GetRoomID() ;
			}
			SendMsg(&msgBack,sizeof(msgBack),nSessionID) ;
		}

		return true ;
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
		msg.nSessionID = nSessionID ;
		SendMsg(&msg,sizeof(msg),pRoom->GetRoomID()) ;
		CLogMgr::SharedLogMgr()->PrintLog("rquest player data for room ");
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
			CTaxasServerApp::SharedGameServerApp()->sendMsg(nSessionID,(char*)&msgBack,sizeof(msgBack)) ;
			CLogMgr::SharedLogMgr()->SystemLog("invalid session id = %d can not get player data ret = %d ", nSessionID,pRet->nRet ) ;
			return true ;
		}

		CTaxasRoom* pRoom = GetRoomByID(pRet->nRoomID) ;
		if ( pRoom == NULL )
		{
			CLogMgr::SharedLogMgr()->ErrorLog("why room is null server error room id = %d",pRet->nRoomID ) ;

			stMsgTaxasEnterRoomRet msgBack ;
			msgBack.nRet = 3;
			CTaxasServerApp::SharedGameServerApp()->sendMsg(nSessionID,(char*)&msgBack,sizeof(msgBack)) ;
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
		if ( pRoom == NULL || pRoom->IsPlayerInRoomWithSessionID(nSessionID) == false )
		{
			CLogMgr::SharedLogMgr()->ErrorLog("why not the room id = %d is null can not process , or you not in target room msg = %d, session id = %d",pRet->nRoomID,MSG_TP_ORDER_LEAVE,nSessionID)  ;
			
			// still inform data svr , help it reset room id , then player can enter room agian
			stMsgOrderTaxasPlayerLeaveRet msgLeave ;
			msgLeave.nUserUID = pRet->nUserUID;
			msgLeave.nRet = 1 ;
			CTaxasServerApp::SharedGameServerApp()->sendMsg(pRet->nRoomID,(char*)&msgLeave,sizeof(msgLeave)) ;
			return true ;
		}
		pRoom->OnMessage(prealMsg,eSenderPort,nSessionID) ;
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