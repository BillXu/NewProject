#include "GoldenRoomPlayer.h"
#include <cassert>
void CGoldenRoomPlayer::reset(IRoom::stStandPlayer* pPlayer)
{
	ISitableRoomPlayer::reset(pPlayer);
	m_nBetCoin = 0 ;
	memset(m_vCards,0,sizeof(m_vCards));
	m_tPeerCard.Reset();
	setState(eRoomPeer_WaitNextGame) ;
}

void CGoldenRoomPlayer::onGameEnd()
{
	ISitableRoomPlayer::onGameEnd() ;
	setState(eRoomPeer_WaitNextGame) ;
}

void CGoldenRoomPlayer::onGameBegin()
{
	ISitableRoomPlayer::onGameBegin() ;
	setState(eRoomPeer_CanAct) ;
	m_nBetCoin = 0 ;
	memset(m_vCards,0,sizeof(m_vCards));
	m_tPeerCard.Reset();
}

void CGoldenRoomPlayer::onGetCard( uint8_t nIdx , uint8_t nCard )
{
	assert(nIdx < GOLDEN_PEER_CARD && "invalid idx" );
	m_vCards[nIdx] = nCard ;
	
	for ( auto p : m_vCards )
	{
		if ( p == 0 )
		{
			return ;
		}
	}

	m_tPeerCard.SetPeerCardByNumber(m_vCards[0],m_vCards[1],m_vCards[2]) ;
}

uint8_t CGoldenRoomPlayer::getCardByIdx(uint8_t nIdx )
{
	assert(nIdx < GOLDEN_PEER_CARD && "invalid idx" );
	return m_vCards[nIdx] ;
}

void CGoldenRoomPlayer::switchPeerCard(ISitableRoomPlayer* pPlayer )
{
	CGoldenRoomPlayer* pp = (CGoldenRoomPlayer*)pPlayer ;
	uint8_t vCards[GOLDEN_PEER_CARD] = { 0 } ;
	memcpy(vCards,m_vCards,sizeof(vCards));
	memcpy(m_vCards,pp->m_vCards,sizeof(vCards));
	memcpy(pp->m_vCards,vCards,sizeof(vCards));
	m_tPeerCard.SetPeerCardByNumber(m_vCards[0],m_vCards[1],m_vCards[2]) ;
	pp->m_tPeerCard.SetPeerCardByNumber(pp->m_vCards[0],pp->m_vCards[1],pp->m_vCards[2]) ;
}

bool CGoldenRoomPlayer::betCoin(uint32_t nBetCoin )
{
	if ( getCoin() < nBetCoin )
	{
		return false ;
	}

	m_nBetCoin += nBetCoin ;
	setCoin(getCoin() - nBetCoin ) ;
	return true ;
}