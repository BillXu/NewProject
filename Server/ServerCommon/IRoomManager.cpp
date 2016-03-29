#include "IRoomManager.h"
#include "LogManager.h"
#include "IRoomInterface.h"
#include "ServerMessageDefine.h"
#include "RoomConfig.h"
#include "AutoBuffer.h"
#include <cassert>
#include "IRoomState.h"
#include "RoomConfig.h"
#include "ISeverApp.h"
#define ROOM_LIST_ITEM_CNT_PER_PAGE 5 
#define TIME_SAVE_ROOM_INFO 60*10
IRoomManager::IRoomManager(CRoomConfigMgr* pConfigMgr)
{
	m_pConfigMgr = pConfigMgr ;
}

IRoomManager::~IRoomManager()
{

}

void IRoomManager::init(IServerApp* svrApp)
{
	IGlobalModule::init(svrApp);

	m_nMaxRoomID = 1 ;
	m_vRooms.clear();
	m_vCreatorAndRooms.clear();
	m_pGoTyeAPI.init("https://qplusapi.gotye.com.cn:8443/api/");
	m_pGoTyeAPI.setDelegate(this);
}

void IRoomManager::sendMsg(stMsg* pmsg, uint32_t nLen , uint32_t nSessionID )
{
	getSvrApp()->sendMsg(nSessionID,(char*)pmsg,nLen);
}

bool IRoomManager::onMsg( stMsg* prealMsg , eMsgPort eSenderPort , uint32_t nSessionID )
{
	if ( IGlobalModule::onMsg(prealMsg,eSenderPort,nSessionID) )
	{
		return true ;
	}

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
	IRoomInterface* pRoom = GetRoomByID(pRoomMsg->nRoomID) ;
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
	case MSG_GET_MAX_ROOM_ID:
		{
			stMsgGetMaxRoomIDRet* pRet = (stMsgGetMaxRoomIDRet*)prealMsg ;
			m_nMaxRoomID = pRet->nMaxRoomID ;
		}
		break;
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
			for ( IRoomInterface* proom : vRL )
			{
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
			IRoomInterface* pRoom = doCreateRoomObject((eRoomType)pRet->nRoomType,pRet->nRoomOwnerUID != MATCH_MGR_UID ) ;
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
			pRoom->serializationFromDB(this,pConfig,pRet->nRoomID,jsRoot);
			m_vRooms[pRoom->getRoomID()] = pRoom ;

			if ( pRet->nRoomID > m_nMaxRoomID )
			{
				m_nMaxRoomID = pRet->nRoomID ;
			}

			addRoomToCreator(pRoom->getOwnerUID(),pRoom);
		}
		break;
	//case MSG_REQUEST_MY_OWN_ROOM_DETAIL:
	//	{
	//		stMsgToRoom* pRet = (stMsgToRoom*)prealMsg ;
	//		stMsgRequestMyOwnRoomDetailRet msgRet ;
	//		msgRet.nRet = 0 ;
	//		IRoomInterface* pRoom = GetRoomByID(pRet->nRoomID);
	//		if ( pRoom == nullptr )
	//		{
	//			msgRet.nRet = 1 ;
	//			sendMsg(&msgRet,sizeof(msgRet),nSessionID) ;
	//			return true ;
	//		}

	//		msgRet.nRoomType = pRoom->getRoomType() ;
	//		msgRet.nRoomID = pRoom->getRoomID() ;
	//		sendMsg(&msgRet,sizeof(msgRet),nSessionID) ;
	//	}
	//	break;
	case MSG_SVR_ENTER_ROOM:
		{
			stMsgSvrEnterRoomRet msgBack ;
			msgBack.nRet = 0 ;
			stMsgSvrEnterRoom* pRet = (stMsgSvrEnterRoom*)prealMsg ;
			msgBack.nGameType = pRet->nGameType ;
			msgBack.nRoomID = pRet->nRoomID ;
			
			IRoomInterface* pRoom = GetRoomByID(pRet->nRoomID) ;
			if ( pRoom == nullptr )
			{
				msgBack.nRet = 8 ;
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
			msgBack.nSubIdx = (uint8_t)pRet->nSubIdx ;
			sendMsg(&msgBack,sizeof(msgBack),nSessionID) ;

			if ( msgBack.nRet == 0 )
			{
				pRoom->onPlayerEnterRoom(&pRet->tPlayerData,msgBack.nSubIdx);
			}
		}
		break;
	case MSG_REQUEST_ROOM_LIST:
		{
			LIST_ROOM vRL ;
			if ( getRoomCreatorRooms(MATCH_MGR_UID,vRL) == false )
			{
				CLogMgr::SharedLogMgr()->PrintLog("system not create room so , need not respone list" ) ;
				return true ;
			}

			stMsgRequestRoomListRet msgRet ;
			msgRet.nRoomCnt = vRL.size() ;
			msgRet.nRoomType = getMgrRoomType();
			CAutoBuffer auBuffer(sizeof(msgRet) + sizeof(uint32_t) * msgRet.nRoomCnt );
			auBuffer.addContent((char*)&msgRet,sizeof(msgRet)) ;
			for ( auto pRoom : vRL )
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
			msgRet.nOwnerUID = 0 ;
			if ( pRoom )
			{
				if ( pRoom->getOwnerUID() != MATCH_MGR_UID )
				{
					msgRet.nOwnerUID = pRoom->getOwnerUID() ;
				}

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

IRoomInterface* IRoomManager::GetRoomByID(uint32_t nRoomID )
{
	MAP_ID_ROOM::iterator iter = m_vRooms.find(nRoomID) ;
	if ( iter != m_vRooms.end() )
	{
		return iter->second ;
	}
	return NULL ;
}

void IRoomManager::update(float fDelta )
{
	IGlobalModule::update(fDelta);
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
		if ( iter->second->isDeleteRoom())
		{
			vDoDelteRoom.push_back(iter->second) ;
		}
	}

	for ( IRoomInterface* pRoom : vDoDelteRoom )
	{
		removeRoom(pRoom);
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
		if ( bSuccess )
		{
			pRoom->setChatRoomID(nChatRoomID);
		}
		else
		{
			CLogMgr::SharedLogMgr()->ErrorLog("request chat room id error ") ;
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
	/*	if ( eCrossSvrReq_RoomProfit == pRequest->nRequestType )
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
	else */
	if ( eCrossSvrReq_CreateRoom == pRequest->nRequestType )
	{
		uint16_t nConfigID = (uint16_t)pRequest->vArg[0];
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
		vCreateJs["duringTime"] = (uint32_t)pRequest->vArg[1] * 60 ;
		vCreateJs["ownerUID"] = (uint32_t)pRequest->nReqOrigID;
		if ( (*vJsValue)["subRoomCnt"].isNull() == false )
		{
			vCreateJs["subRoomCnt"] = (*vJsValue)["subRoomCnt"].asUInt();
		}
		CLogMgr::SharedLogMgr()->PrintLog("recived create room name = %s",strName.c_str()) ;
		IRoomInterface* pRoom = doCreateInitedRoomObject(++m_nMaxRoomID,pRequest->nReqOrigID != MATCH_MGR_UID,nConfigID,(eRoomType)pRequest->vArg[2],vCreateJs);
		if ( pRoom == nullptr )
		{
			--m_nMaxRoomID;
			msgRet.nRet = 1;
			sendMsg(&msgRet,sizeof(msgRet),msgRet.nTargetID);
			return true ;
		}

		m_vRooms[pRoom->getRoomID()] = pRoom ;
		addRoomToCreator(pRequest->nReqOrigID,pRoom);

		msgRet.vArg[1] = pRoom->getRoomID();
		sendMsg(&msgRet,sizeof(msgRet),msgRet.nTargetID);
		return true ;
	}
	else if ( eCrossSvrReq_DeleteRoom == pRequest->nRequestType )
	{
		IRoomInterface* pRoom = GetRoomByID((uint32_t)pRequest->vArg[1] );
		if ( pRoom == nullptr )
		{
			CLogMgr::SharedLogMgr()->ErrorLog("uid = %d delete room = %d , not exsit",pRequest->nReqOrigID,pRequest->vArg[1]) ;
			return true ;
		}
		pRoom->deleteRoom();
		return true ;
	}
	else if ( eCrossSvrReq_ApplyLeaveRoom == pRequest->nRequestType )
	{
		IRoomInterface* pRoom = GetRoomByID(pRequest->nTargetID );
		if ( pRoom )
		{
			if ( pRoom->onPlayerApplyLeaveRoom(pRequest->nReqOrigID) )
			{

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
				sendMsg(&msgEnter,sizeof(msgEnter),pRoom->getRoomID()) ;
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

void IRoomManager::onConnectedSvr()
{
	IGlobalModule::onConnectedSvr();
	stMsgGetMaxRoomID msgBack ;
	sendMsg(&msgBack,sizeof(msgBack),0) ;

	if ( m_vRooms.empty() )
	{
		stMsgReadRoomInfo msgRead ;
		msgRead.nRoomType = getMgrRoomType() ;
		sendMsg(&msgRead,sizeof(msgRead),0) ;
		//CLogMgr::SharedLogMgr()->ErrorLog("test stage do not read room info");
		CLogMgr::SharedLogMgr()->PrintLog("read room info ") ;
	}
}

void IRoomManager::addRoomToCreator(uint32_t nOwnerUID ,IRoomInterface* pRoom)
{
	MAP_UID_CR::iterator iter =  m_vCreatorAndRooms.find(nOwnerUID);
	if ( iter != m_vCreatorAndRooms.end() )
	{
		iter->second.vRooms.push_back(pRoom) ;
		return ;
	}

	stRoomCreatorInfo sInfo ;
	sInfo.nPlayerUID = nOwnerUID ;
	sInfo.vRooms.push_back(pRoom) ;
	m_vCreatorAndRooms[sInfo.nPlayerUID] = sInfo ;
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
	IGlobalModule::onTimeSave();
	MAP_ID_ROOM::iterator iter = m_vRooms.begin() ;
	for ( ; iter != m_vRooms.end() ; ++iter )
	{
		if ( iter->second )
		{
			iter->second->onTimeSave();
		}
	}
}

void IRoomManager::removeRoom( IRoomInterface* pRoom )
{
	// remove from db ;
	stMsgSaveDeleteRoom msgSaveDelte ;
	msgSaveDelte.nRoomID = pRoom->getRoomID() ;
	msgSaveDelte.nRoomType = pRoom->getRoomType() ;
	sendMsg(&msgSaveDelte,sizeof(msgSaveDelte),0 ) ;

	// remove m_vRooms ;
	auto iter = m_vRooms.find(pRoom->getRoomID()) ;
	if ( iter != m_vRooms.end() )
	{
		m_vRooms.erase(iter) ;
	}

	// remove m_vCreatorAndRooms ; maybe sys create
	auto iter_Create = m_vCreatorAndRooms.find( pRoom->getOwnerUID() ) ;
	if ( iter_Create != m_vCreatorAndRooms.end() )
	{
		LIST_ROOM& list = iter_Create->second.vRooms ;
		LIST_ROOM::iterator iterC = list.begin() ;
		for ( ; iterC != list.end(); ++iterC )
		{
			if ( (*iterC)->getRoomID() == pRoom->getRoomID() )
			{
				list.erase(iterC) ;
				break; ;
			}
		}
	}

	delete pRoom ;
	pRoom = nullptr ;
}

void IRoomManager::deleteRoomChatID( uint32_t nChatID )
{
	// delete chat room id ;
	Json::Value cValue ;
	cValue["email"] = "378569952@qq.com" ;
	cValue["devpwd"] = "bill007" ;
	cValue["appkey"] = "abffee4b-deea-4e96-ac8d-b9d58f246c3f" ;
	cValue["room_id"] = nChatID ;
	Json::StyledWriter sWrite ;
	std::string str = sWrite.write(cValue);
	m_pGoTyeAPI.performRequest("DeleteRoom",str.c_str(),str.size(),nullptr,eCrossSvrReq_DeleteRoom);

	CLogMgr::SharedLogMgr()->PrintLog("delte chat room id = %u",nChatID) ;
}

