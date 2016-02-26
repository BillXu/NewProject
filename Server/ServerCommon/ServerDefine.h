#pragma once
#pragma pack(push)
#pragma pack(1)
#include "ServerCommon.h"

struct stEnterRoomLimitCondition
{
	bool isRegisted ;
	uint32_t nCoinLowLimit ;
	uint64_t nCoinUpLimit ;
};

struct stEnterRoomData
{
	uint32_t nUserUID ;
	uint32_t nUserSessionID ;
	uint8_t nNewPlayerHaloWeight ; // xin shou  guang huan quan zhong 
	bool isRegisted ;
	uint64_t nCoin ;
};




#pragma pack(pop)//