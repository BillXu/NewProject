#include "NJMJPlayer.h"
void NJMJPlayer::init(stEnterRoomData* pData)
{
	MJPlayer::init(pData);
	m_nWaiBaoCoin = 0;
	m_nWaitBaoOffset = 0;
}

void NJMJPlayer::onWillStartGame()
{
	MJPlayer::onWillStartGame();
	clearHuaGangFlag();
	clearBuHuaFlag();
	m_nWaitBaoOffset = 0;
}

IMJPlayerCard* NJMJPlayer::getPlayerCard()
{
	return &m_tPlayerCard;
}

void NJMJPlayer::signHuaGangFlag()
{
	m_bHaveHuaGangFlag = true;
}

void NJMJPlayer::clearHuaGangFlag()
{
	m_bHaveHuaGangFlag = false;
}

void NJMJPlayer::signBuHuaFlag()
{
	m_bHaveBuHuaFlag = true;
}

void NJMJPlayer::clearBuHuaFlag()
{
	m_bHaveBuHuaFlag = false;
}

void NJMJPlayer::addWaiBaoOffset(int32_t nOffsetCoin)
{
	m_nWaiBaoCoin += nOffsetCoin;
	m_nWaitBaoOffset += nOffsetCoin;
}

int32_t NJMJPlayer::getWaiBaoOffset()
{
	return m_nWaitBaoOffset;
}

int32_t NJMJPlayer::getWaiBaoCoin()
{
	return m_nWaiBaoCoin;
}