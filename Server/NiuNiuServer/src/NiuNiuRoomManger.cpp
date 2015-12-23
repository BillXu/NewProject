#include "NiuNiuRoomManger.h"
#include "NiuNiuServer.h"
#include "NiuNiuRoom.h"
#include "NiuNiuMessageDefine.h"
#include "NiuNiuRoomPlayer.h"
#include "LogManager.h"
bool CNiuNiuRoomManager::init()
{
	IRoomManager::init();

	// temp create room ;
	IRoom* pRoom = doCreateInitedRoomObject(1,2);
	assert(pRoom&&"create room must success");
	if ( pRoom == nullptr )
	{
		return true ;
	}
	m_vRooms[pRoom->getRoomID()] = pRoom ;
	// temp code 
	return true ;
}

bool CNiuNiuRoomManager::onPublicMsg(stMsg* prealMsg , eMsgPort eSenderPort , uint32_t nSessionID)
{
	if ( IRoomManager::onPublicMsg(prealMsg,eSenderPort,nSessionID) )
	{
		return true ;
	}

	switch ( prealMsg->usMsgType )
	{
	default:
		break;
	}
	return false ;
}

void CNiuNiuRoomManager::sendMsg(stMsg* pmsg, uint32_t nLen , uint32_t nSessionID )
{
	CNiuNiuServerApp::getInstance()->sendMsg(nSessionID,(char*)pmsg,nLen) ;
}

bool CNiuNiuRoomManager::onCrossServerRequest(stMsgCrossServerRequest* pRequest , eMsgPort eSenderPort,Json::Value* vJsValue )
{
	if ( IRoomManager::onCrossServerRequest(pRequest,eSenderPort,vJsValue) )
	{
		return true ;
	}

	switch ( pRequest->nRequestType )
	{
	case eCrossSvrReq_EnterRoom:
		{
			stMsgCrossServerRequestRet resultBack ;
			FILL_CROSSE_REQUEST_BACK(resultBack,pRequest,eSenderPort);
			resultBack.vArg[1] = eRoom_NiuNiu ;
			resultBack.vArg[2] = pRequest->vArg[1] ;
			IRoom* pRoom = GetRoomByID(pRequest->vArg[1]) ;
			if ( pRoom == nullptr )
			{
				resultBack.nRet = 1 ;
				sendMsg(&resultBack,sizeof(resultBack),0) ;
				CLogMgr::SharedLogMgr()->ErrorLog("can not find room id = %d , for uid = %d to enter",(uint32_t)pRequest->vArg[1],pRequest->nTargetID) ;
				return true ;
			}
			IRoomPlayer* pPlayer = pRoom->getReusePlayerObject();
			pPlayer->setSessionID(pRequest->vArg[0]) ;
			pPlayer->setUserUID(pRequest->nReqOrigID);
			pPlayer->setCoin(pRequest->vArg[2]) ;
			if ( pRoom->addRoomPlayer(pPlayer) == false )
			{
				CLogMgr::SharedLogMgr()->ErrorLog("this peer already in this room uid = %d",pPlayer->getUserUID()) ;
				delete pPlayer ;
				pPlayer = nullptr ;
				resultBack.nRet = 1 ;
				sendMsg(&resultBack,sizeof(resultBack),0) ;
				return true ;
			}
			pRoom->sendRoomInfoToPlayer(pPlayer->getSessionID());
			sendMsg(&resultBack,sizeof(resultBack),0) ;
			CLogMgr::SharedLogMgr()->PrintLog("send niuniu room info to player uid = %d",pPlayer->getUserUID()) ;
		}
		break;
	default:
		//CLogMgr::SharedLogMgr()->PrintLog("un processed request type = %d",pRequest->nRequestType) ;
		return false;
	}
	return true ;
}

bool CNiuNiuRoomManager::onCrossServerRequestRet(stMsgCrossServerRequestRet* pResult,Json::Value* vJsValue )
{
	if ( IRoomManager::onCrossServerRequestRet(pResult,vJsValue) )
	{
		return true ;
	}
	CLogMgr::SharedLogMgr()->PrintLog("un processed requestRet type = %d",pResult->nRequestType) ;
	return false ;
}

void CNiuNiuRoomManager::onConnectedToSvr()
{

}

IRoom* CNiuNiuRoomManager::doCreateInitedRoomObject(uint32_t nRoomID , uint16_t nRoomConfigID ) 
{
	stSitableRoomConfig* pConfig = CNiuNiuServerApp::getInstance()->getRoomConfigMgr()->GetConfigByConfigID(nRoomConfigID) ;
	if ( pConfig == nullptr )
	{
		return nullptr ;
	}

	IRoom* pRoom = new CNiuNiuRoom() ;
	pRoom->init(pConfig,nRoomID);
	return pRoom ;
}

void CNiuNiuRoomManager::onGetChatRoomIDResult(IRoom* pNewRoom, bool bSuccess )
{

}