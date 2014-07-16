#pragma once
#include <map>
#include <vector>
class CTimer ;
class CTimerManager ;
class CTimerDelegate
{
public:
	typedef void (CTimerDelegate::*lpTimerSelector)(float fTimeElaps,unsigned int nTimerID );
public:
	CTimerDelegate();
	virtual ~CTimerDelegate();
	virtual void Update(float fTimeElpas, unsigned int nTimerID );
	void SetEnableUpdate( bool bEnable );
protected:
	friend class CTimerManager;
protected:
	void SetTimerManager( CTimerManager* pMgr ) { m_pTimerMgr = pMgr ;}
protected:
	CTimer* m_pUpdateTimer ;
	CTimerManager* m_pTimerMgr ;
};
#define cc_selector_timer(func) (CTimerDelegate::lpTimerSelector)&func

//warning : time merasuse by second ;
class CTimer
{
public:
	enum eTimerState
	{
		eTimerState_None,
		eTimerState_Stop,
		eTimerState_Pause,
		eTimerState_Runing,
		//eTimerState_Delaying,
		eTimerState_Max,
	};
public:
	CTimer(CTimerDelegate* pDeleate,CTimerDelegate::lpTimerSelector pFunc , float fInterval );
	~CTimer();
	void Pause(){ m_eState = eTimerState_Pause ;}
	void Resume(){ m_eState = eTimerState_Runing ;}
	void Reset();
	void Update( float fTimeElaps );
	void SetDelayTime( float fTimerDelay ){ m_fDelay = fTimerDelay ;}
	void SetInterval(float fNewInterval ){ m_fInterval = fNewInterval ;}
	void Stop(){ m_eState = eTimerState_Stop ;}
	bool IsStop(){ return m_eState == eTimerState_Stop ;} 
	bool IsRuning(){ return eTimerState_Runing == m_eState ; }
	unsigned int GetTimerID(){ return m_nTimerID ;}
	void Start(){m_eState = eTimerState_Runing ;}
	CTimerDelegate* GetDelegate(){ return m_pDelegate ;}
protected:
	CTimerDelegate* m_pDelegate ;
	CTimerDelegate::lpTimerSelector m_pTimerFunc ;

	float m_fDelay ;
	float m_fInterval ;

	eTimerState m_eState ;
 
	float m_fDelayKeeper ;
	float m_fIntervalKeeper ;

	static unsigned int s_TimerCount ;
	unsigned int m_nTimerID ;
};

class CTimerManager
{
public:
	typedef std::map<unsigned int ,CTimer*> MAP_TIMERS ;
	typedef std::vector<unsigned int> VEC_TIMER_WILL_REMOVE ;
public:
	//static CTimerManager* SharedTimerManager();
	CTimerManager();
	~CTimerManager();
	CTimer* AddTimer(CTimer* pTimer );
	CTimer* AddTimer( CTimerDelegate* pDelegate, CTimerDelegate::lpTimerSelector pFunc );
	void Update();
	void SetTimeScale( float fScale ){ m_fTimerScale = fScale ;}
	float GetTimeScale(){ return m_fTimerScale ;} 
	void RemoveTimer(CTimer* pTimer ){ if ( pTimer)RemoveTimer(pTimer->GetTimerID());}
	void RemoveTimer( unsigned int nTimerID ); 
	CTimer* GetTimer(unsigned int nTimerID );
protected:
	MAP_TIMERS m_vAllTimers ;
	VEC_TIMER_WILL_REMOVE m_vTimerWillRemove;
	float m_fTimerScale ;
};
