#include "SZMJPlayer.h"
IMJPlayerCard* SZMJPlayer::getPlayerCard()
{
	return &m_tPlayerCard;
}

void SZMJPlayer::onWillStartGame()
{
	MJPlayer::onWillStartGame();
	clearBuHuaFlag();
}

void SZMJPlayer::signBuHuaFlag()
{
	m_bHaveBuHuaFlag = true;
}

void SZMJPlayer::clearBuHuaFlag()
{
	m_bHaveBuHuaFlag = false;
}