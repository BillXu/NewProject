#pragma once
#include "NativeTypes.h"
#include "MessageIdentifer.h"
#include "CardPoker.h"
#include <list>
#include <map>
#include "CommonDefine.h"
class IRoomState ;
class IRoomPlayer ;
struct stMsg ;
struct stBaseRoomConfig ;
struct stMsgCrossServerRequest ;
struct stMsgCrossServerRequestRet ;
namespace Json
{
	class Value ;
};

class IRoom
{
public:
	struct stRoomRankItem
	{
		uint32_t nUserUID ;
		int64_t nOffset ;
		uint32_t nPlayerTimes ;
		uint32_t nWinTimes ;
		bool bIsDiryt ;
	};
public:
	typedef std::list<IRoomPlayer*> LIST_ROOM_PLAYER ;
	typedef std::map<uint32_t,IRoomPlayer*> MAP_UID_ROOM_PLAYER ;
	typedef std::map<uint16_t,IRoomState*>	MAP_ID_ROOM_STATE;
	typedef std::map<uint32_t,stRoomRankItem*> MAP_UID_ROOM_RANK_ITEM ;
	typedef std::list<stRoomRankItem*> LIST_ROOM_RANK_ITEM ;
	typedef MAP_UID_ROOM_PLAYER::iterator PLAYER_ITER ;
public:
	IRoom();
	virtual ~IRoom();
	virtual bool init(stBaseRoomConfig* pConfig, uint32_t nRoomID );
	uint32_t getRoomID();
	virtual void update(float fDelta);

	bool addRoomPlayer(IRoomPlayer* pPlayer );
	void removePlayer(IRoomPlayer* pPlayer );
	IRoomPlayer* getPlayerByUserUID(uint32_t nUserUID );
	IRoomPlayer* getPlayerBySessionID(uint32_t nSessionID );
	bool isPlayerInRoom(IRoomPlayer* pPlayer );
	bool isPlayerInRoomWithSessionID(uint32_t nSessioID );
	bool isPlayerInRoomWithUserUID(uint32_t nUserUID );
	uint16_t getPlayerCount();
	IRoomPlayer* getReusePlayerObject();
	virtual IRoomPlayer* doCreateRoomPlayerObject() = 0 ;
	PLAYER_ITER beginIterForPlayers();
	PLAYER_ITER endIterForPlayers();
	void updatePlayerOffset(uint32_t nUserUID , int64_t nOffsetThisOnce );
	void sortRoomRankItem();
	LIST_ROOM_RANK_ITEM::iterator getSortRankItemListBegin(){ return m_vSortedRankItems.begin() ;}
	LIST_ROOM_RANK_ITEM::iterator getSortRankItemListEnd(){ return m_vSortedRankItems.end() ; }

	void sendRoomMsg( stMsg* pmsg , uint16_t nLen );
	virtual void sendMsgToPlayer( stMsg* pmsg , uint16_t nLen , uint32_t nSessionID ) = 0 ;
	virtual bool onMessage( stMsg* prealMsg , eMsgPort eSenderPort , uint32_t nPlayerSessionID );
	virtual void sendRoomInfoToPlayer(uint32_t nSessionID) = 0 ;
	virtual uint8_t getRoomType() = 0 ;
	virtual void onGameWillBegin(){}
	virtual void onGameDidEnd(){}
	virtual bool onCrossServerRequest(stMsgCrossServerRequest* pRequest , eMsgPort eSenderPort,Json::Value* vJsValue = nullptr);
	virtual bool onCrossServerRequestRet(stMsgCrossServerRequestRet* pResult,Json::Value* vJsValue = nullptr ){ return false ;}

	virtual void onTimeSave( bool isRightNow = false );
	void goToState(IRoomState* pTargetState );
	void goToState( uint16_t nStateID );
	void setInitState(IRoomState* pDefaultState );
	IRoomState* getCurRoomState();
	IRoomState* getRoomStateByID(uint16_t nStateID );
	CPoker* getPoker(){ return &m_tPoker ; }

	// room life and attribute
	void onCreateByPlayer(uint32_t nUserUID, uint16_t nRentDays );
	void setOwnerUID(uint32_t nCreatorUID );
	uint32_t getOwnerUID();
	void addLiftTime(uint32_t nDays );
	void setDeadTime(uint32_t nDeadTime);
	void setAvataID(uint32_t nAvaID );
	void setRoomName(const char* pRoomName);
	const char* getRoomName();
	void setRoomInform(const char* pRoomInform );
	std::string getRoomInform(){ return m_strRoomInForm ;}
	bool isRoomAlive();
	void setProfit(uint64_t nProfit );
	uint64_t getProfit(){ return m_nRoomProfit ;}
	uint64_t getTotalProfit(){ return m_nTotalProfit ;}
	void addTotoalProfit(uint64_t nAdd ){ m_nTotalProfit += nAdd ;}
	void setTotalProfit( uint64_t nProfit ){ m_nTotalProfit = nProfit ;}
	void setCreateTime(uint32_t nTime);
	uint32_t getCreateTime();
	void setInformSieral(uint32_t nSieaial);
	void setChatRoomID(uint32_t nChatRoomID );
	uint32_t getChatRoomID(){ return m_nChatRoomID ; }
	uint32_t getConfigID();
	uint32_t getDeadTime();
	void sendExpireInform();
	void finishReadInfoInitRoom(){ m_bRoomInfoDiry = false ; }
	void deleteRoom();
	bool isDeleteRoom();
	void removeAllRankItemPlayer();
	virtual void onMatchFinish(){}
	virtual void onMatchRestart(){};
protected:
	bool addRoomState(IRoomState* pRoomState );
private:
	uint32_t m_nRoomID ;
	LIST_ROOM_PLAYER m_vReseverPlayerObjects;
	MAP_UID_ROOM_PLAYER m_vInRoomPlayers ;
	IRoomState* m_pCurRoomState ;
	MAP_ID_ROOM_STATE m_vRoomStates ;
	CPoker m_tPoker ;

	bool m_bIsDelte ;
	// creator info 
	bool m_bRoomInfoDiry ;
	uint32_t m_nRoomOwnerUID ;
	uint32_t m_nCreateTime ;
	uint32_t m_nDeadTime ;
	uint16_t m_nAvataID ;
	char m_vRoomName[MAX_LEN_ROOM_NAME] ;
	std::string m_strRoomInForm ;
	uint32_t m_nInformSerial;
	uint64_t m_nRoomProfit;
	uint64_t m_nTotalProfit ;
	uint32_t m_nChatRoomID ;
	uint16_t m_nConfigID ;

	MAP_UID_ROOM_RANK_ITEM m_vRoomRankHistroy ;
	LIST_ROOM_RANK_ITEM m_vSortedRankItems ;
	bool m_bDirySorted ;
};