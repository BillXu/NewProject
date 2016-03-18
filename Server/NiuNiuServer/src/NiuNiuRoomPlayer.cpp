#include "NiuNiuRoomPlayer.h"
#include <string>
#include "LogManager.h"
#include "ServerMessageDefine.h"
void CNiuNiuRoomPlayer::reset(IRoom::stStandPlayer* pPlayer)
{
	ISitableRoomPlayer::reset(pPlayer) ;
	m_nTryBankerTimes = 0 ;
	m_nBetTimes = 0 ;
	memset(m_vCards,0,sizeof(m_vCards)) ;
	m_tPeerCard.reset();
}

void CNiuNiuRoomPlayer::onGameEnd()
{
	ISitableRoomPlayer::onGameEnd();
	m_nTryBankerTimes = 0 ;
	m_nBetTimes = 0 ;
	memset(m_vCards,0,sizeof(m_vCards)) ;
	m_tPeerCard.reset();
	setState(eRoomPeer_WaitNextGame) ;
}

void CNiuNiuRoomPlayer::onGameBegin()
{
	ISitableRoomPlayer::onGameBegin() ;
	m_nTryBankerTimes = 0 ;
	m_nBetTimes = 0 ;
	memset(m_vCards,0,sizeof(m_vCards)) ;
	m_tPeerCard.reset();
	setState(eRoomPeer_CanAct) ;
}

void CNiuNiuRoomPlayer::doSitdown(uint8_t nIdx )
{
	setIdx(nIdx) ;
}

void CNiuNiuRoomPlayer::willStandUp()
{
	ISitableRoomPlayer::willStandUp() ;
}

void CNiuNiuRoomPlayer::onGetCard( uint8_t nIdx , uint8_t nCard )
{
	if ( nIdx >= NIUNIU_HOLD_CARD_COUNT )
	{
		CLogMgr::SharedLogMgr()->ErrorLog("uid = %d ,on get card idx invalid idx = %d",getUserUID(),nIdx);
		return ;
	}
	m_vCards[nIdx] = nCard ;
	getPeerCard()->addCompositCardNum(nCard);
}

uint8_t CNiuNiuRoomPlayer::getCardByIdx(uint8_t nIdx )
{
	if ( nIdx >= NIUNIU_HOLD_CARD_COUNT )
	{
		CLogMgr::SharedLogMgr()->ErrorLog("uid = %d , get card idx invalid idx = %d",getUserUID(),nIdx);
		return 0 ;
	}
	return m_vCards[nIdx] ;
}

uint8_t CNiuNiuRoomPlayer::getTryBankerTimes()
{
	return m_nTryBankerTimes ;
}

void CNiuNiuRoomPlayer::setTryBankerTimes(uint8_t nTimes )
{
	m_nTryBankerTimes = nTimes ;
}

uint8_t CNiuNiuRoomPlayer::getBetTimes()
{
	return m_nBetTimes ;
}

void CNiuNiuRoomPlayer::setBetTimes(uint8_t nTimes)
{
	m_nBetTimes = nTimes ;
}

void CNiuNiuRoomPlayer::switchPeerCard(ISitableRoomPlayer* pPlayer )
{
	CNiuNiuRoomPlayer* pnp = (CNiuNiuRoomPlayer*)pPlayer ;
	uint8_t vCards[NIUNIU_HOLD_CARD_COUNT];
	memset(vCards,0,sizeof(vCards)) ;

	memcpy(vCards,m_vCards,sizeof(vCards));
	memcpy(m_vCards,pnp->m_vCards,sizeof(vCards)) ;
	memcpy(pnp->m_vCards,vCards,sizeof(vCards)) ;

	CNiuNiuPeerCard tM = m_tPeerCard ;
	m_tPeerCard = pnp->m_tPeerCard ;
	pnp->m_tPeerCard = tM ;
}