#pragma once
#include "NetWorkManager.h"
#include "CommonDefine.h"
#include <string>
#include "RobotConfig.h"
#include "MessageDefine.h"
class CPlayerData
	:public CNetMessageDelegate
{
public:
	CPlayerData(){ memset(&stBaseData,0,sizeof(stBaseData)); nSessionID = 0 ; }
	~CPlayerData(){}
	void SetLoginConfig( CRobotConfigFile::stRobotItem* pItem ){ pRobotItem = pItem ;}
	bool OnMessage( Packet* pMsg ) ;
	eRoomType GetEnterRoomType(){ return eRoom_TexasPoker ;}
	unsigned short GetEnterRoomID(){ return 0 ;}
	void OnWinCoin( int64_t nCoinOffset, bool bDiamoned  );
	int64_t GetCoin( bool bDiamond );
	unsigned int GetSessionID(){ return nSessionID ;}
	char GetWillEnterRoomLevel(){ return 0 ;}
	void TakeIn(uint64_t nCoin , bool bDimoned);
	bool IsMustWin(bool bDiamoned );
	const char* GetName(){ return stBaseData.cName ;}
	unsigned int getUserUID(){ return stBaseData.nUserUID ;}
	unsigned int getDstRoomID(){ return pRobotItem->nDstRoomID; }
	unsigned int getDstGameType(){ return pRobotItem->nDstGameType ;}
public:
	unsigned int nSessionID ;
	stCommonBaseData stBaseData ;
	CRobotConfigFile::stRobotItem* pRobotItem ;
};