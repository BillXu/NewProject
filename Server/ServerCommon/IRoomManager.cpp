#include "IRoomManager.h"
#include "LogManager.h"
#include "IRoom.h"
#include "ServerMessageDefine.h"
#include "RoomConfig.h"
#include "AutoBuffer.h"
#include <cassert>
IRoomManager::IRoomManager()
{

}

IRoomManager::~IRoomManager()
{

}

bool IRoomManager::init()
{
	m_nMaxRoomID = 1 ;
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
			IRoom* pRoom = GetRoomByID(pRet->nTargetID);
			assert(pRoom&&"this request no one to process or target id error");
			return pRoom->onCrossServerRequestRet(pRet,pJsValue);
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
	return false ;
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
	MAP_ID_ROOM::iterator iter = m_vRooms.begin() ;
	for ( ; iter != m_vRooms.end() ; ++iter )
	{
		if ( iter->second )
		{
			iter->second->update(fDelta) ;
		}
	}
}

void IRoomManager::onHttpCallBack(char* pResultData, size_t nDatalen , void* pUserData , size_t nUserTypeArg)
{
	if ( nUserTypeArg == eCrossSvrReq_CreateRoom )
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

		IRoom* pRoom = (IRoom*)pUserData ;
		onGetChatRoomIDResult(pRoom,bSuccess) ;
		if ( bSuccess )
		{
			pRoom->setChatRoomID(nChatRoomID);
			pRoom->onTimeSave(true);
			addRoomToCreator(pRoom);
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
}

void IRoomManager::onGetChatRoomIDResult(IRoom* pNewRoom, bool bSuccess )
{

}

bool IRoomManager::onCrossServerRequest(stMsgCrossServerRequest* pRequest , eMsgPort eSenderPort,Json::Value* vJsValue)
{
	if ( eCrossSvrReq_CreateRoom == pRequest->nRequestType )
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
		IRoom* pRoom = doCreateInitedRoomObject(++m_nMaxRoomID,nConfigID);
		if ( pRoom == nullptr )
		{
			--m_nMaxRoomID;
			msgRet.nRet = 1;
			sendMsg(&msgRet,sizeof(msgRet),msgRet.nTargetID);
			return true ;
		}

		m_vRooms[pRoom->getRoomID()] = pRoom ;
		pRoom->onCreateByPlayer(pRequest->nReqOrigID,pRequest->vArg[1]);
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
	return m_pGoTyeAPI.performRequest("CreateRoom",str.c_str(),str.size(),pRoom,eCrossSvrReq_CreateTaxasRoom);
}

void IRoomManager::onConnectedToSvr()
{

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