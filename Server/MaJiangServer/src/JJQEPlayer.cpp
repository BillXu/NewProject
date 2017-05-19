#include "JJQEPlayer.h"
void JJQEPlayer::onWillStartGame()
{
	MJPlayer::onWillStartGame();
	m_isChaoZhuang = false;
}

IMJPlayerCard* JJQEPlayer::getPlayerCard()
{
	return &m_tPlayerCard;
}

void JJQEPlayer::setIsChaoZhuang(bool isChaoZhuang)
{
	m_isChaoZhuang = isChaoZhuang;
}