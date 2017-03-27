#pragma once
#include "MJPlayer.h"
#include "SZMJPlayerCard.h"
class SZMJPlayer
	:public MJPlayer
{
public:
	IMJPlayerCard* getPlayerCard()override;
	void onWillStartGame();
	void signBuHuaFlag();
	void clearBuHuaFlag();
	bool haveBuHuaFlag() { return m_bHaveBuHuaFlag; }
protected:
	SZMJPlayerCard m_tPlayerCard;
	bool m_bHaveBuHuaFlag;
};