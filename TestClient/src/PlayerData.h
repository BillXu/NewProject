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
	CPlayerData(){ memset(&stBaseData,0,sizeof(stBaseData)); nSessionID = 0 ; m_bIsLackOfCoin = false ; m_nTotalOffset = 0 ; }
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

	unsigned int getMaxCanLose(){ return pRobotItem->nMaxCanLoseCoin ; }
	bool isLackOfCoin(){ return m_bIsLackOfCoin ; }
	void setIsLackOfCoin( bool isLack ){ m_bIsLackOfCoin = isLack ;}
	CRobotConfigFile::stRobotItem* getConfigItem(){ return pRobotItem ;}
	bool isLogined(){ return stBaseData.nUserUID > 0 ;}
	int32_t getTotalGameOffset(){ return m_nTotalOffset ;}
public:
	unsigned int nSessionID ;
	stCommonBaseData stBaseData ;
	int32_t m_nTotalOffset; 
	CRobotConfigFile::stRobotItem* pRobotItem ;
	bool m_bIsLackOfCoin ;
};