#pragma once
#pragma pack(push)
#pragma pack(1)
#include "CommonDefine.h"
struct stPeerBaseData
{
	unsigned int nSessionID ;
	uint64_t nCurCoin ;
	unsigned int nDiamond;
	unsigned char cRoomIdx ;
	unsigned int nUserUID ;
	char cName[MAX_LEN_CHARACTER_NAME] ;
	unsigned int nUserDefinePhotoID ;      // 0 means do not upload image 
	unsigned short nDefaultPhotoID ;
	unsigned char cVipLevel ; // 0 menas no vip ;
	int nPeerState ; //  eRoomPeerState
};

struct stRoomBaseDataOnly
{
	unsigned int nRoomID ;
	unsigned char cGameType ; // eRoomType ;
	unsigned short nRoomLevel ;
	unsigned char cCurRoomState ; // eeRoomState ;
	unsigned char fOperateTime ;  // by second , wait player act ;
	float fTimeTick ;     // by second 
	unsigned char cMaxPlayingPeers ; // not include  standup peers , in some game ;
	unsigned int cMiniCoinNeedToEnter; 
};

// golden 
struct stGoldenPeerData
	:public stPeerBaseData
{
	uint64_t nBetCoin ;
	unsigned char nShowedCardCnt ;
	unsigned char vShowedCardIdx[GOLDEN_PEER_CARD] ;
	unsigned char vHoldCard[GOLDEN_PEER_CARD];
	unsigned short nChangeCardUsedDiamond ; 
	unsigned char nChangeCardTimes ; 
	unsigned short nPKTimes;     // fan bei ka shi yong
};

struct stRoomGoldenDataOnly
	:public stRoomBaseDataOnly
{
	uint64_t nAllBetCoin ;
	unsigned char cBankerIdx ;
	char cCurActIdx ;
	unsigned char nRound ;   // begin with 0 ;
	unsigned int nMiniBet ; 
	uint64_t nCurMaxBet ; 
	unsigned short nTitleNeedToEnter ;
	unsigned char nChangeCardRound; 
	bool bCanDoublePK ;
};

struct stGoldenHoldPeerCard
{
	unsigned char nIdx ;
	unsigned char vCard[GOLDEN_PEER_CARD];
};

// taxpoker peer 
struct stTaxasPeerData
	:public stPeerBaseData
{
	unsigned char eCurAct ;  // eRoomPeerAction
	uint64_t nAllBetCoin ;
	uint64_t nWinCoinFromPools ; // include the nAllBetCoin
	uint64_t nBetCoinThisRound ;
	unsigned char vHoldCard[TAXAS_PEER_CARD];
};

struct stTaxasRoomDataSimple
	:public stRoomBaseDataOnly
{
	char cBankerIdx ;
	unsigned char cLittleBlindIdx ;
	unsigned char cBigBlindIdx ;
	char cCurWaitPlayerActionIdx ;

	uint64_t  nCurMainBetPool ;
	uint64_t  nMostBetCoinThisRound;

	unsigned char vPublicCardNums[TAXAS_PUBLIC_CARD] ; 
	unsigned char ePrePeerAction ; 
	unsigned char nBetRound ; //valid value , 0,1 , 2 , 3 ,4 

	unsigned int nBigBlindBet ;
	uint64_t nMaxTakeIn ;
	float fCurStateTime;  // cur state will last seconds ;
};

#pragma pack(pop)