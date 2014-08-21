#include "RoomBaseNew.h"
#include "Player.h"
#include "MessageDefine.h"
#include "RoomBaseData.h"
#include "LogManager.h"
#include "GameServerApp.h"
#include <assert.h>
#include "RoomConfig.h"
#include "RoomManager.h"
CRoomBaseNew::CRoomBaseNew()
{
	m_pRoomData = NULL ;
}

CRoomBaseNew::~CRoomBaseNew()
{
	if ( m_pRoomData )
	{
		delete m_pRoomData ;
		m_pRoomData = NULL ;
	}
	m_vAllPeers.clear() ;
}

bool CRoomBaseNew::Init(stBaseRoomConfig* pConfig)
{
	assert(m_pRoomData == NULL&& "error m_pRoomData can not be NULL") ;
	stRoomBaseDataOnly*pBase = GetRoomDataOnly();
	pBase->cGameType = pConfig->nRoomType ;
	pBase->nRoomLevel = pConfig->nRoomLevel ;
	pBase->cMaxPlayingPeers = pConfig->nMaxSeat;
	pBase->nRoomID = ++CRoomManager::s_RoomID ;
	pBase->cMiniCoinNeedToEnter = pConfig->nMinNeedToEnter ;
	pBase->fOperateTime = pConfig->nWaitOperateTime ;

	m_vAllPeers.clear() ;
	SetTimerManager(CGameServerApp::SharedGameServerApp()->GetTimerMgr());
	SetEnableUpdate(true) ;
	
	return true ;
}

void CRoomBaseNew::Enter(CPlayer* pEnter )
{
	MAP_SESSION_PLAYER::iterator iter = m_vAllPeers.find(pEnter->GetSessionID()) ;
	if ( iter != m_vAllPeers.end() )
	{
		CLogMgr::SharedLogMgr()->ErrorLog("player uid = %d ,with session id = %d already in room id = %d , type = %d",pEnter->GetUserUID(),pEnter->GetSessionID(),m_pRoomData->m_pData->nRoomID,m_pRoomData->m_pData->cGameType) ;
		m_vAllPeers.erase(iter) ;
	}
	m_vAllPeers.insert(MAP_SESSION_PLAYER::value_type(pEnter->GetSessionID(),pEnter));
}

void CRoomBaseNew::Leave(CPlayer* pLeaver)
{
	MAP_SESSION_PLAYER::iterator iter = m_vAllPeers.find(pLeaver->GetSessionID()) ;
	if ( iter == m_vAllPeers.end() )
	{
		CLogMgr::SharedLogMgr()->ErrorLog("player not in this room , player uid = %d",pLeaver->GetUserUID()) ;
		return ;
	}
	m_vAllPeers.erase(iter) ;
}

unsigned int CRoomBaseNew::GetRoomID()
{
	return GetRoomDataOnly()->nRoomID ;
}

unsigned char CRoomBaseNew::GetRoomType()
{
	return GetRoomDataOnly()->cGameType ;
}

unsigned char CRoomBaseNew::GetRoomLevel()
{
	return GetRoomDataOnly()->nRoomLevel ;
}

void CRoomBaseNew::Update(float fTimeElpas, unsigned int nTimerID )
{

}

bool CRoomBaseNew::OnMessage(CPlayer*pSender, stMsg* pmsg)
{
	return false ;
}

void CRoomBaseNew::SendMsgBySessionID(stMsg* pMsg , unsigned short nLen,unsigned int nSessionID,bool bToAll )  // if nsessionid = 0 , means send all peers in this room ;
{
	CPlayer* pPlayer = NULL ;
	if ( bToAll == false )
	{
		MAP_SESSION_PLAYER::iterator iter = m_vAllPeers.find(nSessionID) ;
		if ( iter != m_vAllPeers.end() )
		{
			pPlayer = iter->second ;
			pPlayer->SendMsgToClient((char*)pMsg,nLen);
			return ;
		}
		else
		{
			CLogMgr::SharedLogMgr()->ErrorLog("can not send msg = %d to session id = %d",pMsg->usMsgType ,nSessionID) ;
		}
	}
	else
	{
		MAP_SESSION_PLAYER::iterator iter = m_vAllPeers.begin() ;
		for ( ; iter != m_vAllPeers.end() ; ++iter )
		{
			pPlayer = iter->second ;
			if ( pPlayer->GetSessionID() != nSessionID )
			{
				pPlayer->SendMsgToClient((char*)pMsg,nLen);
			}
		}
	}
}

void CRoomBaseNew::SendMsgByRoomIdx(stMsg* pMsg, unsigned short nLen ,unsigned char nIdx)
{
	stPeerBaseData* pdata = m_pRoomData->GetPeerDataByIdx(nIdx);
	if ( pdata )
	{
		SendMsgBySessionID(pMsg,nLen,pdata->nSessionID) ;
	}
	else
	{
		CLogMgr::SharedLogMgr()->ErrorLog("Can not send msg to player with room idx = %d , msg = %d, idx player is NULL",nIdx,pMsg->usMsgType) ;
	}
}

void CRoomBaseNew::SitDown(CPlayer* pPlayer )
{

}

void CRoomBaseNew::StandUp(CPlayer* pPlayer)
{

}

unsigned char CRoomBaseNew::CheckCanJoinThisRoom(CPlayer* pPlayer) // 0 means ok , other value means failed ;
{
	return 0 ;
}

void CRoomBaseNew::SendRoomInfoToPlayer(CPlayer* pPlayer)
{

}

unsigned int CRoomBaseNew::GetAntesCoin()
{
	return GetRoomDataOnly()->cMiniCoinNeedToEnter ;
}

unsigned short CRoomBaseNew::GetEmptySeatCount()
{
	return GetData()->GetEmptySeatCnt();
}

unsigned short CRoomBaseNew::GetMaxSeat()
{
	return GetData()->GetMaxSeat();
}

stRoomBaseDataOnly* CRoomBaseNew::GetRoomDataOnly()
{
	return m_pRoomData->m_pData ;
}

CPlayer* CRoomBaseNew::GetPlayerByRoomIdx(unsigned char pIdx )
{
	stPeerBaseData* p = m_pRoomData->GetPeerDataByIdx(pIdx) ;
	if ( p )
	{
		MAP_SESSION_PLAYER::iterator iter = m_vAllPeers.find(p->nSessionID) ;
		if ( iter != m_vAllPeers.end() )
		{
			return iter->second ;
		}
	}
	return NULL ;
}