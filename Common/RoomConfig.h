#pragma once
#include "NativeTypes.h"
#include "IConfigFile.h"
#include <list>
#include <map>
#include "CommonDefine.h"
struct stBaseRoomConfig
{
	uint16_t nConfigID ;
	uint8_t nGameType ;
	//unsigned char nRoomLevel ;
	unsigned short nMaxSeat ;
	//unsigned int nMinNeedToEnter ;
	//unsigned char nWaitOperateTime ; // second ;
	//unsigned int nCreateCount ;
	//uint32_t nCreateFee ;
};


struct stTaxasRoomConfig
	:public stBaseRoomConfig
{
	uint32_t nMiniTakeInCoin;
	uint64_t nMaxTakeInCoin ;
	uint64_t nBigBlind ;
	uint32_t nRentFeePerDay;
	uint32_t nDeskFee;
	float fDividFeeRate;  // chou shui bi li
};

struct stGoldenRoomConfig
	:public stBaseRoomConfig
{
	unsigned int nMiniBet ; // di zhu 
	unsigned short nTitleNeedToEnter ;
	unsigned char nChangeCardRound; 
	bool bCanDoublePK ;
#ifndef GAME_SERVER
	unsigned int vCoinLevels[GOLDEN_ROOM_COIN_LEVEL_CNT];
#endif
};

typedef std::vector<stTaxasRoomConfig*> VEC_BLIND_ROOM_CONFIG ;

class CRoomConfigMgr 
	:public IConfigFile
{
public:
	typedef std::list<stBaseRoomConfig*> LIST_ROOM_CONFIG ;
	typedef LIST_ROOM_CONFIG::iterator LIST_ITER ;
public:
	CRoomConfigMgr(){ Clear();}
	~CRoomConfigMgr(){Clear();}
	bool OnPaser(CReaderRow& refReaderRow );
	LIST_ITER GetBeginIter(){ return m_vAllConfig.begin(); }
	LIST_ITER GetEndIter(){ return m_vAllConfig.end();}
	virtual void OnFinishPaseFile();
	int GetConfigCnt();
	stTaxasRoomConfig* GetConfigByConfigID( uint16_t nConfigID );
protected:
	void Clear() ;
protected:
	LIST_ROOM_CONFIG m_vAllConfig ;
};