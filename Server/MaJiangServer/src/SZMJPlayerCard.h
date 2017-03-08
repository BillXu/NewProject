#pragma once
#include "MJPlayerCard.h"
#include "json/json.h"
class SZMJPlayerCard
	:public MJPlayerCard
{
public:
	void onBuHua(uint8_t nHuaCard, uint8_t nCard);
	bool getCardInfo(Json::Value& jsPeerCards);
	bool onDoHu(bool isZiMo, uint8_t nCard, std::vector<uint16_t>& vHuTypes, uint16_t& nHuHuaCnt, uint16_t& nHardAndSoftHua);
	uint8_t getSongGangIdx();
};