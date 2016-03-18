#include "IRoomManager.h"
#include "LogManager.h"
#include "IRoom.h"
#include "ServerMessageDefine.h"
#include "RoomConfig.h"
#include "AutoBuffer.h"
#include <cassert>
#include "IRoomState.h"
#include "RoomConfig.h"
#define ROOM_LIST_ITEM_CNT_PER_PAGE 5 
#define TIME_SAVE_ROOM_INFO 60*10
IRoomManager::IRoomManager()
{

}

IRoomManager::~IRoomManager()
{

}

bool IRoomManager::init(CRoomConfigMgr* pConfigMgr)
{
	m_fTimeSaveTicket = TIME_SAVE_ROOM_INFO * 2;
	m_pConfigMgr = pConfigMgr ;
	m_nMaxRoomID = 1 ;
	m_vCongfigIDRooms.clear();
	m_vRooms.clear();
	m_vCreatorAndRooms.clear();
	m_pGoTyeAPI.init("https://qplusapi.gotye.com.cn:8443/api/");
	m_pGoTyeAPI.setDelegate(this);
	return true ;
}

bool IRoomManager::onMsg( stMsg* prealMsg , eMsgPort eSenderPort , uint32_t nSessionID )
{
	if ( onPublicMsg(prealMsg,eSenderPort,nSessionID) )
	{
		return true ;
	}

	if ( MSG_CROSS_SERVER_REQUEST == prealMsg->usMsgType )
	{
		stMsgCrossServerRequest* pRet = (stMsgCrossServerRequest*)prealMsg ;
		//CLogMgr::SharedLogMgr()->SystemLog("request crose type = %d",pRet->nRequestType);
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
			IRoom* pRoom = GetRoomByID(pRet->nTargetID);
			if ( pRoom )
			{
				return pRoom->onCrossServerRequest(pRet,eSenderPort,pJsValue);
			}
			else
			{
				CLogMgr::SharedLogMgr()->ErrorLog("unprocess cross svr request type = %d",pRet->nRequestType) ;
				return false ;
			}
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
			IRoom* pRoom = GetRoomByID(pRet->nTargetID);
			if ( pRoom )
			{
				return pRoom->onCrossServerRequestRet(pRet,pJsValue);
			}
			return false ;
		}
		return true ;
	}

	////if ( prealMsg->usMsgType <= MSG_TP_BEGIN || MSG_TP_END <= prealMsg->usMsgType )
	////{
	////	CLogMgr::SharedLogMgr()->ErrorLog("why this msg send here msg = %d ",prealMsg->usMsgType ) ;
	////	return false ;
	////}



	// msg give to room process 
	stMsgToRoom* pRoomMsg = (stMsgToRoom*)prealMsg;
	IRoom* pRoom = GetRoomByID(pRoomMsg->nRoomID) ;
	if ( pRoom == NULL )
	{
		CLogMgr::SharedLogMgr()->ErrorLog("can not find room to process id = %d ,from = %d, room id = %d",prealMsg->usMsgType,eSenderPort,pRoomMsg->nRoomID ) ;
		return  false ;
	}

	return pRoom->onMessage(prealMsg,eSenderPort,nSessionID) ;
}

bool IRoomManager::onPublicMsg(stMsg* prealMsg , eMsgPort eSenderPort , uint32_t nSessionID)
{
	switch ( prealMsg->usMsgType )
	{
	case MSG_READ_MY_OWN_ROOMS:
		{
			stMsgReadMyOwnRooms* pRet = (stMsgReadMyOwnRooms*)prealMsg ;
			LIST_ROOM vRL ;
			if ( getRoomCreatorRooms(pRet->nUserUID,vRL) == false )
			{
				CLogMgr::SharedLogMgr()->PrintLog("uid = %d do not create room so , need not respone list" ,pRet->nUserUID ) ;
				return true ;
			}

			stMsgReadMyOwnRoomsRet msgRead ;
			msgRead.nCnt = vRL.size() ;
			msgRead.nRoomType = getMgrRoomType();
			CAutoBuffer auBuffer(msgRead.nCnt * sizeof(stMyOwnRoom) + sizeof(msgRead));
			auBuffer.addContent(&msgRead,sizeof(msgRead)) ;
			stMyOwnRoom info ;
			for ( IRoom* proom : vRL )
			{
				info.nConfigID = proom->getConfigID() ;
				info.nRoomID = proom->getRoomID() ;
				auBuffer.addContent(&info,sizeof(info)) ;
			}
			sendMsg((stMsg*)auBuffer.getBufferPtr(),auBuffer.getContentSize(),nSessionID) ;
			CLogMgr::SharedLogMgr()->PrintLog("respone uid = %d have owns cnt = %d",pRet->nUserUID,vRL.size()) ;
		}
		break;
	case MSG_READ_ROOM_INFO:
		{
			stMsgReadRoomInfoRet* pRet = (stMsgReadRoomInfoRet*)prealMsg ;
			CLogMgr::SharedLogMgr()->PrintLog("read room info room id = %d",pRet->nRoomID);
			IRoom* pRoom = doCreateRoomObject((eRoomType)pRet->nRoomType) ;
			Json::Reader jsReader ;
			Json::Value jsRoot;
			CAutoBuffer auBufo (pRet->nJsonLen + 1 );
			auBufo.addContent( ((char*)pRet) + sizeof(stMsgReadRoomInfoRet),pRet->nJsonLen);
			jsReader.parse(auBufo.getBufferPtr(),jsRoot);
			stBaseRoomConfig* pConfig = m_pConfigMgr->GetConfigByConfigID(pRet->nConfigID) ;
			if ( pConfig == nullptr )
			{
				delete pRoom ;
				pRoom = nullptr ;
				CLogMgr::SharedLogMgr()->ErrorLog("read room info , room = %d , config = %d is null",pRet->nRoomID,pRet->nConfigID) ;
				break;
			}
			pRoom->serializationFromDB(pConfig,pRet->nRoomID,jsRoot);
			m_vRooms[pRoom->getRoomID()] = pRoom ;

			if ( pRet->nRoomID > m_nMaxRoomID )
			{
				m_nMaxRoomID = pRet->nRoomID ;
			}

			addRoomToCreator(pRoom);
			addRoomToConfigRooms(pRoom);
		}
		break;
	case MSG_REQUEST_MY_OWN_ROOM_DETAIL:
		{
			stMsgToRoom* pRet = (stMsgToRoom*)prealMsg ;
			stMsgRequestMyOwnRoomDetailRet msgRet ;
			msgRet.nRet = 0 ;
			IRoom* pRoom = GetRoomByID(pRet->nRoomID);
			if ( pRoom == nullptr )
			{
				msgRet.nRet = 1 ;
				sendMsg(&msgRet,sizeof(msgRet),nSessionID) ;
				return true ;
			}

			msgRet.nRoomType = pRoom->getRoomType() ;
			msgRet.nCanWithdrawProfit = pRoom->getProfit() ;
			msgRet.nConfigID = pRoom->getConfigID() ;
			msgRet.nDeadTime = pRoom->getDeadTime() ;
			msgRet.nFollows = 2 ;
			msgRet.nRoomID = pRoom->getRoomID() ;
			msgRet.nTotalProfit = pRoom->getTotalProfit() ;
			sprintf_s(msgRet.vRoomName,sizeof(msgRet.vRoomName),"%s",pRoom->getRoomName());
			sendMsg(&msgRet,sizeof(msgRet),nSessionID) ;
		}
		break;
	case MSG_SVR_ENTER_ROOM:
		{
			stMsgSvrEnterRoomRet msgBack ;
			msgBack.nRet = 0 ;
			stMsgSvrEnterRoom* pRet = (stMsgSvrEnterRoom*)prealMsg ;
			msgBack.nGameType = pRet->nGameType ;
			msgBack.nRoomID = pRet->nRoomID ;

			IRoom* pRoom = GetRoomByID(pRet->nRoomID) ;
			if ( pRoom == nullptr )
			{
				msgBack.nRet = 5 ;
				sendMsg(&msgBack,sizeof(msgBack),nSessionID) ;
				break;
			}

			if ( pRoom->getRoomType() != pRet->nGameType )
			{
				msgBack.nRet = 6 ;
				sendMsg(&msgBack,sizeof(msgBack),nSessionID) ;
				break;
			}

			msgBack.nRet = pRoom->canPlayerEnterRoom(&pRet->tPlayerData) ;
			msgBack.nRoomID = pRoom->getRoomID() ;
			sendMsg(&msgBack,sizeof(msgBack),nSessionID) ;

			if ( msgBack.nRet == 0 )
			{
				pRoom->onPlayerEnterRoom(&pRet->tPlayerData);
			}
		}
		break;
	case MSG_REQUEST_ROOM_LIST:
		{
			std::vector<IRoom*> vActiveRoom ;
			MAP_ID_ROOM::iterator iter = m_vRooms.begin() ;
			for ( ; iter != m_vRooms.end(); ++iter )
			{
				if ( iter->second->isRoomAlive() )
				{
					vActiveRoom.push_back(iter->second) ;
				}
			}

			stMsgRequestRoomListRet msgRet ;
			msgRet.nRoomCnt = vActiveRoom.size() ;
			msgRet.nRoomType = getMgrRoomType();
			CAutoBuffer auBuffer(sizeof(msgRet) + sizeof(uint32_t) * msgRet.nRoomCnt );
			auBuffer.addContent((char*)&msgRet,sizeof(msgRet)) ;
			for ( auto pRoom : vActiveRoom )
			{
				uint32_t nid = pRoom->getRoomID();
				auBuffer.addContent(&nid,sizeof(nid)) ;
			}
			sendMsg((stMsg*)auBuffer.getBufferPtr(),auBuffer.getContentSize(),nSessionID);
			CLogMgr::SharedLogMgr()->PrintLog("send msg room list ") ;
		}
		break;
	case MSG_REQUEST_ROOM_ITEM_DETAIL:
		{
			stMsgRequestRoomItemDetail* pret = (stMsgRequestRoomItemDetail*)prealMsg ;
			auto pRoom = GetRoomByID(pret->nRoomID);

			stMsgRequestRoomItemDetailRet msgRet ;
			msgRet.nRet = 1 ;
			msgRet.nRoomType = getMgrRoomType() ;
			msgRet.nRoomID = pret->nRoomID ;
			if ( pRoom )
			{
				msgRet.nRet = 0 ;
				Json::Value vDetail ;
				pRoom->roomItemDetailVisitor(vDetail) ;
				Json::StyledWriter write ;
				std::string strDe = write.write(vDetail);
				msgRet.nJsonLen = strDe.size() ;
				CAutoBuffer auBffer (sizeof(msgRet) + msgRet.nJsonLen );
				auBffer.addContent(&msgRet,sizeof(msgRet)) ;
				auBffer.addContent(strDe.c_str(),msgRet.nJsonLen) ;
				sendMsg((stMsg*)auBffer.getBufferPtr(),auBffer.getContentSize(),nSessionID) ;
				CLogMgr::SharedLogMgr()->PrintLog("send item detial room id = %d detail: %s",msgRet.nRoomID,strDe.c_str()) ;
			}
			else
			{
				msgRet.nJsonLen = 0 ;
				sendMsg(&msgRet,sizeof(msgRet),nSessionID) ;
				CLogMgr::SharedLogMgr()->ErrorLog("can not find room detail room id = %d",msgRet.nRoomID) ;
			}
		}
		break;
	default:
		return false;
	}
	return true ;
}

IRoom* IRoomManager::GetRoomByID(uint32_t nRoomID )
{
	MAP_ID_ROOM::iterator iter = m_vRooms.find(nRoomID) ;
	if ( iter != m_vRooms.end() )
	{
		return iter->second ;
	}
	return NULL ;
}

//IRoom* CRoomManager::GetQuickEnterRoom(uint64_t nCoin )
//{
//	LIST_ROOM vValidRooms , vLiveRooms;
//	MAP_ID_ROOM::iterator iter = m_vRooms.begin() ;
//	for ( ; iter != m_vRooms.end() ; ++iter )
//	{
//		if ( iter->second->isRoomAlive() )
//		{
//			uint8_t nPCnt = iter->second->GetPlayerCntWithState(eRoomPeer_SitDown);
//			if ( nPCnt > 0 && nPCnt != iter->second->getSeatCnt() )
//			{
//				vValidRooms.push_back(iter->second);
//			}
//			else
//			{
//				vLiveRooms.push_back(iter->second) ;
//			}
//		}
//	}
//
//	uint16_t nRandom = 0 ;
//	if ( vValidRooms.empty() == false )
//	{
//		nRandom = rand() % vValidRooms.size() ;
//		LIST_ROOM::iterator iter = vValidRooms.begin() ;
//		while ( nRandom-- )
//		{
//			++iter ;
//		}
//
//		if ( iter == vValidRooms.end() )
//		{
//			--iter ;
//			CLogMgr::SharedLogMgr()->ErrorLog("will not run to here   iter == vValidRooms.end() ");
//		}
//		return *iter ;
//	}
//
//	if ( vLiveRooms.empty() == false )
//	{
//		nRandom = rand() % vLiveRooms.size() ;
//		LIST_ROOM::iterator iterL = vLiveRooms.begin() ;
//		while ( nRandom-- )
//		{
//			++iterL ;
//		}
//
//		if ( iterL == vLiveRooms.end() )
//		{
//			--iterL ;
//			CLogMgr::SharedLogMgr()->ErrorLog("will not run to here   iter == vLiveRooms.end() ");
//		}
//		return *iterL ;
//	}
//	return nullptr ;
//}

void IRoomManager::update(float fDelta )
{
	// check delete rooms ;
	LIST_ROOM vDoDelteRoom ;

	MAP_ID_ROOM::iterator iter = m_vRooms.begin() ;
	for ( ; iter != m_vRooms.end() ; ++iter )
	{
		if ( iter->second )
		{
			iter->second->update(fDelta) ;
		}

		// for check delete 
		if ( iter->second->isDeleteRoom() && iter->second->getCurRoomState()->getStateID() == eRoomState_Dead )
		{
			vDoDelteRoom.push_back(iter->second) ;
		}
	}

	for ( IRoom* pRoom : vDoDelteRoom )
	{
		pRoom->onRoomWillDoDelete();
		doDeleteRoom(pRoom) ;
	}

	// check time save 
	m_fTimeSaveTicket -= fDelta ;
	if ( m_fTimeSaveTicket < 0 )
	{
		m_fTimeSaveTicket = TIME_SAVE_ROOM_INFO ;
		onTimeSave();
	}
}

void IRoomManager::onHttpCallBack(char* pResultData, size_t nDatalen , void* pUserData , size_t nUserTypeArg)
{
	if ( nUserTypeArg == eCrossSvrReq_CreateRoom )
	{
		uint32_t nChatRoomID = 0 ;
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

		IRoom* pRoom = (IRoom*)pUserData ;

		stMsgCrossServerRequestRet msgRet ;
		msgRet.cSysIdentifer = ID_MSG_PORT_DATA ;
		msgRet.nReqOrigID = pRoom->getRoomID();
		msgRet.nTargetID = pRoom->getOwnerUID();
		msgRet.nRequestType = eCrossSvrReq_CreateRoom ;
		msgRet.nRequestSubType = eCrossSvrReqSub_Default;
		msgRet.nRet = bSuccess ? 0 : 2 ;
		msgRet.vArg[0] = pRoom->getConfigID() ;
		msgRet.vArg[1] = pRoom->getRoomID() ;
		msgRet.vArg[2] = pRoom->getRoomType() ;
		msgRet.vArg[3] = ( pRoom->getDeadTime() - pRoom->getCreateTime() ) / (60*60*24) ;
		sendMsg(&msgRet,sizeof(msgRet),msgRet.nTargetID);

		if ( bSuccess )
		{
			pRoom->setChatRoomID(nChatRoomID);

			pRoom->serializationToDB(true);
			addRoomToCreator(pRoom);
			addRoomToConfigRooms(pRoom);
		}
		else
		{
			if ( m_vRooms.find(pRoom->getRoomID())!= m_vRooms.end() )
			{
				m_vRooms.erase(m_vRooms.find(pRoom->getRoomID())) ;
				delete pRoom ;
				pRoom = nullptr ;
			}
		}
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

bool IRoomManager::onCrossServerRequest(stMsgCrossServerRequest* pRequest , eMsgPort eSenderPort,Json::Value* vJsValue)
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
		msgRet.vArg[2] = eRoom_NiuNiu ;

		IRoom* pRoom = GetRoomByID(pRequest->nTargetID );
		if ( pRoom != nullptr )
		{
			msgRet.vArg[1] = pRoom->getProfit();
			pRoom->setProfit(0) ;
			pRoom->addTotoalProfit(msgRet.vArg[1]) ;
		}
		else
		{
			msgRet.nRet = 1 ;
			msgRet.vArg[1] = 0;
		}
		sendMsg(&msgRet,sizeof(msgRet),msgRet.nTargetID);
		return true ;
	}
	else if ( eCrossSvrReq_AddRentTime == pRequest->nRequestType )
	{
		IRoom* pRoom = GetRoomByID(pRequest->nTargetID) ;
		stMsgCrossServerRequestRet msgRet ;
		msgRet.cSysIdentifer = ID_MSG_PORT_DATA ;
		msgRet.nReqOrigID = pRequest->nTargetID ;
		msgRet.nTargetID = pRequest->nReqOrigID ;
		msgRet.nRequestType = pRequest->nRequestType ;
		msgRet.nRequestSubType = pRequest->nRequestSubType ;
		msgRet.nRet = 0 ;
		msgRet.vArg[0] = pRequest->vArg[0] ;
		msgRet.vArg[1] = pRequest->vArg[1] ;
		msgRet.vArg[2] = pRequest->vArg[2] ;
		if ( pRoom == nullptr )
		{
			msgRet.nRet = 1 ;
		}
		else
		{
			pRoom->addLiftTime(pRequest->vArg[0]) ;
		}
		sendMsg(&msgRet,sizeof(msgRet),msgRet.nTargetID);
		return true ;
	}
	else if ( eCrossSvrReq_CreateRoom == pRequest->nRequestType )
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
		msgRet.vArg[2] = pRequest->vArg[2] ;
		msgRet.vArg[3] = pRequest->vArg[1] ;
		Json::Value vCreateJs ;
		vCreateJs["name"] = strName ;
		vCreateJs["ownerUID"] = pRequest->nReqOrigID ; 
		vCreateJs["duringTime"] = (uint32_t)pRequest->vArg[1] * 60 ;
		IRoom* pRoom = doCreateInitedRoomObject(++m_nMaxRoomID,nConfigID,(eRoomType)pRequest->vArg[2],vCreateJs);
		if ( pRoom == nullptr )
		{
			--m_nMaxRoomID;
			msgRet.nRet = 1;
			sendMsg(&msgRet,sizeof(msgRet),msgRet.nTargetID);
			return true ;
		}

		m_vRooms[pRoom->getRoomID()] = pRoom ;
		pRoom->setRoomName(strName.c_str());
		if ( false == reqeustChatRoomID(pRoom) )
		{
			CLogMgr::SharedLogMgr()->ErrorLog("uid = %d create room failed can not connect to chat svr",pRoom->getOwnerUID());
			m_vRooms.erase(m_vRooms.find(pRoom->getRoomID())) ;
			delete pRoom ;
			pRoom = nullptr ;
			msgRet.nRet = 3;
			sendMsg(&msgRet,sizeof(msgRet),msgRet.nTargetID);
			return true ;
		}
		return true ;
	}
	else if ( eCrossSvrReq_DeleteRoom == pRequest->nRequestType )
	{
		IRoom* pRoom = GetRoomByID(pRequest->vArg[1] );
		if ( pRoom == nullptr )
		{
			CLogMgr::SharedLogMgr()->ErrorLog("uid = %d delete room = %d , not exsit",pRequest->nReqOrigID,pRequest->vArg[1]) ;
			return true ;
		}
		removeRoom(pRoom) ;
		return true ;
	}
	else if ( eCrossSvrReq_ApplyLeaveRoom == pRequest->nRequestType )
	{
		IRoom* pRoom = GetRoomByID(pRequest->nTargetID );
		if ( pRoom )
		{
			IRoom::stStandPlayer* pp = pRoom->getPlayerByUserUID(pRequest->nReqOrigID) ;
			if ( pp )
			{
				pRoom->onPlayerWillLeaveRoom(pp) ;
				pRoom->playerDoLeaveRoom(pp);
			}
			else
			{
				stMsgCrossServerRequest msgEnter ;
				msgEnter.cSysIdentifer = ID_MSG_PORT_DATA ;
				msgEnter.nJsonsLen = 0 ;
				msgEnter.nReqOrigID = pRoom->getRoomID();
				msgEnter.nRequestSubType = eCrossSvrReqSub_Default ;
				msgEnter.nRequestType = eCrossSvrReq_LeaveRoomRet ;
				msgEnter.nTargetID = pRequest->nReqOrigID ;
				msgEnter.vArg[0] = pRoom->getRoomType() ;
				msgEnter.vArg[1] = pRoom->getRoomID() ;
				pRoom->sendMsgToPlayer(&msgEnter,sizeof(msgEnter),pRoom->getRoomID()) ;
				CLogMgr::SharedLogMgr()->PrintLog("you are not in room but i let you go!") ;
			}
		}
		else
		{
			stMsgCrossServerRequest msgEnter ;
			msgEnter.cSysIdentifer = ID_MSG_PORT_DATA ;
			msgEnter.nJsonsLen = 0 ;
			msgEnter.nReqOrigID = pRequest->nTargetID;
			msgEnter.nRequestSubType = eCrossSvrReqSub_Default ;
			msgEnter.nRequestType = eCrossSvrReq_LeaveRoomRet ;
			msgEnter.nTargetID = pRequest->nReqOrigID ;
			msgEnter.vArg[0] = pRequest->vArg[2] ;
			msgEnter.vArg[1] = pRequest->nTargetID ;
			sendMsg(&msgEnter,sizeof(msgEnter),pRequest->nTargetID);
			CLogMgr::SharedLogMgr()->PrintLog("can not find room ,  but i let you go!") ;
		}
		return true ;
	}
	return false ;
}

bool IRoomManager::onCrossServerRequestRet(stMsgCrossServerRequestRet* pResult,Json::Value* vJsValue)
{
	return false ;
}

bool IRoomManager::reqeustChatRoomID(IRoom* pRoom)
{
	Json::Value cValue ;
	cValue["email"] = "378569952@qq.com" ;
	cValue["devpwd"] = "bill007" ;
	cValue["appkey"] = "abffee4b-deea-4e96-ac8d-b9d58f246c3f" ;
	cValue["room_name"] = pRoom->getRoomID() ;
	cValue["room_type"] = 1;
	cValue["room_create_type"] = 0 ;
	Json::StyledWriter sWrite ;
	std::string str = sWrite.write(cValue);
	return m_pGoTyeAPI.performRequest("CreateRoom",str.c_str(),str.size(),pRoom,eCrossSvrReq_CreateRoom);
}

void IRoomManager::onConnectedToSvr()
{
	if ( m_vRooms.empty() )
	{
		stMsgReadRoomInfo msgRead ;
		msgRead.nRoomType = getMgrRoomType() ;
		sendMsg(&msgRead,sizeof(msgRead),0) ;
		//CLogMgr::SharedLogMgr()->ErrorLog("test stage do not read room info");
		CLogMgr::SharedLogMgr()->PrintLog("read room info ") ;
	}
}

void IRoomManager::addRoomToCreator(IRoom* pRoom)
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

void IRoomManager::addRoomToConfigRooms(IRoom* pRoom)
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

bool IRoomManager::getRoomCreatorRooms(uint32_t nCreatorUID, LIST_ROOM& vInfo )
{
	MAP_UID_CR::iterator iter = m_vCreatorAndRooms.find(nCreatorUID) ;
	if ( iter == m_vCreatorAndRooms.end() )
	{
		return false ;
	}
	vInfo = iter->second.vRooms ;
	return true ;
}

void IRoomManager::onTimeSave()
{
	MAP_ID_ROOM::iterator iter = m_vRooms.begin() ;
	for ( ; iter != m_vRooms.end() ; ++iter )
	{
		if ( iter->second )
		{
			iter->second->onTimeSave();
		}
	}
}

void IRoomManager::removeRoom(IRoom* pRoom )
{
	pRoom->deleteRoom();
	// remove from db ;
	stMsgSaveDeleteRoom msgSaveDelte ;
	msgSaveDelte.nRoomID = pRoom->getRoomID() ;
	msgSaveDelte.nRoomType = pRoom->getRoomType() ;
	sendMsg(&msgSaveDelte,sizeof(msgSaveDelte),0 ) ;
}

void IRoomManager::doDeleteRoom(IRoom* pRoom )
{
	// remove m_vRooms ;
	auto iter = m_vRooms.find(pRoom->getRoomID()) ;
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
			if ( (*iterC)->getRoomID() == pRoom->getRoomID() )
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
			if ( (*iterC)->getRoomID() == pRoom->getRoomID() )
			{
				list.erase(iterC) ;
				break;
			}
		}
	}

	// remove from db ;
	stMsgSaveDeleteRoom msgSaveDelte ;
	msgSaveDelte.nRoomID = pRoom->getRoomID() ;
	msgSaveDelte.nRoomType = pRoom->getRoomType() ;
	sendMsg(&msgSaveDelte,sizeof(msgSaveDelte),0 ) ;

	// delete chat room id ;
	Json::Value cValue ;
	cValue["email"] = "378569952@qq.com" ;
	cValue["devpwd"] = "bill007" ;
	cValue["appkey"] = "abffee4b-deea-4e96-ac8d-b9d58f246c3f" ;
	cValue["room_id"] = pRoom->getChatRoomID() ;
	Json::StyledWriter sWrite ;
	std::string str = sWrite.write(cValue);
	m_pGoTyeAPI.performRequest("DeleteRoom",str.c_str(),str.size(),nullptr,eCrossSvrReq_DeleteRoom);

	CLogMgr::SharedLogMgr()->SystemLog("do delete room id = %d",pRoom->getRoomID()) ;
	// delete room ;
	delete pRoom ;
	pRoom = nullptr ;
	return ;
}

