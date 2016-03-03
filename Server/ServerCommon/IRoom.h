#pragma once
#include "NativeTypes.h"
#include "MessageIdentifer.h"
#include "CardPoker.h"
#include <list>
#include <map>
#include "CommonDefine.h"
#include "ServerDefine.h"
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
	struct stStandPlayer
		:public stEnterRoomData
	{
		uint32_t nWinTimes ;
		uint32_t nPlayerTimes ;
		uint64_t nSingleWinMost ;
	};

	struct stRoomRankItem
	{
		uint32_t nUserUID ;
		int64_t nGameOffset ;
		int64_t nOtherOffset ;
		bool bIsDiryt ;
	};
public:
	typedef std::list<stStandPlayer*> LIST_STAND_PLAYER ;
	typedef std::map<uint32_t,stStandPlayer*> MAP_UID_STAND_PLAYER ;
	typedef std::map<uint16_t,IRoomState*>	MAP_ID_ROOM_STATE;
	typedef std::map<uint32_t,stRoomRankItem*> MAP_UID_ROOM_RANK_ITEM ;
	typedef std::list<stRoomRankItem*> LIST_ROOM_RANK_ITEM ;
	typedef MAP_UID_STAND_PLAYER::iterator STAND_PLAYER_ITER ;
public:
	IRoom();
	virtual ~IRoom();
	virtual bool init(stBaseRoomConfig* pConfig, uint32_t nRoomID, Json::Value& vJsValue );
	virtual void serializationFromDB(uint32_t nRoomID , Json::Value& vJsValue );
	virtual void willSerializtionToDB(Json::Value& vOutJsValue);
	virtual void prepareState();
	void serializationToDB(bool bIsNewCreate = false);
	uint32_t getRoomID();
	virtual void update(float fDelta);

	// event function 
	virtual uint8_t canPlayerEnterRoom( stEnterRoomData* pEnterRoomPlayer );  // return 0 means ok ;
	virtual void onPlayerEnterRoom(stEnterRoomData* pEnterRoomPlayer );
	virtual void onPlayerWillLeaveRoom(stStandPlayer* pPlayer );
	virtual bool canStartGame() = 0 ;
	void playerDoLeaveRoom(stStandPlayer* pp );
	uint32_t getOpenTime(){ return m_nOpenTime ; }
	uint32_t getDeadTime(){ return m_nDeadTime ;}
	uint32_t getCloseTime(){ return m_nOpenTime + m_nDuringTime  ;}
	uint32_t getDeskFee(){ return m_nDeskFree ;}
private:
	bool addRoomPlayer(stStandPlayer* pPlayer );
	void removePlayer(stStandPlayer* pPlayer );
public:
	stStandPlayer* getPlayerByUserUID(uint32_t nUserUID );
	stStandPlayer* getPlayerBySessionID(uint32_t nSessionID );
	bool isPlayerInRoom(stStandPlayer* pPlayer );
	bool isPlayerInRoomWithSessionID(uint32_t nSessioID );
	bool isPlayerInRoomWithUserUID(uint32_t nUserUID );
	uint16_t getPlayerCount();
	STAND_PLAYER_ITER beginIterForPlayers();
	STAND_PLAYER_ITER endIterForPlayers();
	void updatePlayerOffset(uint32_t nUserUID , int64_t nOffsetGame, int64_t nOtherOffset = 0 );
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
	void setOwnerUID(uint32_t nCreatorUID );
	uint32_t getOwnerUID();
	void addLiftTime(uint32_t nMinites );
	void setDeadTime(uint32_t nDeadTime);
	void setRoomName(const char* pRoomName);
	const char* getRoomName();
	void setRewardDesc(const char* pRewardDesc );
	std::string getRewardDesc(){ return m_strRewardDesc ;}
	bool isRoomAlive();
	void setProfit(uint64_t nProfit );
	uint64_t getProfit(){ return m_nCurProfit ;}
	uint64_t getTotalProfit(){ return m_nTotalProfit ;}
	void addTotoalProfit(uint64_t nAdd ){ m_nTotalProfit += nAdd ; m_bRoomInfoDiry = true; }
	void setTotalProfit( uint64_t nProfit ){ m_nTotalProfit = nProfit ;}
	void setCreateTime(uint32_t nTime);
	uint32_t getCreateTime();
	void setChatRoomID(uint32_t nChatRoomID );
	uint32_t getChatRoomID(){ return m_nChatRoomID ; }
	uint32_t getConfigID();
	void sendExpireInform();
	void deleteRoom();
	bool isDeleteRoom();
	void removeAllRankItemPlayer();
	virtual void onRoomClosed();
	virtual void onRoomOpened();
	virtual void onRoomWillDoDelete();
protected:
	bool addRoomState(IRoomState* pRoomState );
	bool isOmitNewPlayerHalo(){ return m_isOmitNewPlayerHalo ; }
	void debugRank();
private:
	bool m_bRoomInfoDiry ;
	uint32_t m_nRoomID ;
	LIST_STAND_PLAYER m_vReseverPlayerObjects;

	MAP_UID_STAND_PLAYER m_vInRoomPlayers ;
	IRoomState* m_pCurRoomState ;
	MAP_ID_ROOM_STATE m_vRoomStates ;
	CPoker m_tPoker ;

	bool m_bIsDelte ;
	stEnterRoomLimitCondition m_stLimitConition ;
	// creator info 
	uint32_t m_nRoomOwnerUID ;
	uint32_t m_nCreateTime ;
	uint32_t m_nDeadTime ;
	uint32_t m_nOpenTime ;
	uint32_t m_nDuringTime ; // by seconds ;

	char m_vRoomName[MAX_LEN_ROOM_NAME] ;
	std::string m_strRewardDesc ;
	uint64_t m_nCurProfit;
	uint64_t m_nTotalProfit ;
	uint32_t m_nChatRoomID ;
	uint16_t m_nConfigID ;
	uint32_t m_nDeskFree ;
	bool m_isOmitNewPlayerHalo ;

	MAP_UID_ROOM_RANK_ITEM m_vRoomRankHistroy ;
	LIST_ROOM_RANK_ITEM m_vSortedRankItems ;
	bool m_bDirySorted ;
};