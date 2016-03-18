#pragma once
#include "MessageDefine.h"
class CRobotControl ;
class IScene ;
struct stSitableRoomPlayer
{
	uint8_t nIdx;
	uint32_t nUserUID ;
	uint32_t nCoin ;
	uint32_t nStateFlag ;
	virtual void reset(){ nIdx = 0 ; nUserUID = 0 ; nCoin = 0 ; nStateFlag = 0 ;}
	bool isValid(){ return nUserUID > 0 ;}
	bool isHaveState(uint32_t ns ){ return ((nStateFlag & ns) == ns );}
	virtual void onGameBegin(){ nStateFlag = eRoomPeer_CanAct ;}
	virtual void onGameEnd(){ nStateFlag = eRoomPeer_WaitNextGame ; }
};

class CSitableRoomData
{
public:
	CSitableRoomData();
	virtual ~CSitableRoomData();
	void init(IScene* pScene );
	void setBaseInfo(uint32_t nRoomID, uint8_t nSeatCnt, uint32_t nDeskFee, uint8_t nRoomState );
	virtual bool onMsg(stMsg* pmsg );
	virtual void onGameBegin();
	virtual void onGameEnd();
	uint8_t getPlayerCntWithState( uint32_t nState );
	bool isRoomActive(){ return m_isActive ;}
	virtual stSitableRoomPlayer* doCreateSitDownPlayer() = 0 ;
	stSitableRoomPlayer* getPlayerByIdx( uint8_t nIdx );
	uint8_t getRoomState(){ return m_nRoomState ;}
	uint8_t getPlayerCnt(){ return getPlayerCntWithState(eRoomPeer_SitDown) ;}
	int8_t getRandEmptySeatIdx();
	uint8_t getSeatCount(){ return m_nSeatCount ;}
	virtual uint8_t getRoomType() = 0 ;
	uint32_t getRoomID(){ return m_nRoomID ;}
	void sendMsg(stMsg* pmsg , uint16_t nLen );
	virtual uint8_t getTargetSvrPort() = 0 ;
	CRobotControl* getRobotControl(){ return m_pRobot ;}
	virtual CRobotControl* doCreateRobotControl() = 0 ;
protected:
	CRobotControl* m_pRobot ;
	IScene* m_pScene ;
	bool m_isActive;
	uint8_t m_nRoomID ;
	uint8_t m_nSeatCount ;
	uint32_t m_nDeskFee ;
	uint8_t m_nRoomState ;
	stSitableRoomPlayer* m_vSitDownPlayer[10] ;
};