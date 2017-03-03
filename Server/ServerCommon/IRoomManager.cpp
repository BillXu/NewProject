#include "IRoomManager.h"
#include "log4z.h"
#include "IRoomInterface.h"
#include "ServerMessageDefine.h"
#include "RoomConfig.h"
#include "AutoBuffer.h"
#include <cassert>
#include "IRoomState.h"
#include "RoomConfig.h"
#include "ISeverApp.h"
#include "AsyncRequestQuene.h"
#include "ConfigDefine.h"
#define ROOM_LIST_ITEM_CNT_PER_PAGE 5 
#define TIME_SAVE_ROOM_INFO 60*10
IRoomManager::IRoomManager(CRoomConfigMgr* pConfigMgr)
{
	m_pConfigMgr = pConfigMgr ;
	//m_eRequestChatRoomIDs = eOperate_NotDo ;
}

IRoomManager::~IRoomManager()
{
 
}

void IRoomManager::init(IServerApp* svrApp)
{
	IGlobalModule::init(svrApp);

	m_vRooms.clear();
	m_pGoTyeAPI.init("https://qplusapi.gotye.com.cn:8443/api/");
	m_pGoTyeAPI.setDelegate(this);
}

void IRoomManager::sendMsg(stMsg* pmsg, uint32_t nLen , uint32_t nSessionID )
{
	getSvrApp()->sendMsg(nSessionID,(char*)pmsg,nLen);
}

bool IRoomManager::sendMsg( uint32_t nSessionID , Json::Value& recvValue, uint16_t nMsgID ,uint8_t nTargetPort )
{
	return getSvrApp()->sendMsg(nSessionID,recvValue,nMsgID,nTargetPort,false);
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
		//LOGFMTI("request crose type = %d",pRet->nRequestType);
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
				LOGFMTE("unprocess cross svr request type = %d",pRet->nRequestType) ;
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
		return false ;
	}

	////if ( prealMsg->usMsgType <= MSG_TP_BEGIN || MSG_TP_END <= prealMsg->usMsgType )
	////{
	////	LOGFMTE("why this msg send here msg = %d ",prealMsg->usMsgType ) ;
	////	return false ;
	////}



	// msg give to room process 
	stMsgToRoom* pRoomMsg = (stMsgToRoom*)prealMsg;
	IRoomInterface* pRoom = GetRoomByID(pRoomMsg->nRoomID) ;
	if ( pRoom == NULL )
	{
		LOGFMTE("can not find room to process id = %d ,from = %d, room id = %d",prealMsg->usMsgType,eSenderPort,pRoomMsg->nRoomID ) ;
		Json::Value jsmsg;
		jsmsg["roomID"] = pRoomMsg->nRoomID;
		jsmsg["ret"] = 100;
		getSvrApp()->sendMsg(nSessionID, jsmsg,pRoomMsg->usMsgType );
		return  false ;
	}

	return pRoom->onMessage(prealMsg,eSenderPort,nSessionID) ;
}

bool IRoomManager::onPublicMsg(stMsg* prealMsg , eMsgPort eSenderPort , uint32_t nSessionID)
{
	switch ( prealMsg->usMsgType )
	{
	case MSG_READ_GAME_RESULT:
		{
			stMsgReadGameResultRet* pRet = (stMsgReadGameResultRet*)prealMsg ;
			stPrivateRoomRecorder* pRecorder = new stPrivateRoomRecorder ;
			pRecorder->nRoomID = pRet->nRoomID ;
			pRecorder->nCreaterUID = pRet->nCreaterUID ;
			pRecorder->nConfigID = pRet->nConfigID ;
			pRecorder->nTime = pRet->tTime ;
			pRecorder->nDuringSeconds = pRet->nDuringSeconds ;
			pRecorder->nSieralNum = pRet->nSieralNum;

			char* pJsBuffer = (char*)prealMsg ;
			pJsBuffer += sizeof(stMsgReadGameResultRet) ;

			Json::Reader jsReader ;
			jsReader.parse(pJsBuffer,pJsBuffer + pRet->nJsLen,pRecorder->playerDetail) ;
			addPrivateRoomRecorder(pRecorder,false) ;
			//LOGFMTD("read game result is room id = %u",pRet->nRoomID ) ;
		}
		break ;
	case MSG_REQUEST_PRIVATE_ROOM_RECORDER:
		{
			stMsgRequestPrivateRoomRecorder* pRet = (stMsgRequestPrivateRoomRecorder*)prealMsg ;
			auto iter = m_mapPrivateRecorder.find(pRet->nRoomID) ;
			stMsgRequestPrivateRoomRecorderRet msgBack ;
			if ( iter == m_mapPrivateRecorder.end() )
			{
				msgBack.nRet = 1 ;
				msgBack.nRoomID = pRet->nRoomID ;
				msgBack.nJsLen = 0 ;
				sendMsg(&msgBack,sizeof(msgBack),nSessionID) ;
				LOGFMTD("can not find to send room record msg ") ;
				break ;
			}

			msgBack.nRet = 0 ;
			msgBack.nConfigID = iter->second->nConfigID ;
			msgBack.nCreaterUID = iter->second->nCreaterUID ;
			msgBack.nRoomID = iter->second->nRoomID ;
			msgBack.tTime = (uint32_t)iter->second->nTime ;
			msgBack.nRoomType = getMgrRoomType();

			Json::StyledWriter jsWrite ;
			std::string strJon = jsWrite.write(iter->second->playerDetail) ;

			msgBack.nJsLen = strJon.size() ;

			CAutoBuffer buBffer(sizeof(msgBack) + msgBack.nJsLen );
			buBffer.addContent(&msgBack,sizeof(msgBack)) ;
			buBffer.addContent(strJon.c_str(),msgBack.nJsLen) ;
			sendMsg((stMsg*)buBffer.getBufferPtr(),buBffer.getContentSize(),nSessionID) ;
			LOGFMTD("send room record msg ") ;
		}
		break ;
		case MSG_REQUEST_PRIVATE_ROOM_RECORDER_NEW:
		{
			stMsgRequestPrivateRoomRecorderNew* pRet = (stMsgRequestPrivateRoomRecorderNew*)prealMsg;
			auto iter = m_mapSieralPrivateRecorder.find(pRet->nSieralNum);
			stMsgRequestPrivateRoomRecorderNewRet msgBack ;
			if (iter == m_mapSieralPrivateRecorder.end())
			{
				msgBack.nRet = 1 ;
				msgBack.nRoomID = pRet->nRoomID ;
				msgBack.nJsLen = 0 ;
				sendMsg(&msgBack,sizeof(msgBack),nSessionID) ;
				LOGFMTD("can not find to send room record msg ") ;
				break ;
			}

			msgBack.nRet = 0 ;
			msgBack.nConfigID = iter->second->nConfigID ;
			msgBack.nCreaterUID = iter->second->nCreaterUID ;
			msgBack.nRoomID = iter->second->nRoomID ;
			msgBack.tTime = (uint32_t)iter->second->nTime ;
			msgBack.nSieralNum = iter->second->nSieralNum;
			msgBack.nRoomType = getMgrRoomType();

			Json::StyledWriter jsWrite ;
			std::string strJon = jsWrite.write(iter->second->playerDetail) ;

			msgBack.nJsLen = strJon.size() ;

			CAutoBuffer buBffer(sizeof(msgBack) + msgBack.nJsLen );
			buBffer.addContent(&msgBack,sizeof(msgBack)) ;
			buBffer.addContent(strJon.c_str(),msgBack.nJsLen) ;
			sendMsg((stMsg*)buBffer.getBufferPtr(),buBffer.getContentSize(),nSessionID) ;
			LOGFMTD("send room record msg ") ;
		}
		break ;
	case MSG_SVR_ENTER_ROOM:
		{
			stMsgSvrEnterRoomRet msgBack ;
			msgBack.nRet = 0 ;
			stMsgSvrEnterRoom* pRet = (stMsgSvrEnterRoom*)prealMsg ;
			msgBack.nRoomID = pRet->nRoomID ;
			
			IRoomInterface* pRoom = GetRoomByID(pRet->nRoomID) ;
			if ( pRoom == nullptr )
			{
				msgBack.nRet = 8 ;
				sendMsg(&msgBack,sizeof(msgBack),nSessionID) ;
				break;
			}

			msgBack.nRet = pRoom->canPlayerEnterRoom(&pRet->tPlayerData) ;
			msgBack.nRoomID = pRoom->getRoomID() ;
			if ( msgBack.nRet == 0 )
			{
				pRoom->onPlayerEnterRoom(&pRet->tPlayerData,pRet->nSubIdx);
			}
			msgBack.nSubIdx = (uint8_t)pRet->nSubIdx ;
			sendMsg(&msgBack,sizeof(msgBack),nSessionID) ;
		}
		break;
	case MSG_REQUEST_ROOM_LIST:
		{
			LOGFMTD("send msg room list (invalid msg) ") ;
		}
		break;
	default:
		return false;
	}
	return true ;
}

bool IRoomManager::onMsg(Json::Value& prealMsg ,uint16_t nMsgType, eMsgPort eSenderPort , uint32_t nSessionID)
{
	switch (nMsgType)
	{
	case MSG_REQ_SELF_CREATE_ROOMS:
		{
			uint32_t nReqUID = prealMsg["uid"].asUInt() ;
			Json::Value jsRoomIDs ;
			for ( auto ref : m_vRooms )
			{
				if ( ref.second->getOwnerUID() == nReqUID )
				{
					jsRoomIDs[jsRoomIDs.size()] = ref.second->getRoomID();
				}
			}
			Json::Value jsMsg ;
			jsMsg["roomIDs"] = jsRoomIDs ;
			getSvrApp()->sendMsg(nSessionID,jsMsg,nMsgType) ;
		}
		break;
	case MSG_REQUEST_ROOM_ITEM_DETAIL:
		{
			Json::Value jsMsgBack ;
			jsMsgBack["ret"] = 0 ;
			auto pRoom = GetRoomByID(prealMsg["roomID"].asUInt());
			if ( pRoom == nullptr )
			{
				jsMsgBack["ret"] = 1 ;
				jsMsgBack["roomID"] = prealMsg["roomID"].asUInt() ;
				getSvrApp()->sendMsg(nSessionID,jsMsgBack,nMsgType) ;
				break;
			} 
			pRoom->roomItemDetailVisitor(jsMsgBack) ;
			getSvrApp()->sendMsg(nSessionID,jsMsgBack,nMsgType) ;
		}
		break;
	default:
		// msg give to room process 
		if ( prealMsg["roomID"].isNull() )
		{
			return false ;
		}

		uint32_t nRoomID = prealMsg["roomID"].asUInt() ;
		IRoomInterface* pRoom = GetRoomByID(nRoomID) ;
		if ( pRoom == NULL )
		{
			LOGFMTE("can not find room to process id = %d ,from = %d, room id = %d",nMsgType,eSenderPort,nRoomID ) ;
			Json::Value jsmsg ;
			jsmsg["roomID"] = nRoomID ;
			jsmsg["ret"] = 100 ;
			getSvrApp()->sendMsg(nSessionID,jsmsg,nMsgType) ;
			return  false ;
		}

		return pRoom->onMessage(prealMsg,nMsgType,eSenderPort,nSessionID) ;
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
	bool bSuccess = nDatalen > 0 ;
	Json::Value jsResult ;
	if ( bSuccess )
	{
		Json::Reader reader ;
		reader.parse(pResultData,pResultData + nDatalen,jsResult) ;
	}

	//printf("%s\n",pResultData);

	if ( eHttpReq_Token == nUserTypeArg )
	{
 
	}
	else if ( nUserTypeArg == eHttpReq_CreateChatRoom )
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
			LOGFMTD("error code = %d room id = %d",cValue["errcode"].asInt(), cValue["room_id"].asUInt() );

		}

		IRoom* pRoom = (IRoom*)pUserData ;
		if ( bSuccess )
		{
			pRoom->setChatRoomID(nChatRoomID);
		}
		else
		{
			LOGFMTE("request chat room id error ") ;
		}
	}
	//else if ( nUserTypeArg == eHttpReq_ChatRoomList )
	//{
	//	Json::Value jsroomIDs = jsResult["entities"];
	//	if ( jsroomIDs.size() < 10 )
	//	{
	//		LOGFMTD("finish read chat room ids") ;
	//		m_eRequestChatRoomIDs = eOperate_Done ;

	//		// read room info ;
	//	}
	//	else
	//	{
	//		//requestChatRoomIDList();
	//	}
	//}

}

bool IRoomManager::onCrossServerRequest(stMsgCrossServerRequest* pRequest , eMsgPort eSenderPort,Json::Value* vJsValue)
{
	if ( eCrossSvrReq_ApplyLeaveRoom == pRequest->nRequestType )
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
				LOGFMTD("you are not in room but i let you go!") ;
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
			LOGFMTD("can not find room ,  but i let you go!") ;
		}
		return true ;
	}
	return false ;
}

bool IRoomManager::onAsyncRequest(uint16_t nRequestType , const Json::Value& jsReqContent, Json::Value& jsResult )
{
	switch (nRequestType)
	{
	case eAsync_CreateRoom:
		{
			jsResult["ret"] = 0 ;
			LOGFMTD("received create room id = %u",jsReqContent["roomID"].asUInt() ) ;
			IRoomInterface* pRoom = doCreateInitedRoomObject(jsReqContent["roomID"].asUInt(),jsReqContent);
			if ( pRoom == nullptr )
			{
				LOGFMTE("why create room is null") ;
				jsResult["ret"] = 0 ;
				return true ;
			}

			m_vRooms[pRoom->getRoomID()] = pRoom ;

			// save to DB ;
			Json::Value jsreq ;
			char pBuffer[2014] = {0} ;
			Json::StyledWriter jsWrite ;
			std::string str = jsWrite.write(jsReqContent) ;
#ifdef GAME_365
			sprintf_s(pBuffer,sizeof(pBuffer),"insert into rooms (serialNum,roomType,ownerUID,clubID,baseBet,createTime,jsCreateJson,leftTime ) values ('%u','%u','%u','%u','%u',now(),'%s','%u'); ", 
				jsReqContent["serialNum"].asUInt(),getMgrRoomType(),jsReqContent["createUID"].asUInt(),jsReqContent["clubID"].asUInt(),jsReqContent["baseBet"].asUInt(),str.c_str(),jsReqContent["duringMin"].asUInt() );
#else
			sprintf_s(pBuffer, sizeof(pBuffer), "insert into rooms (serialNum,roomType,ownerUID,clubID,baseBet,createTime,jsCreateJson,leftTime ) values ('%u','%u','%u','%u','%u',now(),'%s','%u'); ",
				jsReqContent["serialNum"].asUInt(), getMgrRoomType(), jsReqContent["createUID"].asUInt(), jsReqContent["clubID"].asUInt(), jsReqContent["baseBet"].asUInt(), str.c_str(), jsReqContent["duringMin"].asUInt() * 60);
#endif 
			jsreq["sql"] = pBuffer ;
			getSvrApp()->getAsynReqQueue()->pushAsyncRequest(ID_MSG_PORT_DB,eAsync_DB_Add,jsreq) ;
			return true ;
		}
		break;
	case eAsync_DeleteRoom:
		{
			auto proom = GetRoomByID(jsReqContent["roomID"].asUInt()) ;
			if ( proom == nullptr )
			{
				jsResult["ret"] = 1 ;
				break; 
			}

			if ( proom->isPlaying() )
			{
				jsResult["ret"] = 2 ;
				break; 
			}
			proom->deleteRoom() ;
			jsResult["ret"] = 0 ;
		}
		break ;
	default:
		return false;
	}
	return true ;
}

bool IRoomManager::reqeustChatRoomID(IRoom* pRoom)
{

	return false ;
}

void IRoomManager::onConnectedSvr()
{
	IGlobalModule::onConnectedSvr();
#ifdef _DEBUG
	return;
#endif
	if ( m_mapPrivateRecorder.empty() && m_mapSieralPrivateRecorder.empty() ) 
	{
		stMsgReadGameResult msg ;
		msg.nRoomType = getMgrRoomType() ;
		sendMsg(&msg,sizeof(msg),0) ;
		LOGFMTD("read game result ") ;
	}
	
	m_tReaderRoomSerials.setIsAutoRepeat(false) ;
	m_tReaderRoomSerials.setInterval(3) ;
	m_tReaderRoomSerials.setCallBack([this](CTimer* pt , float f ){ readRoomSerails() ;}) ;
	m_tReaderRoomSerials.start() ;
}

void IRoomManager::readRoomSerails()
{
	Json::Value jsReq ;
	jsReq["roomType"] = getMgrRoomType() ;
	getSvrApp()->getAsynReqQueue()->pushAsyncRequest(ID_MSG_PORT_DATA,eAsync_ReqRoomSerials,jsReq,[this](uint16_t nReqType ,const Json::Value& retContent,Json::Value& jsUserData){
		uint8_t nRet = retContent["ret"].asUInt();
		if ( nRet )
		{
			LOGFMTI("data svr is reading from db , wait a moment") ;
			m_tReaderRoomSerials.reset() ;
			m_tReaderRoomSerials.start() ;
			return  ;
		}

		Json::Value jsSerails = retContent["serials"];
		for ( uint32_t nIdx = 0 ; nIdx < jsSerails.size(); ++nIdx )
		{
			auto js = jsSerails[nIdx];
			readRoomInfo(js["serial"].asUInt(),js["chatRoomID"].asUInt()) ;
		}
	}) ;
}

void IRoomManager::onExit()
{
	IGlobalModule::onExit();
	for ( auto& ref : m_vRooms )
	{
		auto pRoom = ref.second ;
		delete pRoom ;
		pRoom = nullptr ;
	}
	m_vRooms.clear() ;
}

void IRoomManager::readRoomInfo(uint32_t nSeailNum, uint32_t nChatRoomID )
{
	LOGFMTD("read room info serail = %u",nSeailNum);
	Json::Value jsreq ;
	char pBuffer[256] = {0} ;
	sprintf_s(pBuffer,sizeof(pBuffer),"select leftTime , jsCreateJson, roomState from rooms where serialNum = %u ;",nSeailNum );
	jsreq["sql"] = pBuffer ;
	Json::Value jsUserData ;
	jsUserData["serial"] = nSeailNum ;
	jsUserData["chatRoomID"] = nChatRoomID ;
	getSvrApp()->getAsynReqQueue()->pushAsyncRequest(ID_MSG_PORT_DB,eAsync_DB_Select,jsreq,[this](uint16_t nReqType ,const Json::Value& retContent,Json::Value& jsUserData){
		uint8_t nAftR = retContent["afctRow"].asUInt() ;
		if ( nAftR < 1 )
		{
			LOGFMTD("can not read room infor with serail number = %u",jsUserData["serial"].asUInt()) ;
			return ;
		}
		
		Json::Value jsData = retContent["data"] ;
		if (jsData.size() != 1 )
		{
			LOGFMTD(" serial number = %u have cnt = %u results" ,jsUserData["serial"].asUInt(),jsData.size()) ;
			return ;
		}

		Json::Value jsRow = jsData[(uint32_t)0] ;
		uint32_t nLeftTime = jsRow["leftTime"].asUInt();
		uint32_t roomState = jsRow["roomState"].asUInt();
		std::string strJs = jsRow["jsCreateJson"].asString();
		Json::Reader jsReader ;
		Json::Value jsCreateRoomArg ;
		jsReader.parse(strJs,jsCreateRoomArg,false) ;
		jsCreateRoomArg["chatRoomID"] = jsUserData["chatRoomID"];

		LOGFMTD("crate room from db ok ") ;
		// do create room ;
		IRoomInterface* pRoom = doCreateRoomObject(getMgrRoomType(),true); //doCreateInitedRoomObject(jsCreateRoomArg["roomID"].asUInt(),jsCreateRoomArg);
		if ( pRoom == nullptr )
		{
			LOGFMTE("why create room is null, save arg : %s",strJs.c_str()) ;
			return ;
		}
		pRoom->onCreateFromDB(this, jsCreateRoomArg["roomID"].asUInt(), jsCreateRoomArg);
		m_vRooms[pRoom->getRoomID()] = pRoom ;
		pRoom->setLeftTime(nLeftTime);
		pRoom->setRoomState(roomState) ;

	},jsUserData) ;
}

void IRoomManager::onTimeSave()
{
//#ifdef _DEBUG
//	return ;
//#endif
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

	delete pRoom ;
	pRoom = nullptr ;
}

void IRoomManager::deleteRoomChatID( uint32_t nChatID )
{
	if ( nChatID == 0 )
	{
		return  ;
	}
#ifdef _DEBUG
	return ;
#endif // _DEBUG
	// delete chat room id ;
	//Json::Value cValue ;
	//cValue["email"] = "378569952@qq.com" ;
	//cValue["devpwd"] = "bill007" ;
	//cValue["appkey"] = "abffee4b-deea-4e96-ac8d-b9d58f246c3f" ;
	//cValue["room_id"] = nChatID ;
	//Json::StyledWriter sWrite ;
	//std::string str = sWrite.write(cValue);
	//m_pGoTyeAPI.performRequest("DeleteRoom",str.c_str(),str.size(),nullptr,eHttpReq_DeleteChatRoomID );

	//LOGFMTD("delte chat room id = %u",nChatID) ;
}

void IRoomManager::addPrivateRoomRecorder(stPrivateRoomRecorder* pRecorder, bool isSaveDB )
{
	if (pRecorder->nSieralNum != 0)
	{
		auto iter = m_mapSieralPrivateRecorder.find(pRecorder->nSieralNum);
		Assert(iter == m_mapSieralPrivateRecorder.end() , "why have duplicate room recorder ?");
		m_mapSieralPrivateRecorder[pRecorder->nSieralNum] = pRecorder;
	}
	else
	{
		auto iter = m_mapPrivateRecorder.find(pRecorder->nRoomID);
		Assert(iter == m_mapPrivateRecorder.end() , "why have duplicate room recorder ?");
		if (iter != m_mapPrivateRecorder.end())
		{
			LOGFMTE("duplicate game recorder , so delete this one room id = %u",pRecorder->nRoomID);
			delete iter->second;
			iter->second = nullptr;
			m_mapPrivateRecorder.erase(iter);
		}
		m_mapPrivateRecorder[pRecorder->nRoomID] = pRecorder;
	}

	if ( isSaveDB == false )
	{
		return  ;
	}

#ifdef _DEBUG
	return;
#endif //

	// save to db ;
	stMsgSaveGameResult msgResult ;
	msgResult.nConfigID = pRecorder->nConfigID ;
	msgResult.nCreaterUID = pRecorder->nCreaterUID ;
	msgResult.nRoomID = pRecorder->nRoomID ;
	msgResult.nRoomType = getMgrRoomType() ;
	msgResult.tTime = pRecorder->nTime ;
	msgResult.nDuringSeconds = pRecorder->nDuringSeconds ;
	msgResult.nSieralNum = pRecorder->nSieralNum;
	
	Json::StyledWriter jsWrite ;
	std::string strJson = jsWrite.write(pRecorder->playerDetail) ;
	msgResult.nJsLen = strJson.size() ;

	CAutoBuffer refBuffer ( sizeof(msgResult) + msgResult.nJsLen );
	refBuffer.addContent(&msgResult,sizeof(msgResult)) ;
	refBuffer.addContent(strJson.c_str(),msgResult.nJsLen ) ;
	
	sendMsg((stMsg*)refBuffer.getBufferPtr(),refBuffer.getContentSize(),0) ;
}

