#include "TaxasPlayer.h"
#include "LogManager.h"
void CTaxasPlayer::willStandUp()
{
	ISitableRoomPlayer::willStandUp() ;
}

void CTaxasPlayer::onGameEnd()
{
	if ( getCoinOffsetThisGame() > 0 )
	{
		increaseWinTimes() ;
	}

	ISitableRoomPlayer::onGameEnd();
	setState(eRoomPeer_WaitNextGame) ;
	zeroData();
}

void CTaxasPlayer::onGameBegin()
{
	ISitableRoomPlayer::onGameBegin() ;
	setState(eRoomPeer_CanAct) ;
	zeroData();
}

void CTaxasPlayer::reset(IRoom::stStandPlayer* pPlayer)
{
	ISitableRoomPlayer::reset(pPlayer) ;
	setState(eRoomPeer_WaitNextGame) ;
	zeroData();
}

void CTaxasPlayer::switchPeerCard(ISitableRoomPlayer* pPlayer )
{
	auto pp = (CTaxasPlayer*)pPlayer;
	uint8_t vTemp[TAXAS_PEER_CARD];
	memcpy(vTemp,vPeerCards,sizeof(vTemp));
	memcpy(vPeerCards,pp->vPeerCards,sizeof(vTemp));
	memcpy(pp->vPeerCards,vTemp,sizeof(vTemp));

	CTaxasPokerPeerCard tTemp ;
	tTemp = m_tPeerCard ;
	m_tPeerCard = pp->m_tPeerCard ;
	pp->m_tPeerCard = tTemp ;
}

uint8_t CTaxasPlayer::getPeerCardByIdx(uint8_t nIdx )
{
	if ( nIdx < 2 )
	{
		return vPeerCards[nIdx] ;
	}
	CLogMgr::SharedLogMgr()->ErrorLog("peer idx = %d must < 2" , nIdx ) ;
	return 0 ;
}

void CTaxasPlayer::addPublicCard(uint8_t vPublicCards[TAXAS_PUBLIC_CARD] )
{
	for ( uint8_t nIdx = 0 ; nIdx < TAXAS_PUBLIC_CARD ; ++nIdx )
	{
		m_tPeerCard.AddCardByCompsiteNum(vPublicCards[nIdx]) ;
	}	
}

void CTaxasPlayer::addPeerCard(uint8_t nIdx , uint8_t nCardNum )
{
	if ( nIdx < 2 )
	{
		vPeerCards[nIdx] = nCardNum ;
		m_tPeerCard.AddCardByCompsiteNum(nCardNum) ;
		return ;
	}
	CLogMgr::SharedLogMgr()->ErrorLog("uid = %d add peer card idx error" , getUserUID()) ;
}

bool CTaxasPlayer::betCoin( uint32_t nBetCoin )
{
	if ( nBetCoin <= getCoin() )
	{
		nBetCoinThisRound += nBetCoin ;
		nAllBetCoin += nBetCoin ;
		setCoin(getCoin() - nBetCoin ) ;
		return true ;
	}
	CLogMgr::SharedLogMgr()->ErrorLog("player uid = %d bet coin error , must fix", getUserUID() ) ;
	return false ;
}

eRoomPeerAction CTaxasPlayer::getCurActType()
{
	return eCurAct ;
}

void CTaxasPlayer::zeroData()
{
	eCurAct = eRoomPeerAction_None;
	nBetCoinThisRound = 0 ;
	nAllBetCoin = 0 ;
	nWinCoinThisGame = 0 ;
	m_tPeerCard.Reset() ;
	memset(vPeerCards,0,sizeof(vPeerCards));
}

uint32_t CTaxasPlayer::getWinCoinThisGame()
{
	return nWinCoinThisGame ;
}

void CTaxasPlayer::addWinCoinThisGame(uint32_t nWinCoin )
{
	nWinCoinThisGame += nWinCoin ;
	setCoin(getCoin() + nWinCoin ) ;
}

int32_t CTaxasPlayer::getCoinOffsetThisGame()
{
	return nWinCoinThisGame - nAllBetCoin ;
}