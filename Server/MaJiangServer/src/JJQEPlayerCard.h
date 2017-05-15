#pragma once
#include "MJPlayerCard.h"
#include "CommonDefine.h"
class JJQEPlayerCard
	:public MJPlayerCard
{
public:
	void reset() override;
	uint8_t getHuaCardToBuHua();
	bool getCardInfo(Json::Value& jsPeerCards);
	bool onFlyUp(std::vector<uint8_t>& vFlyUpCard);
	void onBuHua( uint8_t nHuaCard , uint8_t nNewCard );
	void onGetChaoZhuangHua( uint8_t nHua );
	bool onDoHu(bool isZiMo, uint8_t nCard);
};