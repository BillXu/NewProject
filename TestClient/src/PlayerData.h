#pragma once
#include "NetWorkManager.h"
#include "CommonDefine.h"
#include <string>
#include "RobotConfig.h"
class CPlayerData
	:public CNetMessageDelegate
{
public:
	CPlayerData(){}
	~CPlayerData(){}
	void SetLoginConfig( CRobotConfigFile::stRobotItem* pItem ){ pRobotItem = pItem ;}
	bool OnMessage( Packet* pMsg ) ;
	const char* GetName(){ return strName.c_str() ;}
	const char* GetAccount(){ return pRobotItem->strAccount.c_str(); }
	const char* GetPassword(){ return pRobotItem->strPassword.c_str();}
	eRoomType GetEnterRoomType(){ return eRoom_TexasPoker ;}
	unsigned short GetEnterRoomID(){ return 0 ;}
	void OnWinCoin( int64_t nCoinOffset, bool bDiamoned  );
	int64_t GetCoin( bool bDiamond );
	unsigned int GetSessionID(){ return nSessionID ;}
	char GetWillEnterRoomLevel(){ return 0 ;}
	void TakeIn(uint64_t nCoin , bool bDimoned);
	void BetCoin(bool bDiamoned , int nOffset );
	bool IsMustWin(bool bDiamoned );
public:
	uint64_t nMyCoin ;
	unsigned int nDiamoned ;
	std::string strName ;

	unsigned int nSessionID ;

	unsigned int nTakeInCoin ;
	unsigned int nTakeInDiamoned ;
	unsigned int nUserUID ;
	CRobotConfigFile::stRobotItem* pRobotItem ;
};