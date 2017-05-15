#pragma once
#include "MJPlayer.h"
class JJQEPlayer
	:public MJPlayer
{
public:
	void onWillStartGame()override;
	IMJPlayerCard* getPlayerCard()override;
	void setIsChaoZhuang( bool isChaoZhuang );

};
