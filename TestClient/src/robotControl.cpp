#include "robotControl.h"
#include <ctime>
#include "SitableRoomData.h"
//#define  TIME_CHECK_MODE 2*60
#define  TIME_CHECK_MODE 5
#define  TIME_WORK_TIME_HOUR 4
bool CRobotControl::init( CRobotConfigFile::stRobotItem* pRobot,CSitableRoomData* pRoomData, uint32_t nUserUID )
{
	m_bHaveDelayActionTask = false ;
	m_fDelayActionTicket = 0 ;
	m_pDelayActionUserData = nullptr ;
	m_nIdx = 10 ;
	m_eState = eRs_StandUp ;
	m_eMode = eMode_Undecide ;
	m_fCheckModeTicket = TIME_CHECK_MODE ;
	m_nWorkEndTime = 0 ;
	m_nStartWorkTime = 0 ;
	m_fCheckStateTicket = 0 ;
	m_pRobotItem = pRobot ;
	m_pRoomData = pRoomData ;
	m_nUserUID = nUserUID ;
	printf("robot init uid = %d\n",nUserUID);
	return true ;
}

void CRobotControl::update(float fdeta )
{
	if ( m_eMode == eMode_Idle )
	{
		updateIdleMode(fdeta) ;
	}
	else if ( eMode_Working == m_eMode )
	{
		updateWorkMode(fdeta) ;
	}

	checkMode(fdeta) ;

	if ( m_bHaveDelayActionTask )
	{
		m_fDelayActionTicket -= fdeta ;
		if ( m_fDelayActionTicket <= 0 )
		{
			if ( (m_eState == eRs_SitDown || eRs_StandingUp == m_eState ) )
			{
				doDelayAction(m_delayActType,m_pDelayActionUserData);
			}
			else
			{
				printf("uid = %d , not sit down , skip the action \n", getUserUID() ) ;
			}
			
			m_bHaveDelayActionTask = false ;
			m_fDelayActionTicket = 0 ;
			m_pDelayActionUserData = nullptr ;
		}
	}
}

void CRobotControl::checkMode( float fdeta )
{
	if ( m_eMode != eMode_Idle && eMode_Working != m_eMode )
	{
		return ;
	}

	m_fCheckModeTicket -= fdeta ;
	bool isDoCheck = m_fCheckModeTicket < 0 ;
	if ( !isDoCheck )
	{
		return ;
	}
	m_fCheckModeTicket = TIME_CHECK_MODE ;

	time_t tNow = time(nullptr) ;
	if ( eMode_Idle == m_eMode )
	{
		if ( m_nStartWorkTime == 0 )
		{
			struct tm* pTmNow = localtime(&tNow);
			bool bFind = false ;
			for ( auto& t : m_pRobotItem->vWorkPoints )
			{
				if ( pTmNow->tm_hour < t.nHour + TIME_WORK_TIME_HOUR )
				{
					pTmNow->tm_hour = t.nHour ;
					pTmNow->tm_min = t.nMini ;
					bFind = true ;
					break;
				}

				if ( pTmNow->tm_hour == t.nHour + TIME_WORK_TIME_HOUR )
				{
					if ( pTmNow->tm_min < t.nMini )
					{
						pTmNow->tm_hour = t.nHour ;
						pTmNow->tm_min = t.nMini ;
						bFind = true ;
						break;
					}
				}
			}

			if ( !bFind ) // another day ;
			{
				time_t tTommorow = tNow + 60*60*24 ;
				pTmNow = localtime(&tTommorow);
				auto tp = m_pRobotItem->vWorkPoints.front();
				pTmNow->tm_hour = tp.nHour ;
				pTmNow->tm_min = tp.nMini ;
			}
			
			m_nStartWorkTime = mktime(pTmNow);
			printf("next starwork time : %s",ctime(&m_nStartWorkTime)) ;
		}

		if ( tNow >= m_nStartWorkTime )
		{
			m_eMode = eMode_Working ;
			m_nWorkEndTime = m_nStartWorkTime + TIME_WORK_TIME_HOUR * 60 * 60 ;
			m_nStartWorkTime = 0 ;
			printf("starwork end time : %s",ctime(&m_nWorkEndTime)) ;
			enterWorkMode();
		}
	}
	else if ( eMode_Working == m_eMode )
	{
		if ( tNow >= m_nWorkEndTime )
		{
			m_eMode = eMode_Idle ;
			m_nStartWorkTime = 0 ;
			printf("work end time \n") ;
			enterIdleMode() ;
		}
	}
}

void CRobotControl::updateWorkMode(float fdeta )
{
	updateCheckState(fdeta);
}

void CRobotControl::updateCheckState( float fdeta )
{
	m_fCheckStateTicket -= fdeta ;
	if ( m_fCheckStateTicket > 0 )
	{
		return ;
	}

	float fRate = (float)rand() / float(RAND_MAX);  
	m_fCheckStateTicket = 3 + fRate * 10 ;

	switch (m_eState)
	{
	case CRobotControl::eRs_SitDown:
		{
			if ( m_pRoomData->getRoomState() == eRoomState_Close )
			{
				standUp() ;
				return ;
			}

			if ( m_pRoomData->getPlayerCnt() > m_pRobotItem->nApplyLeaveWhenPeerCount )
			{
				float fRate = (float)rand() / float(RAND_MAX);  
				if ( fRate <= 0.3 )
				{
					printf("uid = %d ,too many player i try to stand up \n", getUserUID());
					standUp() ;
				}
			}
		}
		break;
	case CRobotControl::eRs_Leave:
		break;
	case CRobotControl::eRs_StandingUp:
		break;
	case CRobotControl::eRs_SitingDown:
		break;
	case CRobotControl::eRs_StandUp:                                             
		{
			if ( m_pRoomData->getRoomState() == eRoomState_Close )
			{
				printf("uid = %u keep standup room is closed\n",getUserUID());
				return ;
			}

			if ( m_pRoomData->getPlayerCnt() < m_pRobotItem->nApplyLeaveWhenPeerCount )
			{
				printf("uid = %d ,too few player i try to sit down \n", getUserUID());
				sitDown();
			}
		}
		break;
	case CRobotControl::eRs_Max:
		break;
	default:
		break;
	}
}

void CRobotControl::updateIdleMode(float fdeta)
{
	if ( m_eState == eRs_SitDown )
	{
		printf("uid = %d ,during idle mode i try to stand up \n", getUserUID());
		standUp() ;
	}
}

void CRobotControl::onGameBegin()
{

}

void CRobotControl::onGameEnd()
{

}

void CRobotControl::onReicvedRoomData()
{
	enterIdleMode();
}

uint8_t CRobotControl::getSeatIdx()
{
	return m_nIdx ;
}

void CRobotControl::setSeatidx(uint8_t nIdx )
{
	m_nIdx = nIdx ;
}

void CRobotControl::enterWorkMode()
{
	m_eMode = eMode_Working ;
	printf("enter work mode uid = %d\n",getUserUID());
}

void CRobotControl::enterIdleMode()
{
	m_eMode = eMode_Idle ;
	m_nStartWorkTime = 0 ;
	if ( m_eState == eRs_SitDown )
	{
		printf("uid = %d ,enter idle mode i try to stand up \n", getUserUID());
		standUp();
	}
	printf("enter idle mode uid = %d\n",getUserUID());
}

void CRobotControl::standUp()
{
	stMsgPlayerStandUp msgStandUp ;
	msgStandUp.cSysIdentifer = m_pRoomData->getTargetSvrPort();
	msgStandUp.nRoomID = m_pRoomData->getRoomID() ;
	sendMsg(&msgStandUp,sizeof(msgStandUp));
	m_eState = eRs_StandingUp ;
}

void CRobotControl::sitDown()
{
	stMsgPlayerSitDown msg ;
	msg.nIdx = m_pRoomData->getRandEmptySeatIdx();
	msg.nRoomID = m_pRoomData->getRoomID() ;
	msg.nTakeInCoin = getTakeInCoinWhenSitDown() ;
	msg.cSysIdentifer = m_pRoomData->getTargetSvrPort() ;
	sendMsg(&msg,sizeof(msg));
	m_eState = eRs_SitingDown ;
}

void CRobotControl::sendMsg(stMsg* pmsg , uint16_t nLen )
{
	m_pRoomData->sendMsg(pmsg,nLen) ;
}

void CRobotControl::fireDelayAction(uint8_t nActType ,float fDelay,void* pUserData )
{
	m_delayActType = nActType ;
	m_fDelayActionTicket = fDelay ;
	if ( m_fDelayActionTicket <= 0.00001f )
	{
		m_fDelayActionTicket = m_pRobotItem->fActDelayBegin + (rand() % int(m_pRobotItem->fActDelayEnd - m_pRobotItem->fActDelayBegin + 1) ) ;
	}

	m_pDelayActionUserData = pUserData ;
	m_bHaveDelayActionTask = true ;
}

void CRobotControl::doDelayAction(uint8_t nActType ,void* pUserData )
{

}

bool CRobotControl::onMsg(stMsg* pmsg)
{
	switch (pmsg->usMsgType)
	{
	case MSG_ROOM_SITDOWN:
		{
			stMsgRoomSitDown* p = (stMsgRoomSitDown*)pmsg ;
			if ( p->nSitDownPlayerUserUID == getUserUID() )
			{
				setSeatidx(p->nIdx) ;
				m_eState = eRs_SitDown ;
				m_fCheckStateTicket = 0 ;
				printf("uid = %d , sit down ok \n", getUserUID());
			}
		}
		break;
	case MSG_PLAYER_SITDOWN:
		{
			stMsgPlayerSitDownRet* pRet = (stMsgPlayerSitDownRet*)pmsg ;
			if ( pRet->nRet == 1 || 3 == pRet->nRet )
			{
				// must leave , coin not enough ;
				printf("coin error , must leave room to take coin \n") ;
			}
			else if ( 4 == pRet->nRet )
			{
				printf("uid = %u alreayd sit down ? , i should leave room",getUserUID());
			}
			else
			{
				m_eState = eRs_StandUp ;
				m_fCheckStateTicket = 0 ;
			}
		}
		break;
	case MSG_PLAYER_STANDUP:
		{
			stMsgPlayerStandUpRet* pRet = (stMsgPlayerStandUpRet*)pmsg ;
			if ( pRet->nRet )
			{
				m_eState = eRs_StandUp ;
				setSeatidx(10);
			}
		}
		break;
	case MSG_ROOM_STANDUP:
		{
			stMsgRoomStandUp* p = (stMsgRoomStandUp*)pmsg ;
			if ( p->nIdx == getSeatIdx() )
			{
				 m_eState = eRs_StandUp ;
				 m_fCheckStateTicket = 0 ;
				 setSeatidx(10);
				 printf("uid = %d , stand up  ok \n", getUserUID());
			}
		}
		break;
	default:
		break;
	}
	return false ;
}
