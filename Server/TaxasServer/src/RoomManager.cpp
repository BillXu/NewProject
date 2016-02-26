#include "RoomManager.h"
#include "TaxasServerApp.h"
#include "LogManager.h"
#include "TaxasRoom.h"
#include "ServerMessageDefine.h"
#include "RoomConfig.h"
#include "AutoBuffer.h"
#define ROOM_LIST_ITEM_CNT_PER_PAGE 5 
CRoomManager::CRoomManager()
{

}

CRoomManager::~CRoomManager()
{

}

bool CRoomManager::Init()
{
	m_nMaxRoomID = 1 ;
	m_pGoTyeAPI.init("https://qplusapi.gotye.com.cn:8443/api/");
	m_pGoTyeAPI.setDelegate(this);
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

	if ( onPublicMsg(prealMsg,eSenderPort,nSessionID) )
	{
		return true ;
	}

	////if ( prealMsg->usMsgType <= MSG_TP_BEGIN || MSG_TP_END <= prealMsg->usMsgType )
	////{
	////	CLogMgr::SharedLogMgr()->ErrorLog("why this msg send here msg = %d ",prealMsg->usMsgType ) ;
	////	return false ;
	////}



	// msg give to room process 
	stMsgToRoom* pRoomMsg = (stMsgToRoom*)prealMsg;
	CTaxasRoom* pRoom = GetRoomByID(pRoomMsg->nRoomID) ;
	if ( pRoom == NULL )
	{
		CLogMgr::SharedLogMgr()->ErrorLog("can not find room to process id = %d ,from = %d, room id = %d",prealMsg->usMsgType,eSenderPort,pRoomMsg->nRoomID ) ;
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
			if ( pRoom == nullptr )
			{
				return false ;
			}
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

	if ( MSG_READ_ROOM_INFO == prealMsg->usMsgType )
	{
		stMsgReadRoomInfoRet* pRet = (stMsgReadRoomInfoRet*)prealMsg ;
		stBaseRoomConfig* pRoomConfig = CTaxasServerApp::SharedGameServerApp()->GetConfigMgr()->GetConfigByConfigID(pRet->nConfigID);
		assert(pRoomConfig&&"why config is null");
		CLogMgr::SharedLogMgr()->PrintLog("read room info room id = %d",pRet->nRoomID);
		CTaxasRoom* pRoom = new CTaxasRoom ;
		pRoom->Init( pRet->nRoomID,(stTaxasRoomConfig*)pRoomConfig) ;
		m_vRooms[pRoom->GetRoomID()] = pRoom ;
		pRoom->setOwnerUID(pRet->nRoomOwnerUID);
		pRoom->setRoomName(pRet->vRoomName);
		pRoom->setRoomDesc("unused");
		pRoom->setDeadTime(pRet->nDeadTime);
		pRoom->setProfit(pRet->nRoomProfit);
		pRoom->setAvataID(pRet->nAvataID);
		pRoom->setCreateTime(pRet->nCreateTime);
		pRoom->setInformSieral(pRet->nInformSerial);
		pRoom->setChatRoomID(pRet->nChatRoomID);
		pRoom->setTotalProfit(pRet->nTotalProfit); 
		if ( pRet->nInformLen )
		{
			CAutoBuffer auBufo (pRet->nInformLen + 1 );
			auBufo.addContent( ((char*)&pRet->nInformLen) + sizeof(pRet->nInformLen),pRet->nInformLen);
			pRoom->setRoomInform(auBufo.getBufferPtr()) ;
		}

		if ( pRet->nRoomID > m_nMaxRoomID )
		{
			m_nMaxRoomID = pRet->nRoomID ;
		}

		addRoomToCreator(pRoom);
		addRoomToConfigRooms(pRoom);
		return true ;
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

bool CRoomManager::onPublicMsg(stMsg* prealMsg , eMsgPort eSenderPort , uint32_t nSessionID)
{
	switch (prealMsg->usMsgType)
	{
	case MSG_REQUEST_MATCH_ROOM_LIST:
		{
			auto iter = m_vCreatorAndRooms.find(MATCH_MGR_UID);
			stRoomCreatorInfo& pC = iter->second ;
			LIST_ROOM& vMatchRooms = pC.vRooms ;
			stMsgRequestMatchRoomListRet msgRet ;
			msgRet.nItemCnt = vMatchRooms.size() ;
			msgRet.nRoomType = eRoom_TexasPoker ;

			CAutoBuffer buffer(sizeof(msgRet) + sizeof(stMsgMatchRoomItem) * msgRet.nItemCnt );
			buffer.addContent(&msgRet,sizeof(msgRet)) ;
			for ( CTaxasRoom* pRoom : vMatchRooms )
			{
				stMsgMatchRoomItem msgItem ;
				msgItem.nBaseBet = pRoom->getLittleBlind() ;
				msgItem.nEndTime = pRoom->getDeadTime();
				msgItem.nChapionUID = pRoom->getChampionUID() ;
				msgItem.nRoomID = pRoom->GetRoomID() ;
				memset(msgItem.pRoomName,0,sizeof(msgItem.pRoomName)) ;
				memcpy(msgItem.pRoomName,pRoom->getRoomName(),strlen(pRoom->getRoomName()));
				buffer.addContent(&msgItem,sizeof(msgItem));
			}
			SendMsg((stMsg*)buffer.getBufferPtr(),buffer.getContentSize(),nSessionID) ;
		}
		break;
	case MSG_TP_ENTER_ROOM:
		{
			stMsgTaxasEnterRoom* pRel = (stMsgTaxasEnterRoom*)prealMsg ;
			CTaxasRoom* pRoom = nullptr;
			if ( pRel->nIDType == 0 )  // room id ;
			{
				if ( pRel->nTargetID == 0 )
				{
					pRoom = GetQuickEnterRoom(0);
				}
				else
				{
					pRoom = GetRoomByID(pRel->nTargetID) ;
				}
			}
			else  // config id ;
			{
				pRoom = GetRoomByConfigID(pRel->nTargetID) ;
			}


			if ( !pRoom )
			{
				CLogMgr::SharedLogMgr()->ErrorLog("can not find id type = %d id = %d",pRel->nIDType,pRel->nTargetID );
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
			msg.nRoomID = pRoom->GetRoomID() ;
			SendMsg(&msg,sizeof(msg),nSessionID ) ;
			CLogMgr::SharedLogMgr()->PrintLog("rquest player data for room ");
			return true ;
		}
	case MSG_TP_REQUEST_ROOM_LIST:
		{
			std::vector<CTaxasRoom*> vActiveRoom ;
			MAP_ID_ROOM::iterator iter = m_vRooms.begin() ;
			for ( ; iter != m_vRooms.end(); ++iter )
			{
				if ( iter->second->isRoomAlive() && iter->second->getOwnerUID() != MATCH_MGR_UID )
				{
					vActiveRoom.push_back(iter->second) ;
				}
			}

			uint16_t nPageCnt = ( vActiveRoom.size() + ROOM_LIST_ITEM_CNT_PER_PAGE -1  ) / ROOM_LIST_ITEM_CNT_PER_PAGE ; 
			for ( uint16_t nPageIdx = 0 ; nPageIdx < nPageCnt ; ++nPageIdx )
			{
				stMsgRequestRoomListRet msgRet ;
				msgRet.nRoomType = eRoom_TexasPoker ;
				msgRet.bFinal = nPageIdx == (nPageCnt - 1 );
				msgRet.nListCnt = msgRet.bFinal ? ( vActiveRoom.size() - ( ROOM_LIST_ITEM_CNT_PER_PAGE * nPageIdx ) ) : ROOM_LIST_ITEM_CNT_PER_PAGE ;
				CAutoBuffer auBuffer(sizeof(msgRet) + sizeof(stRoomListItem) * msgRet.nListCnt );
				auBuffer.addContent((char*)&msgRet,sizeof(msgRet)) ;
				for ( uint8_t nIdx = 0 ; nIdx < ROOM_LIST_ITEM_CNT_PER_PAGE; ++nIdx )
				{
					uint16_t nRoomIdx = nPageIdx*ROOM_LIST_ITEM_CNT_PER_PAGE + nIdx ;
					if ( nRoomIdx >= vActiveRoom.size() )
					{
						break;
					}

					CTaxasRoom* pRoom = vActiveRoom[nRoomIdx];
					stRoomListItem stItem ;
					memset(&stItem,0,sizeof(stItem));
					stItem.nCreatOwnerUID = pRoom->getOwnerUID() ;
					stItem.nCurrentCount = pRoom->GetPlayerCntWithState(eRoomPeer_SitDown);
					stItem.nRoomID = pRoom->GetRoomID();
					stItem.nSmiallBlind = pRoom->getLittleBlind();
					stItem.nSeatCnt = pRoom->getSeatCnt();
					stItem.nDeadTime = pRoom->getDeadTime();
					sprintf_s(stItem.vRoomName,sizeof(stItem.vRoomName),"%s",pRoom->getRoomName());
					//sprintf_s(stItem.vDesc,sizeof(stItem.vDesc),"%s",pRoom->getRoomDesc());
					auBuffer.addContent((char*)&stItem,sizeof(stItem)) ;
				}
				SendMsg((stMsg*)auBuffer.getBufferPtr(),auBuffer.getContentSize(),nSessionID);
			}

			if ( nPageCnt == 0 )
			{
				stMsgRequestRoomListRet msgRet ;
				msgRet.nRoomType = eRoom_TexasPoker ;
				msgRet.bFinal = true ;
				msgRet.nListCnt = 0 ;
				SendMsg(&msgRet,sizeof(msgRet),nSessionID) ;
			}
		}  
		break;
	case MSG_TP_READ_MY_OWN_ROOMS:
		{
			stMsgReadMyOwnTaxasRooms* pRet = (stMsgReadMyOwnTaxasRooms*)prealMsg ;
			LIST_ROOM vRL ;
			if ( getRoomCreatorRooms(pRet->nUserUID,vRL) == false )
			{
				CLogMgr::SharedLogMgr()->PrintLog("uid = %d do not create room so , need not respone list" ,pRet->nUserUID ) ;
				return true ;
			}

			stMsgReadMyOwnTaxasRoomsRet msgRead ;
			msgRead.nRoomType = eRoom_TexasPoker ;
			msgRead.nCnt = vRL.size() ;
			CAutoBuffer auBuffer(msgRead.nCnt * sizeof(stMyOwnRoom) + sizeof(msgRead));
			auBuffer.addContent(&msgRead,sizeof(msgRead)) ;
			stMyOwnRoom info ;
			for ( CTaxasRoom* proom : vRL )
			{
				info.nConfigID = proom->getConfigID() ;
				info.nRoomID = proom->GetRoomID() ;
				auBuffer.addContent(&info,sizeof(info)) ;
			}
			SendMsg((stMsg*)auBuffer.getBufferPtr(),auBuffer.getContentSize(),nSessionID) ;
			CLogMgr::SharedLogMgr()->PrintLog("respone uid = %d have owns cnt = %d",pRet->nUserUID,vRL.size()) ;
		}
		break;
	default:
		return false;
	}
	return true ;
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

CTaxasRoom* CRoomManager::GetRoomByConfigID(uint32_t nRoomConfigID )
{
	MAP_CONFIG_ROOMS::iterator iter = m_vCongfigIDRooms.find(nRoomConfigID) ;
	if ( iter == m_vCongfigIDRooms.end() )
	{
		return nullptr ;
	}

	LIST_ROOM& vRooms = iter->second ;
	if ( vRooms.empty() )
	{
		return nullptr ;
	}

	LIST_ROOM vAcitveRooms ;
	LIST_ROOM vEmptyRooms ;
	for ( CTaxasRoom* pRoom : vRooms )
	{
		if ( pRoom == nullptr || pRoom->isRoomAlive() == false )
		{
			continue; 
		}

		if ( pRoom->GetPlayerCntWithState(eRoomPeer_SitDown) )
		{
			 vAcitveRooms.push_back(pRoom) ;
		}
		else
		{
			vEmptyRooms.push_back(pRoom) ;
		}
	}

	if ( vAcitveRooms.empty() && vEmptyRooms.empty() )
	{
		return nullptr ;
	}

	if ( vAcitveRooms.empty() )  // if all room is empty , then just rand a room to enter ;
	{
		vAcitveRooms.insert(vAcitveRooms.begin(),vEmptyRooms.begin(),vEmptyRooms.end()) ;
	}
	else if ( vAcitveRooms.size() <= 10 )  // put some empty rooms in 
	{
		uint8_t naddEmtpy = 0 ;
		for ( CTaxasRoom* pRoom : vEmptyRooms )
		{
			if ( naddEmtpy > 8 )
			{
				break;
			}

			if ( naddEmtpy % 2 == 0 )
			{
				vAcitveRooms.push_back(pRoom) ;
			}
			else
			{
				vAcitveRooms.insert(vAcitveRooms.begin(),pRoom) ;
			}
			
			++naddEmtpy ;
		}
	}

	if ( vAcitveRooms.empty() )
	{
		return nullptr ;
	}

	uint16_t nStartIdx = rand() % vAcitveRooms.size() ;
	uint16_t iter_idx = 0 ;
	for( CTaxasRoom* pRoom : vAcitveRooms )
	{
		if ( iter_idx != nStartIdx )
		{
			++iter_idx ;
			continue;
		}

		return pRoom ;

	}
	return nullptr ;
}

CTaxasRoom* CRoomManager::GetQuickEnterRoom(uint64_t nCoin )
{
	LIST_ROOM vValidRooms , vLiveRooms;
	MAP_ID_ROOM::iterator iter = m_vRooms.begin() ;
	for ( ; iter != m_vRooms.end() ; ++iter )
	{
		if ( iter->second->isRoomAlive() )
		{
			uint8_t nPCnt = iter->second->GetPlayerCntWithState(eRoomPeer_SitDown);
			if ( nPCnt > 0 && nPCnt != iter->second->getSeatCnt() )
			{
				vValidRooms.push_back(iter->second);
			}
			else
			{
				vLiveRooms.push_back(iter->second) ;
			}
		}
	}

	uint16_t nRandom = 0 ;
	if ( vValidRooms.empty() == false )
	{
		nRandom = rand() % vValidRooms.size() ;
		LIST_ROOM::iterator iter = vValidRooms.begin() ;
		while ( nRandom-- )
		{
			++iter ;
		}

		if ( iter == vValidRooms.end() )
		{
			--iter ;
			CLogMgr::SharedLogMgr()->ErrorLog("will not run to here   iter == vValidRooms.end() ");
		}
		return *iter ;
	}

	if ( vLiveRooms.empty() == false )
	{
		nRandom = rand() % vLiveRooms.size() ;
		LIST_ROOM::iterator iterL = vLiveRooms.begin() ;
		while ( nRandom-- )
		{
			++iterL ;
		}

		if ( iterL == vLiveRooms.end() )
		{
			--iterL ;
			CLogMgr::SharedLogMgr()->ErrorLog("will not run to here   iter == vLiveRooms.end() ");
		}
		return *iterL ;
	}
	return nullptr ;
}

void CRoomManager::SendMsg(stMsg* pmsg, uint32_t nLen , uint32_t nSessionID )
{
	CTaxasServerApp::SharedGameServerApp()->sendMsg(nSessionID,(char*)pmsg,nLen) ;
}

void CRoomManager::onHttpCallBack(char* pResultData, size_t nDatalen , void* pUserData , size_t nUserTypeArg)
{
	if ( nUserTypeArg == eCrossSvrReq_CreateTaxasRoom )
	{
		uint64_t nChatRoomID = 0 ;
		bool bSuccess = nDatalen > 0 ;
		if ( bSuccess )
		{
			Json::Reader reader ;
			Json::Value cValue ;
			reader.parse(pResultData,pResultData + nDatalen,cValue) ;
			bSuccess = cValue["errcode"].asInt() == 200 ;
			nChatRoomID = cValue["room_id"].asUInt();
			CLogMgr::SharedLogMgr()->PrintLog("error code = %d room id = %d",cValue["errcode"].asInt(), cValue["room_id"].asUInt() );
		}

		CTaxasRoom* pRoom = (CTaxasRoom*)pUserData ;
		stMsgCrossServerRequestRet msgRet ;
		msgRet.cSysIdentifer = ID_MSG_PORT_DATA ;
		msgRet.nReqOrigID = pRoom->GetRoomID();
		msgRet.nTargetID = pRoom->getOwnerUID();
		msgRet.nRequestType = eCrossSvrReq_CreateTaxasRoom ;
		msgRet.nRequestSubType = eCrossSvrReqSub_Default;
		msgRet.nRet = 0 ;
		msgRet.vArg[0] = pRoom->getConfigID() ;
		msgRet.vArg[1] = 0 ;
		msgRet.vArg[2] = ( pRoom->getDeadTime() - pRoom->getCreateTime() ) / ( 60 * 60 * 24 );
		if ( bSuccess ) // success
		{
			pRoom->setChatRoomID(nChatRoomID);
			msgRet.vArg[1] = pRoom->GetRoomID() ;

			stMsgSaveCreateRoomInfo msgCreateInfo ;
			msgCreateInfo.nRoomType = eRoom_TexasPoker;
			msgCreateInfo.nCreateTime = pRoom->getCreateTime();
			msgCreateInfo.nConfigID = pRoom->getConfigID() ;
			msgCreateInfo.nRoomID = pRoom->GetRoomID();
			msgCreateInfo.nRoomOwnerUID = pRoom->getOwnerUID() ;
			msgCreateInfo.nChatRoomID = nChatRoomID ;
			SendMsg(&msgCreateInfo,sizeof(msgCreateInfo),pRoom->GetRoomID());
			pRoom->forceDirytInfo();
			pRoom->saveUpdateRoomInfo();
			addRoomToCreator(pRoom);
			addRoomToConfigRooms(pRoom) ;
			CLogMgr::SharedLogMgr()->PrintLog("uid = %d create room success",pRoom->getOwnerUID());
		}
		else
		{
			CLogMgr::SharedLogMgr()->ErrorLog("uid = %d create room failed no more chat room",pRoom->getOwnerUID());
			msgRet.nRet = 2;
			if ( m_vRooms.find(pRoom->GetRoomID())!= m_vRooms.end() )
			{
				m_vRooms.erase(m_vRooms.find(pRoom->GetRoomID())) ;
				delete pRoom ;
				pRoom = nullptr ;
			}
		}
		SendMsg(&msgRet,sizeof(msgRet),msgRet.nTargetID);
	}
	else if ( nUserTypeArg == eCrossSvrReq_DeleteRoom )
	{
		bool bSuccess = nDatalen > 0 ;
		if ( bSuccess )
		{
			Json::Reader reader ;
			Json::Value cValue ;
			reader.parse(pResultData,pResultData + nDatalen,cValue) ;
			bSuccess = cValue["errcode"].asInt() == 200 ;
			CLogMgr::SharedLogMgr()->PrintLog("delete room chat id code = %d",cValue["errcode"].asInt());
		}
	}
}

bool CRoomManager::onCrossServerRequest(stMsgCrossServerRequest* pRequest , eMsgPort eSenderPort,Json::Value* vJsValue)
{
	if ( eCrossSvrReq_RoomProfit == pRequest->nRequestType )
	{
		stMsgCrossServerRequestRet msgRet ;
		msgRet.cSysIdentifer = ID_MSG_PORT_DATA ;
		msgRet.nReqOrigID = pRequest->nTargetID ;
		msgRet.nTargetID = pRequest->nReqOrigID ;
		msgRet.nRequestType = pRequest->nRequestType ;
		msgRet.nRequestSubType = pRequest->nRequestSubType ;
		msgRet.nRet = 0 ;
		msgRet.vArg[0] = true ;
		msgRet.vArg[2] = eRoom_TexasPoker ;

		CTaxasRoom* pRoom = GetRoomByID(pRequest->nTargetID );
		if ( pRoom != nullptr )
		{
			msgRet.vArg[1] = pRoom->getProfit();
			pRoom->setProfit(0) ;
			pRoom->addTotoalProfit(msgRet.vArg[1]) ;
			pRoom->forceDirytInfo() ;
		}
		else
		{
			msgRet.nRet = 1 ;
			msgRet.vArg[1] = 0;
		}
		CTaxasServerApp::SharedGameServerApp()->sendMsg(msgRet.nTargetID,(char*)&msgRet,sizeof(msgRet)) ;
		return true ;
	}
	else if ( eCrossSvrReq_CreateTaxasRoom == pRequest->nRequestType )
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
		msgRet.vArg[0] = pRequest->vArg[0];
		msgRet.vArg[1] = 0 ;
		msgRet.vArg[2] = pRequest->vArg[1] ;
		stBaseRoomConfig* pRoomConfig = CTaxasServerApp::SharedGameServerApp()->GetConfigMgr()->GetConfigByConfigID(nConfigID);
		if ( pRoomConfig == nullptr )
		{
			msgRet.nRet = 1;
			SendMsg(&msgRet,sizeof(msgRet),msgRet.nTargetID);
			return true ;
		}

		CTaxasRoom* pRoom = new CTaxasRoom ;
		pRoom->Init( ++m_nMaxRoomID,(stTaxasRoomConfig*)pRoomConfig) ;
		m_vRooms[pRoom->GetRoomID()] = pRoom ;
		pRoom->onCreateByPlayer(pRequest->nReqOrigID,pRequest->vArg[1]);
		pRoom->setRoomName(strName.c_str());
		pRoom->setRoomDesc("I want you !");
		if ( false == reqeustChatRoomID(pRoom) )
		{
			CLogMgr::SharedLogMgr()->ErrorLog("uid = %d create room failed can not connect to chat svr",pRoom->getOwnerUID());
			m_vRooms.erase(m_vRooms.find(pRoom->GetRoomID())) ;
			delete pRoom ;
			pRoom = nullptr ;
			msgRet.nRet = 3;
			SendMsg(&msgRet,sizeof(msgRet),msgRet.nTargetID);
			return true ;
		}
		return true ;
	}
	else if ( eCrossSvrReq_DeleteRoom == pRequest->nRequestType )
	{
		CTaxasRoom* pRoom = GetRoomByID(pRequest->vArg[1] );
		if ( pRoom == nullptr )
		{
			CLogMgr::SharedLogMgr()->ErrorLog("uid = %d delete room = %d , not exsit",pRequest->nReqOrigID,pRequest->vArg[1]) ;
			return true ;
		}
		removeRoom(pRoom) ;
		return true ;
	}
	return false ;
}

bool CRoomManager::onCrossServerRequestRet(stMsgCrossServerRequestRet* pResult,Json::Value* vJsValue)
{
	return false ;
}

bool CRoomManager::reqeustChatRoomID(CTaxasRoom* pRoom)
{
	Json::Value cValue ;
	cValue["email"] = "378569952@qq.com" ;
	cValue["devpwd"] = "bill007" ;
	cValue["appkey"] = "abffee4b-deea-4e96-ac8d-b9d58f246c3f" ;
	cValue["room_name"] = pRoom->GetRoomID() ;
	cValue["room_type"] = 1;
	cValue["room_create_type"] = 0 ;
	Json::StyledWriter sWrite ;
	std::string str = sWrite.write(cValue);
	return m_pGoTyeAPI.performRequest("CreateRoom",str.c_str(),str.size(),pRoom,eCrossSvrReq_CreateTaxasRoom);
}

void CRoomManager::onConnectedToSvr()
{
	if ( m_vRooms.empty() )
	{
		stMsgReadRoomInfo msg ;
		msg.nRoomType = eRoom_TexasPoker ;
		SendMsg(&msg,sizeof(msg),0) ;
		CLogMgr::SharedLogMgr()->PrintLog("request taxas rooms");
	}
}

void CRoomManager::onPlayerChangeRoom(uint32_t nCurRoomID , uint32_t nPlayerSessionID )
{
	int nSize = m_vRooms.size();
	int nR = rand()%nSize ;
	MAP_ID_ROOM::iterator iter = m_vRooms.begin();
	for ( int nidx = 0 ; iter != m_vRooms.end(); ++iter,++nidx )
	{
		if ( nidx == nR )
		{
			break;
		}
	}

	stMsgRequestTaxasPlayerData msg ;
	msg.nRoomID = iter->first ;
	SendMsg(&msg,sizeof(msg),nPlayerSessionID ) ;
	CLogMgr::SharedLogMgr()->PrintLog("change room rquest player data for room ");
}

void CRoomManager::addRoomToCreator(CTaxasRoom* pRoom)
{
	MAP_UID_CR::iterator iter =  m_vCreatorAndRooms.find(pRoom->getOwnerUID());
	if ( iter != m_vCreatorAndRooms.end() )
	{
		iter->second.vRooms.push_back(pRoom) ;
		return ;
	}

	stRoomCreatorInfo sInfo ;
	sInfo.nPlayerUID = pRoom->getOwnerUID() ;
	sInfo.vRooms.push_back(pRoom) ;
	m_vCreatorAndRooms[sInfo.nPlayerUID] = sInfo ;
}

void CRoomManager::addRoomToConfigRooms(CTaxasRoom* pRoom)
{
	MAP_CONFIG_ROOMS::iterator iter = m_vCongfigIDRooms.find( pRoom->getConfigID() ) ;
	if ( iter == m_vCongfigIDRooms.end() )
	{
		LIST_ROOM vRooms ;
		vRooms.push_back(pRoom);
		m_vCongfigIDRooms[pRoom->getConfigID()] = vRooms ;
		return ;
	}

	LIST_ROOM& vRoom = iter->second ;
	vRoom.push_back(pRoom) ;
	return ;
}

bool CRoomManager::getRoomCreatorRooms(uint32_t nCreatorUID, LIST_ROOM& vInfo )
{
	MAP_UID_CR::iterator iter = m_vCreatorAndRooms.find(nCreatorUID) ;
	if ( iter == m_vCreatorAndRooms.end() )
	{
		return false ;
	}
	vInfo = iter->second.vRooms ;
	return true ;
}

void CRoomManager::removeRoom(CTaxasRoom* pRoom )
{
	pRoom->deleteRoom();
	// remove from db ;
	stMsgSaveDeleteRoom msgSaveDelte ;
	msgSaveDelte.nRoomID = pRoom->GetRoomID() ;
	msgSaveDelte.nRoomType = eRoom_TexasPoker ;
	SendMsg(&msgSaveDelte,sizeof(msgSaveDelte),0 ) ;
}

void CRoomManager::doDeleteRoom(CTaxasRoom* pRoom )
{
	// remove m_vRooms ;
	auto iter = m_vRooms.find(pRoom->GetRoomID()) ;
	if ( iter != m_vRooms.end() )
	{
		m_vRooms.erase(iter) ;
	}

	// m_vCongfigIDRooms 
	auto iter_config = m_vCongfigIDRooms.find(pRoom->getConfigID());
	if ( iter_config != m_vCongfigIDRooms.end() )
	{
		LIST_ROOM& list = iter_config->second ;
		LIST_ROOM::iterator iterC = list.begin() ;
		for ( ; iterC != list.end(); ++iterC )
		{
			if ( (*iterC)->GetRoomID() == pRoom->GetRoomID() )
			{
				list.erase(iterC) ;
				break;
			}
		}
	}

	// remove m_vCreatorAndRooms ;
	auto iter_Create = m_vCreatorAndRooms.find(pRoom->getOwnerUID()) ;
	if ( iter_Create != m_vCreatorAndRooms.end() )
	{
		LIST_ROOM& list = iter_Create->second.vRooms ;
		LIST_ROOM::iterator iterC = list.begin() ;
		for ( ; iterC != list.end(); ++iterC )
		{
			if ( (*iterC)->GetRoomID() == pRoom->GetRoomID() )
			{
				list.erase(iterC) ;
				break;
			}
		}
	}

	// remove from db ;
	stMsgSaveDeleteRoom msgSaveDelte ;
	msgSaveDelte.nRoomID = pRoom->GetRoomID() ;
	msgSaveDelte.nRoomType = eRoom_TexasPoker ;
	SendMsg(&msgSaveDelte,sizeof(msgSaveDelte),0 ) ;

	// delete chat room id ;
	Json::Value cValue ;
	cValue["email"] = "378569952@qq.com" ;
	cValue["devpwd"] = "bill007" ;
	cValue["appkey"] = "abffee4b-deea-4e96-ac8d-b9d58f246c3f" ;
	cValue["room_id"] = pRoom->getChatRoomID() ;
	Json::StyledWriter sWrite ;
	std::string str = sWrite.write(cValue);
	m_pGoTyeAPI.performRequest("DeleteRoom",str.c_str(),str.size(),nullptr,eCrossSvrReq_DeleteRoom);

	// delete room ;
	CLogMgr::SharedLogMgr()->SystemLog("do delete room id = %d",pRoom->GetRoomID());
	delete pRoom ;
	pRoom = nullptr ;
	return ;
}

void CRoomManager::onUpdate(float fDelta )
{
	LIST_ROOM vDoDelteRoom ;
	auto iter = m_vRooms.begin();
	for ( ; iter != m_vRooms.end() ; ++iter )
	{
		if ( ! iter->second->isDeleteRoom() )
		{
			continue;
		}

		if ( iter->second->getCurRoomState() == eRoomState_Dead )
		{
			vDoDelteRoom.push_back(iter->second) ;
		}
	}

	for ( CTaxasRoom* pRoom : vDoDelteRoom )
	{
		doDeleteRoom(pRoom) ;
	}
}