#pragma once
#include "ISitableRoom.h"
class CNiuNiuRoom
	:public ISitableRoom
{
public:
	bool init(stBaseRoomConfig* pConfig, uint32_t nRoomID )override;
	void sendMsgToPlayer( stMsg* pmsg , uint16_t nLen , uint32_t nSessionID )override;
	bool onMessage( stMsg* prealMsg , eMsgPort eSenderPort , uint32_t nPlayerSessionID ) override;
	void sendRoomInfoToPlayer(uint32_t nSessionID)override ;
	void onTimeSave() override;
	void setBankerIdx(uint8_t nIdx ){ m_nBankerIdx = nIdx ;}
	uint8_t getBankerIdx(){ return m_nBankerIdx ;}
	void setBetBottomTimes(uint8_t nTimes ){ m_nBetBottomTimes = nTimes ;}
	uint8_t getBetBottomTimes(){ return m_nBetBottomTimes ;}
protected:
	IRoomPlayer* doCreateRoomPlayerObject() override ;
	ISitableRoomPlayer* doCreateSitableRoomPlayer() override;
protected:
	uint8_t m_nBankerIdx ;
	uint8_t m_nBetBottomTimes ;
};