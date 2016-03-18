#pragma once
#include "ISitableRoom.h"
#include <json/json.h>
class CNiuNiuRoom
	:public ISitableRoom
{
public:
	CNiuNiuRoom();
	bool init(stBaseRoomConfig* pConfig, uint32_t nRoomID , Json::Value& vJsValue)override;
	void serializationFromDB(stBaseRoomConfig* pConfig,uint32_t nRoomID , Json::Value& vJsValue )override;
	void willSerializtionToDB(Json::Value& vOutJsValue)override;
	void roomItemDetailVisitor(Json::Value& vOutJsValue)override;
	void prepareState()override ;
	void sendMsgToPlayer( stMsg* pmsg , uint16_t nLen , uint32_t nSessionID )override;
	bool onMessage( stMsg* prealMsg , eMsgPort eSenderPort , uint32_t nPlayerSessionID ) override;
	void sendRoomInfoToPlayer(uint32_t nSessionID)override ;
	void onTimeSave(bool bRightNow = false) override;
	void setBankerIdx(uint8_t nIdx ){ m_nBankerIdx = nIdx ;}
	uint8_t getBankerIdx(){ return m_nBankerIdx ;}
	void setBetBottomTimes(uint8_t nTimes ){ m_nBetBottomTimes = nTimes ;}
	uint8_t getBetBottomTimes(){ return m_nBetBottomTimes ;}
	uint8_t getMaxRate();
	uint8_t getDistributeCardCnt();
	uint32_t getBaseBet(); // ji chu di zhu ;
	uint64_t& getBankCoinLimitForBet();
	void setBankCoinLimitForBet( uint64_t nCoin );
	uint8_t getReateByNiNiuType(uint8_t nType , uint8_t nPoint );
	uint64_t getLeastCoinNeedForBeBanker( uint8_t nBankerTimes );
	bool onCrossServerRequest(stMsgCrossServerRequest* pRequest , eMsgPort eSenderPort,Json::Value* vJsValue = nullptr)override ;
	bool onCrossServerRequestRet(stMsgCrossServerRequestRet* pResult,Json::Value* vJsValue = nullptr )override ;
	void onGameWillBegin()override ;
	void onGameDidEnd()override ;
	void onPlayerWillStandUp( ISitableRoomPlayer* pPlayer )override ;
	uint32_t getLeastCoinNeedForCurrentGameRound(ISitableRoomPlayer* pp)override ;
	uint8_t getRoomType()override{ return eRoom_NiuNiu ;}
	void prepareCards()override;
	uint32_t coinNeededToSitDown()override;
	void caculateGameResult();
protected:
	ISitableRoomPlayer* doCreateSitableRoomPlayer() override;
protected:
	uint8_t m_nBankerIdx ;
	uint8_t m_nBetBottomTimes ;
	uint64_t m_nBankerCoinLimitForBet ; // 
	uint32_t m_nBaseBet ;

	Json::Value m_arrPlayers ;
};