#include "robotControl.h"
bool CRobotControl::init()
{
	m_bHaveDelayActionTask = false ;
	m_fDelayActionTicket = 0 ;
	m_pDelayActionUserData = nullptr ;
	return true ;
}

void CRobotControl::update(float fdeta )
{
	if ( m_bHaveDelayActionTask )
	{
		m_fDelayActionTicket -= fdeta ;
		if ( m_fDelayActionTicket <= 0 )
		{
			doDelayAction(m_pDelayActionUserData);
			m_bHaveDelayActionTask = false ;
			m_fDelayActionTicket = 0 ;
			m_pDelayActionUserData = nullptr ;
		}
	}
}

void CRobotControl::fireDelayAction(float fDelay,void* pUserData )
{
	m_fDelayActionTicket = fDelay ;
	m_pDelayActionUserData = pUserData ;
	m_bHaveDelayActionTask = true ;
}

void CRobotControl::doDelayAction(void* pUserData )
{

}

bool CRobotControl::onMsg(stMsg* pmsg)
{
	return false ;
}