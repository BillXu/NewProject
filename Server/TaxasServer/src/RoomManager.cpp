#include "RoomManager.h"
#include "TaxasServerApp.h"
#include "LogManager.h"
#include "TaxasRoom.h"
#include "ServerMessageDefine.h"
#include "RoomConfig.h"
#include "AutoBuffer.h"
#include "SystemRoom.h"
#include "PrivateRoom.h"
IRoomInterface* CRoomManager::doCreateInitedRoomObject(uint32_t nRoomID  ,bool isPrivateRoom, uint16_t nRoomConfigID ,eRoomType reqSubRoomType, Json::Value& vJsValue)
{
	stSitableRoomConfig* pConfig = (stSitableRoomConfig*)CTaxasServerApp::SharedGameServerApp()->GetConfigMgr()->GetConfigByConfigID(nRoomConfigID) ;
	if ( pConfig == nullptr )
	{
		return nullptr ;
	}

	IRoomInterface* pR = doCreateRoomObject(reqSubRoomType,isPrivateRoom);
	pR->onFirstBeCreated(this,pConfig,nRoomID,vJsValue) ;
	return pR ;
}

IRoomInterface* CRoomManager::doCreateRoomObject(eRoomType reqSubRoomType ,bool isPrivateRoom)
{
	IRoomInterface* pRoom = nullptr ;
	if ( !isPrivateRoom )
	{
		pRoom = new CSystemRoom<CTaxasRoom>() ;
	}
	else
	{
		pRoom = new CPrivateRoom<CTaxasRoom> ;
	}
	return pRoom ;
}
