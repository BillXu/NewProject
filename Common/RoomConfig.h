#pragma once
#include "NativeTypes.h"
#include "IConfigFile.h"
#include <list>
#include <map>
#include "CommonDefine.h"
struct stBaseRoomConfig
{
	unsigned int nRoomID ;
	unsigned char nRoomType ;
	unsigned char nRoomLevel ;
	unsigned short nMaxSeat ;
	unsigned int nMinNeedToEnter ;
	unsigned char nWaitOperateTime ; // second ;
	unsigned int nCreateCount ;
	uint32_t nCreateFee ;
};

struct stPaiJiurRoomConfig
	:public stBaseRoomConfig
{
	uint64_t nBankerNeedCoin ;
};

struct stTaxasRoomConfig
	:public stBaseRoomConfig
{
	uint64_t nMaxTakeInCoin ;
	uint64_t nBigBlind ;
	uint32_t nRentFeeOneMonth;
	uint32_t nDeskFee;
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

struct stSpeedRoomConfigs
{
	VEC_BLIND_ROOM_CONFIG m_vPlayerCountRoom[eSeatCount_Max];  
	void AddConfig(stTaxasRoomConfig* pConfig);
	void Reset();
};

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
	stBaseRoomConfig* GetRoomConfig( unsigned int nRoomID ) ;
	stBaseRoomConfig* GetRoomConfig( char cRoomType , char cRoomLevel ) ;
	LIST_ITER GetBeginIter(){ return m_vAllConfig.begin(); }
	LIST_ITER GetEndIter(){ return m_vAllConfig.end();}
	virtual void OnFinishPaseFile();
	int GetConfigCnt(eSpeed speed , eRoomSeat eSeatCn );
	stTaxasRoomConfig* GetConfig( eSpeed speed , eRoomSeat eSeatCn , unsigned int nIdx );
	stGoldenRoomConfig* GetGoldenConfig(unsigned short cLevel );
protected:
	void Clear() ;
protected:
	LIST_ROOM_CONFIG m_vAllConfig ;
	stSpeedRoomConfigs m_vSpeedRoomConfig[eSpeed_Max];
};