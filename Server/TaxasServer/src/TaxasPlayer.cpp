#include "TaxasPlayer.h"
#include "LogManager.h"
void CTaxasPlayer::willStandUp()
{
	ISitableRoomPlayer::willStandUp() ;
}

void CTaxasPlayer::onGameEnd()
{
	ISitableRoomPlayer::onGameEnd();
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


uint8_t CTaxasPlayer::getPeerCardByIdx(uint8_t nIdx )
{
	if ( nIdx < 2 )
	{
		m_tPeerCard.getCardByIdx(nIdx) ;
	}
	CLogMgr::SharedLogMgr()->ErrorLog("peer idx = %d must < 2" , nIdx ) ;
	return 0 ;
}

void CTaxasPlayer::addPublicCard(uint8_t vPublicCards[TAXAS_PUBLIC_CARD] )
{
	for ( uint8_t nIdx = 0 ; nIdx < TAXAS_PUBLIC_CARD ; ++nIdx )
	{
		m_tPeerCard.addCompositCardNum(vPublicCards[nIdx]) ;
	}	
}

void CTaxasPlayer::addPeerCard(uint8_t nIdx , uint8_t nCardNum )
{
	if ( nIdx < 2 )
	{
		m_tPeerCard.addCompositCardNum(nCardNum) ;
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
	m_tPeerCard.reset() ;
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

int32_t CTaxasPlayer::getGameOffset()
{
	return nWinCoinThisGame - nAllBetCoin ;
}