#include "NiuNiuRoom.h"
#include "NiuNiuRoomPlayer.h"
#include "NiuNiuServer.h"
bool CNiuNiuRoom::init(stBaseRoomConfig* pConfig, uint32_t nRoomID )
{
	ISitableRoom::init(pConfig,nRoomID) ;
	// create room state ;
	// set default room state ;
}

ISitableRoomPlayer* CNiuNiuRoom::doCreateSitableRoomPlayer()
{
	return new CNiuNiuRoomPlayer();
}

void CNiuNiuRoom::sendMsgToPlayer( stMsg* pmsg , uint16_t nLen , uint32_t nSessionID )
{
	CNiuNiuServerApp::getInstance()->sendMsg(nSessionID,(char*)pmsg,nLen);
}


bool CNiuNiuRoom::onMessage( stMsg* prealMsg , eMsgPort eSenderPort , uint32_t nPlayerSessionID )
{
	if ( ISitableRoom::onMessage(prealMsg,eSenderPort,nPlayerSessionID) )
	{
		return true ;
	}
	return false ;
}

void CNiuNiuRoom::sendRoomInfoToPlayer(uint32_t nSessionID)
{

}

void CNiuNiuRoom::onTimeSave()
{
	ISitableRoom::onTimeSave();
}

IRoomPlayer* CNiuNiuRoom::doCreateRoomPlayerObject()
{
	return new IRoomPlayer();
}