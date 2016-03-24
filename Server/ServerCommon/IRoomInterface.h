#pragma once
#include "NativeTypes.h"
#include "MessageIdentifer.h"
#include "CardPoker.h"
#include <list>
#include <map>
#include "CommonDefine.h"
#include "ServerDefine.h"
#include <vector>
struct stBaseRoomConfig;
struct stMsgCrossServerRequest ;
struct stMsgCrossServerRequestRet ;
struct stMsg ;
namespace Json
{
	class Value ;
};

class IRoomInterface
{
public:
	struct stRoomRankItem
	{
		uint32_t nUserUID ;
		int32_t nGameOffset ;
		int32_t nOtherOffset ;
		uint16_t nRankIdx ;
		bool bIsDiryt ;
		stRoomRankItem(){ nRankIdx = 0 ; }
	};

	typedef std::list<stRoomRankItem*> LIST_ROOM_RANK_ITEM ;
public:
	virtual ~IRoomInterface(){ }
	virtual void serializationFromDB(stBaseRoomConfig* pConfig,uint32_t nRoomID , Json::Value& vJsValue );
	virtual void willSerializtionToDB(Json::Value& vOutJsValue);
	virtual uint8_t canPlayerEnterRoom( stEnterRoomData* pEnterRoomPlayer );  // return 0 means ok ;
	virtual void onPlayerEnterRoom(stEnterRoomData* pEnterRoomPlayer );
	virtual void roomItemDetailVisitor(Json::Value& vOutJsValue);
	virtual bool onCrossServerRequest(stMsgCrossServerRequest* pRequest , eMsgPort eSenderPort,Json::Value* vJsValue = nullptr);
	virtual bool onCrossServerRequestRet(stMsgCrossServerRequestRet* pResult,Json::Value* vJsValue = nullptr ){ return false ;}

	virtual void onTimeSave( bool isRightNow = false );
	void sortRoomRankItem();
	virtual void onRankChanged();
	virtual bool onMessage( stMsg* prealMsg , eMsgPort eSenderPort , uint32_t nPlayerSessionID );
	void refreshLastGameRank();
protected:
	uint32_t m_nRoomID ;
	stEnterRoomLimitCondition m_stLimitConition ;
	// creator info 
	uint32_t m_nRoomOwnerUID ;
	uint32_t m_nCreateTime ;
	uint32_t m_nDeadTime ;
	uint32_t m_nOpenTime ;
	uint32_t m_nDuringTime ; // by seconds ;
	uint32_t m_nTermNumber ; // dang qian shi di ji qi 

	char m_vRoomName[MAX_LEN_ROOM_NAME] ;
	MAP_UID_ROOM_RANK_ITEM m_vRoomRankHistroy ;
	LIST_ROOM_RANK_ITEM m_vSortedRankItems ;
	LIST_ROOM_RANK_ITEM m_vLastGameRank ;
	bool m_bDirySorted ;
};