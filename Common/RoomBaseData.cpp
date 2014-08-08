#include "RoomBaseData.h"
#include <iostream>
#include "LogManager.h"
CRoomBaseData::CRoomBaseData()
	:m_pData(NULL)
{

}

CRoomBaseData::~CRoomBaseData()
{
	ClearAllPeers();
	if ( m_pData )
	{
		delete m_pData ;
		m_pData = NULL ;
	}
}

void CRoomBaseData::Init()
{
	m_pData = NULL ;
}

void CRoomBaseData::AddPeer(stPeerBaseData* peerData)
{
	if ( GetEmptySeatCnt() <= 0 )
	{
		CLogMgr::SharedLogMgr()->ErrorLog("can not add peer to room , id = %d",peerData->nUserUID);
		return ;
	}

	if ( m_vPeerDatas == 0 && m_pData->cMaxPlayingPeers != 0 )
	{
		m_vPeerDatas = new stPeerBaseData*[m_pData->cMaxPlayingPeers] ;
		memset(m_vPeerDatas,0,sizeof(stPeerBaseData*) * m_pData->cMaxPlayingPeers) ;
	}
	
	for ( unsigned char idx = 0 ; idx < m_pData->cMaxPlayingPeers ; ++idx )
	{
		if ( m_vPeerDatas[idx] == NULL )
		{
			m_vPeerDatas[idx] = peerData ;
			peerData->cRoomIdx = idx ;
			break; 
		}
	}
}

void CRoomBaseData::RemovePeer(unsigned int nSessionID )
{
	for ( unsigned char idx = 0 ; idx < m_pData->cMaxPlayingPeers ; ++idx )
	{
		if ( m_vPeerDatas[idx] && m_vPeerDatas[idx]->nSessionID == nSessionID )
		{
			delete m_vPeerDatas[idx] ;
			m_vPeerDatas[idx] = NULL ;
			break; 
		}
	}
}

stPeerBaseData* CRoomBaseData::GetPeerDataBySessionID(unsigned int nSessionID )
{
	for ( unsigned char idx = 0 ; idx < m_pData->cMaxPlayingPeers ; ++idx )
	{
		if ( m_vPeerDatas[idx] && m_vPeerDatas[idx]->nSessionID == nSessionID )
		{
			return m_vPeerDatas[idx] ;  
		}
	}
	return NULL ;
}

stPeerBaseData* CRoomBaseData::GetPeerDataByIdx(unsigned char cIdx )
{
	if ( cIdx >= m_pData->cMaxPlayingPeers )
		return NULL ;
	return m_vPeerDatas[cIdx] ;
}

stPeerBaseData* CRoomBaseData::GetPeerDataByUserUID(unsigned int nUserUID)
{
	for ( unsigned char idx = 0 ; idx < m_pData->cMaxPlayingPeers ; ++idx )
	{
		if ( m_vPeerDatas[idx] && m_vPeerDatas[idx]->nUserUID == nUserUID )
		{
			return m_vPeerDatas[idx] ;  
		}
	}
	return NULL ;
}

unsigned char CRoomBaseData::GetEmptySeatCnt()
{
	unsigned char nCnt = 0 ;
	for ( unsigned char idx = 0 ; idx < m_pData->cMaxPlayingPeers ; ++idx )
	{
		if ( m_vPeerDatas[idx] == NULL  )
		{
			++nCnt ;  
		}
	}
	return nCnt ;
}


unsigned char CRoomBaseData::GetPlayingSeatCnt()
{
	return m_pData->cMaxPlayingPeers - GetEmptySeatCnt() ;
}

void CRoomBaseData::ClearAllPeers()
{
	for ( unsigned char idx = 0 ; idx < m_pData->cMaxPlayingPeers ; ++idx )
	{
		if ( m_vPeerDatas[idx] )
		{
			delete m_vPeerDatas[idx] ;
			m_vPeerDatas[idx] = NULL ;
		}
	}
	delete m_vPeerDatas ;
}