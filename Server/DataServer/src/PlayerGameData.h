#pragma once
#include "IPlayerComponent.h"
#include <set>
class CPlayerGameData
	:public IPlayerComponent
{
public:
	struct stGameData
		: public stPlayerGameData
	{
		bool bDirty ;
	};
public:
	typedef std::set<uint32_t> SET_ROOM_ID ;
	typedef std::map<uint32_t,stMyOwnRoom> MAP_ID_MYROOW ;
public:
	CPlayerGameData(CPlayer* pPlayer):IPlayerComponent(pPlayer){ m_eType = ePlayerComponent_PlayerGameData ; }
	void Reset()override;
	void Init()override;
	bool OnMessage( stMsg* pMessage , eMsgPort eSenderPort)override ;
	bool onCrossServerRequest(stMsgCrossServerRequest* pRequest , eMsgPort eSenderPort,Json::Value* vJsValue = nullptr)override;
	bool onCrossServerRequestRet(stMsgCrossServerRequestRet* pResult,Json::Value* vJsValue = nullptr )override;
	void OnPlayerDisconnect()override;
	void OnOtherWillLogined()override;
	void TimerSave();
	void OnReactive(uint32_t nSessionID )override{ sendGameDataToClient(); }
	void OnOtherDoLogined() override{sendGameDataToClient();}
	uint32_t getCurRoomID(){ return m_nStateInRoomID ;}
	uint16_t getCurRoomType(){ return m_nStateInRoomType ; }
	void addOwnRoom(eRoomType eType , uint32_t nRoomID , uint16_t nConfigID );
	bool isCreateRoomCntReachLimit(eRoomType eType);
	bool deleteOwnRoom(eRoomType eType , uint32_t nRoomID );
	uint16_t getMyOwnRoomConfig(eRoomType eType ,  uint32_t nRoomID ) ;
	bool isRoomIDMyOwn(eRoomType eType , uint32_t nRoomID);
	bool isNotInAnyRoom(){ return m_nStateInRoomID == 0 && m_nStateInRoomType == eRoom_Max ; }
protected:
	void sendGameDataToClient();
protected:
	uint32_t m_nStateInRoomID ;
	uint8_t m_nStateInRoomType ;

	stGameData m_vData[eRoom_Max] ;
	MAP_ID_MYROOW m_vMyOwnRooms[eRoom_Max];
	SET_ROOM_ID m_vFollowedRooms ;
};