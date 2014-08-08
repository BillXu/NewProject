#include "GoldenRoomData.h"
#include <iostream>
#define  IS_STATE( SRC,CEK) (((SRC)&(CEK)) == CEK )
void CGoldenRoomData::Init()
{
	tPoker.InitGolden();

	m_pData = new stRoomGoldenDataOnly  ;
	memset(m_pData,0,sizeof(stRoomGoldenDataOnly));
	m_pData->cGameType = eRoom_Gold ;
	
	stRoomGoldenDataOnly* pData = (stRoomGoldenDataOnly*)m_pData ;
	pData->cCurActIdx = -1 ;
}

stRoomGoldenDataOnly* CGoldenRoomData::GetDataOnly()
{
	stRoomGoldenDataOnly* pData = (stRoomGoldenDataOnly*)m_pData ;
	return pData ;
}

unsigned char CGoldenRoomData::GetReadyPeerCnt()
{
	unsigned char nMax = GetDataOnly()->cMaxPlayingPeers ;
	unsigned char nCnt = 0 ;
	for ( unsigned char i = 0 ; i < nMax ; ++i )
	{
		if ( m_vPeerDatas[i] == NULL )
			continue;
		if ( IS_STATE(m_vPeerDatas[i]->nPeerState,eRoomPeer_Golden_Ready) )
		{
			++nCnt ;
		}
	}
	return nCnt ;
}

unsigned char CGoldenRoomData::GetNextActIdx()
{
	if ( GetDataOnly()->cCurActIdx < 0 )
		return GetDataOnly()->cBankerIdx ;

	unsigned char nMax = GetDataOnly()->cMaxPlayingPeers ;
	for ( int i = GetDataOnly()->cCurActIdx + 1 ; i < nMax * 2 ; ++i )
	{
		unsigned char nRealIdx = i%nMax ;
		if ( nRealIdx == GetDataOnly()->cCurActIdx )
		{
			break;
		}

		if ( nRealIdx == GetDataOnly()->cBankerIdx )
		{
			GetDataOnly()->nRound += 1 ;
		}

		stPeerBaseData* pPeer = m_vPeerDatas[nRealIdx];
		if ( pPeer == NULL )
			continue;
		if ( IS_STATE(pPeer->nPeerState,eRoomPeer_Golden_Playing) == false )
			continue; 

		// update cur act idx ;
		GetDataOnly()->cCurActIdx = pPeer->cRoomIdx ;
		return pPeer->cRoomIdx ;
	}
	return -1 ;
}

void CGoldenRoomData::OnStartGame()
{
	tPoker.RestAllPoker() ;
	GetDataOnly()->cCurActIdx = -1 ;
	GetDataOnly()->nAllBetCoin = 0 ;
	GetDataOnly()->nCurMaxBet = GetDataOnly()->nMiniBet ;
	GetDataOnly()->nRound = 0 ;
	GetDataOnly()->fTimeTick = 0 ;
	
	// decide new banker idx ;
	unsigned char nMax = GetDataOnly()->cMaxPlayingPeers ;
	for ( int i = GetDataOnly()->cBankerIdx+ 1 ; i < nMax * 2 ; ++i )
	{
		unsigned char nRealIdx = i%nMax ;

		stPeerBaseData* pPeer = m_vPeerDatas[nRealIdx];
		if ( pPeer == NULL )
			continue;
		if ( IS_STATE(pPeer->nPeerState,eRoomPeer_Golden_Ready) )
			GetDataOnly()->cBankerIdx = i ;
	}

	// update all peer state 
	for ( int i = 0 ; i < nMax ; ++i )
	{
		stPeerBaseData* pPeer = m_vPeerDatas[i];
		if ( pPeer == NULL )
			continue;
		if ( IS_STATE(pPeer->nPeerState,eRoomPeer_Golden_Ready) )
		{
			pPeer->nPeerState = eRoomPeer_Golden_Playing ;
		}
		else
		{
			pPeer->nPeerState = eRoomPeer_Golden_WaitNextPlay ;
		}
	}
}

void CGoldenRoomData::OnEndGame()
{
	GetDataOnly()->cCurActIdx = -1 ;
	GetDataOnly()->nAllBetCoin = 0 ;
	GetDataOnly()->nCurMaxBet = GetDataOnly()->nMiniBet ;
	GetDataOnly()->nRound = 0 ;
	GetDataOnly()->fTimeTick = 0 ;
	// update all peer state 
	unsigned char nMax = GetDataOnly()->cMaxPlayingPeers ;
	for ( int i = 0 ; i < nMax ; ++i )
	{
		stGoldenPeerData* pPeer = (stGoldenPeerData*)m_vPeerDatas[i];
		if ( pPeer == NULL )
			continue;
		pPeer->nPeerState = eRoomPeer_Golden_WaitToReady ;
		pPeer->nBetCoin = 0 ;
	}
}

char CGoldenRoomData::GameOverCheckAndProcess()  // return > 0 means game over ret value is winner idx ;
{
	unsigned char nMax = GetDataOnly()->cMaxPlayingPeers ;
	stPeerBaseData* pPeer = NULL ,*pWinner = NULL;
	for ( int i = 0 ; i < nMax ; ++i )
	{
		 pPeer = m_vPeerDatas[i];
		if ( pPeer == NULL )
			continue;
		if ( IS_STATE(pPeer->nPeerState,eRoomPeer_Golden_Playing) )
		{
			if ( pWinner )
			{
				return -1 ;
			}
			pWinner = pPeer ;
		}
	}

	// winner get the coin 
	pWinner->nCurCoin += GetDataOnly()->nAllBetCoin ;
	return pWinner->cRoomIdx ;
}

void CGoldenRoomData::DistributeCard()
{
	for ( int iC = 0 ; iC < GOLDEN_PEER_CARD ; ++iC )
	{
		unsigned char nMax = GetDataOnly()->cMaxPlayingPeers ;
		for ( int i = 0 ; i < nMax ; ++i )
		{
			stGoldenPeerData* pPeer = (stGoldenPeerData*)m_vPeerDatas[i];
			if ( pPeer == NULL || IS_STATE(pPeer->nPeerState,eRoomPeer_Golden_Playing) == false )
				continue;
			pPeer->vHoldCard[iC] = tPoker.GetCardWithCompositeNum();
		}
	}
}

