#include "GoldenRoomManager.h"
#include "GoldenServer.h"
#include "LogManager.h"
#include "SystemRoom.h"
#include "PrivateRoom.h"
#include "GoldenRoom.h"
IRoomInterface* CGoldenRoomManager::doCreateInitedRoomObject(uint32_t nRoomID,bool isPrivateRoom , uint16_t nRoomConfigID ,eRoomType reqSubRoomType, Json::Value& vJsValue ) 
{
	stSitableRoomConfig* pConfig = (stSitableRoomConfig*)CGoldenServerApp::getInstance()->getRoomConfigMgr()->GetConfigByConfigID(nRoomConfigID) ;
	if ( pConfig == nullptr )
	{
		return nullptr ;
	}

	IRoomInterface* pRoom = doCreateRoomObject(reqSubRoomType,isPrivateRoom) ;
	pRoom->onFirstBeCreated(this,pConfig,nRoomID,vJsValue);
	return pRoom ;
}

IRoomInterface* CGoldenRoomManager::doCreateRoomObject(eRoomType reqSubRoomType,bool isPrivateRoom)
{
	IRoomInterface* pRoom = nullptr ;
	if ( !isPrivateRoom )
	{
		pRoom = new CSystemRoom<CGoldenRoom> ;
	}
	else
	{
		pRoom = new CPrivateRoom<CGoldenRoom> ;
	}
	return pRoom ;
}
