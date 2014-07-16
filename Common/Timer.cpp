#include "Timer.h"
#include "LogManager.h"
#include <time.h>
CTimerDelegate::CTimerDelegate()
	:m_pUpdateTimer(NULL)
{
	m_pTimerMgr = NULL ;
}

CTimerDelegate::~CTimerDelegate()
{
	if ( m_pUpdateTimer)
		m_pTimerMgr->RemoveTimer(m_pUpdateTimer) ;
}

void CTimerDelegate::Update(float fTimeElpas , unsigned int nTimerID )
{

}

void CTimerDelegate::SetEnableUpdate( bool bEnable )
{
	if ( bEnable )
	{
		if ( m_pUpdateTimer == NULL )
		{
			m_pUpdateTimer = m_pTimerMgr->AddTimer(this,&CTimerDelegate::Update);
		}
		m_pUpdateTimer->Reset();
		m_pUpdateTimer->Start();
	}
	else
	{
		if ( m_pUpdateTimer )
		{
			m_pUpdateTimer->Stop();
			m_pTimerMgr->RemoveTimer(m_pUpdateTimer) ;
			m_pUpdateTimer = NULL ;
		}
	}
}

// timer
unsigned int CTimer::s_TimerCount = 0 ;
CTimer::CTimer(CTimerDelegate* pDeleate,CTimerDelegate::lpTimerSelector pFunc , float fInterval )
{
	m_fDelay = 0 ;
	m_fInterval = fInterval ;
	m_eState = eTimerState_None ;
	m_fDelayKeeper = 0 ;
	m_fIntervalKeeper = 0 ;
	m_nTimerID = ++s_TimerCount ;
	m_pDelegate = pDeleate ;
	m_pTimerFunc = pFunc ;
}

CTimer::~CTimer()
{

}

void CTimer::Reset()
{
	m_eState = eTimerState_None ;
	m_fDelayKeeper = 0 ;
	m_fIntervalKeeper = 0 ;
}

void CTimer::Update(float fTimeElaps)
{
	if ( eTimerState_Pause == m_eState || eTimerState_None == m_eState )
		return ;
	// process delay 
	if ( (m_fDelayKeeper += fTimeElaps) < m_fDelay )
	{
		return ;
	}

	// prcess interval ;
	if ( (m_fIntervalKeeper += fTimeElaps) < m_fInterval )
	{
		return ;
	}

	// invoke funcion ;
	m_fIntervalKeeper -= m_fInterval ;
    (m_pDelegate->*m_pTimerFunc)(fTimeElaps,GetTimerID());
}

// timer manager ;
//CTimerManager* CTimerManager::SharedTimerManager()
//{
//	static CTimerManager g_sTimer ;
//	return &g_sTimer ;
//}

CTimerManager::CTimerManager()
{
	m_vAllTimers.clear() ;
	m_fTimerScale = 1.0 ;
	m_vTimerWillRemove.clear() ;
}

CTimerManager::~CTimerManager()
{
	MAP_TIMERS::iterator iter = m_vAllTimers.begin();
	for ( ; iter != m_vAllTimers.end(); ++iter )
	{
		delete iter->second ;
	}
	m_vAllTimers.clear() ;
}

CTimer* CTimerManager::AddTimer(CTimer* pTimer )
{
	if ( pTimer == NULL )return NULL;
	if ( GetTimer(pTimer->GetTimerID()) != NULL )
	{
		CLogMgr::SharedLogMgr()->ErrorLog("One timer can not be add twice !");
		return NULL;
	}
	if ( pTimer->GetDelegate() )
	{
		pTimer->GetDelegate()->SetTimerManager(this) ;
	}
	m_vAllTimers[pTimer->GetTimerID()] = pTimer ;
	return pTimer ;
}

CTimer* CTimerManager::AddTimer( CTimerDelegate* pDelegate, CTimerDelegate::lpTimerSelector pFunc )
{
	CTimer* pTimer = new CTimer(pDelegate,pFunc,0);
	return AddTimer(pTimer);
}

void CTimerManager::Update()
{
	static clock_t s_clock = clock();
	unsigned int tClock = clock() - s_clock ;
	float fSescond = m_fTimerScale * ( (float)tClock / (float)(CLOCKS_PER_SEC)) ;
	s_clock = clock();
	MAP_TIMERS::iterator iter = m_vAllTimers.begin();
	for ( ; iter != m_vAllTimers.end(); ++iter )
	{
		CTimer* pTimer = iter->second ;
		if ( !pTimer || pTimer->IsRuning() == false )
			continue ;
		pTimer->Update(fSescond) ;
	}
	
	for ( unsigned int i = 0 ; i < m_vTimerWillRemove.size(); ++i )
	{
		MAP_TIMERS::iterator iter = m_vAllTimers.find(m_vTimerWillRemove[i]) ;
		if ( iter != m_vAllTimers.end() )
		{
			delete iter->second ;
			m_vAllTimers.erase(iter) ;
		}
	}
	m_vTimerWillRemove.clear();
}

void CTimerManager::RemoveTimer( unsigned int nTimerID )
{
	CTimer* pTimer = GetTimer(nTimerID) ;
	if ( !pTimer )
		return ;
	pTimer->Stop();
	m_vTimerWillRemove.push_back(nTimerID) ;
}

CTimer* CTimerManager::GetTimer(unsigned int nTimerID )
{
	MAP_TIMERS::iterator iter = m_vAllTimers.find(nTimerID) ;
	if ( iter != m_vAllTimers.end())
		return iter->second ;
	return NULL ;
}