#pragma once
#include "MJPlayer.h"
#include "GG23PlayerCard.h"
class GG23Player
	:public MJPlayer
{
public:
	IMJPlayerCard* getPlayerCard()override
	{
		return &m_tPlayerCard;
	}
protected:
	GG23PlayerCard m_tPlayerCard;
};
