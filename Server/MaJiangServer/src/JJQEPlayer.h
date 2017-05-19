#pragma once
#include "MJPlayer.h"
#include "JJQEPlayerCard.h"
class JJQEPlayer
	:public MJPlayer
{
public:
	void onWillStartGame()override;
	IMJPlayerCard* getPlayerCard()override;
	void setIsChaoZhuang( bool isChaoZhuang );
protected:
	bool m_isChaoZhuang;
	JJQEPlayerCard m_tPlayerCard;
};
