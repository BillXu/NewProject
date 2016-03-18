#include "RoomManager.h"
#include "TaxasServerApp.h"
#include "LogManager.h"
#include "TaxasRoom.h"
#include "ServerMessageDefine.h"
#include "RoomConfig.h"
#include "AutoBuffer.h"
#define ROOM_LIST_ITEM_CNT_PER_PAGE 5 
bool CRoomManager::onPublicMsg(stMsg* prealMsg , eMsgPort eSenderPort , uint32_t nSessionID)
{
	if ( ISitableRoomManager::onPublicMsg(prealMsg,eSenderPort,nSessionID) )
	{
		return true ;
	}

	switch ( prealMsg->usMsgType )
	{
	default:
		return false;
	}
	return true ;
}

void CRoomManager::sendMsg(stMsg* pmsg, uint32_t nLen , uint32_t nSessionID )
{
	CTaxasServerApp::SharedGameServerApp()->sendMsg(nSessionID,(char*)pmsg,nLen) ;
}

IRoom* CRoomManager::doCreateInitedRoomObject(uint32_t nRoomID , uint16_t nRoomConfigID ,eRoomType reqSubRoomType, Json::Value& vJsValue)
{
	stSitableRoomConfig* pConfig = (stSitableRoomConfig*)CTaxasServerApp::SharedGameServerApp()->GetConfigMgr()->GetConfigByConfigID(nRoomConfigID) ;
	if ( pConfig == nullptr )
	{
		return nullptr ;
	}

	IRoom* pR = doCreateRoomObject(reqSubRoomType);
	pR->init(pConfig,nRoomID,vJsValue) ;
	return pR ;
}

IRoom* CRoomManager::doCreateRoomObject(eRoomType reqSubRoomType)
{
	return new CTaxasRoom ;
}
