#pragma once
#include "RobotConfig.h"
#include "MessageDefine.h"
class CSitableRoomData ;
class CRobotControl
{
public:
	enum  eMode
	{
		eMode_Undecide,
		eMode_Working,
		eMode_Idle,
		eMode_Max,
	};

	enum  eRobotState
	{
		eRs_StandUp,
		eRs_Leave,
		eRs_StandingUp,
		eRs_SitingDown,
		eRs_SitDown,
		eRs_Max,
	};
public:
	virtual bool init( CRobotConfigFile::stRobotItem* pRobot,CSitableRoomData* pRoomData, uint32_t nUserUID );
	virtual ~CRobotControl(){}
	virtual void update(float fdeta );
	void updateWorkMode(float fDeta );
	void updateIdleMode(float fdeta);
	void fireDelayAction( uint8_t nActType ,float fDelay,void* pUserData );
	virtual void doDelayAction(uint8_t nActType,void* pUserData );
	bool onMsg(stMsg* pmsg);
	virtual void onGameBegin();
	virtual void onGameEnd();
	virtual void onReicvedRoomData();
	virtual void informRobotAction(uint8_t nActType) = 0 ;
	uint8_t getSeatIdx();
	void setSeatidx(uint8_t nIdx );
	virtual void enterWorkMode();
	virtual void enterIdleMode();
	void sendMsg(stMsg* pmsg , uint16_t nLen );
	virtual uint32_t getTakeInCoinWhenSitDown() = 0 ;
	uint32_t getUserUID(){ return m_nUserUID ;}
	CSitableRoomData* getRoomData(){ return m_pRoomData ; }
	virtual void onGameResult(bool bWin );
	void leaveRoom();
protected:
	void standUp();
private:
	void sitDown();
	void updateCheckState( float fdeta );
protected:
	void checkMode( float fdeta );
	uint8_t getTempHalo(){ return m_nTempHalo ;}
private:
	uint32_t m_nUserUID ;
	CSitableRoomData* m_pRoomData ;
	bool m_bHaveDelayActionTask;
	float m_fDelayActionTicket ;
	void* m_pDelayActionUserData ;
	uint8_t m_delayActType ;
	uint8_t m_nIdx ;
	eRobotState m_eState ;

	float m_fCheckStateTicket ;

	eMode m_eMode ;
	CRobotConfigFile::stRobotItem* m_pRobotItem ;
	float m_fCheckModeTicket ;

	// work mode 
	time_t m_nWorkEndTime ;

	// idle mode 
	time_t m_nStartWorkTime ;

	uint8_t m_nTempHalo ;
};