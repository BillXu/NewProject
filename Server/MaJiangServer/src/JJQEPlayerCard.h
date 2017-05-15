#pragma once
#include "MJPlayerCard.h"
#include "CommonDefine.h"
class JJQEPlayerCard
	:public MJPlayerCard
{
public:
	void reset() override;
	uint8_t getHuaCardToBuHua();
};