#pragma once
#include "CommonDefine.h"
#include <map>
#include "RoomGolden.h"
#include "RoomPaiJiu.h"
class CGameRooms
{
public:
 	  typedef std::map<int,CRoomBase*> MAP_ROOM;
public:
	CGameRooms();
	~CGameRooms();
	void AddRoom(CRoomBase* pRoom, eRoomLevel eLevel );
	CRoomBase* GetRoomByID(unsigned int nRoomID, eRoomLevel eLevel );
	int GetRoomCount( eRoomLevel eLevel ) ;
	int GetAllRoomCount();
protected:
	friend class CRoomManager ;
	void RemoveEmptyRoom();
	void RemoveEmptyRoom(MAP_ROOM& vRooms, unsigned short nLeftEmpty ) ;
protected:
	MAP_ROOM m_vRooms[eRoomLevel_Max] ;
};

class CPlayer ;
struct stBaseRoomConfig;
class CRoomManager
{
public:
	typedef std::vector<CRoomBase*> VEC_ROOM;
	typedef std::map<unsigned int, VEC_ROOM>  MAP_BLIND_ROOMS;
	struct stSpeedRoom
	{
		MAP_BLIND_ROOMS vSeatRooms[eSeatCount_Max];
		void AddRoom(CRoomBase*pRoom);
	};
public:
	CRoomManager();
	~CRoomManager();
	void Init();
	CRoomBase* GetRoom(char cRoomType , char cRoomLevel, unsigned int nRoomID);
	CRoomBase* CreateRoom( unsigned int nRoomID );
	CRoomBase* CreateRoom(stBaseRoomConfig* pConfig );
	void SendRoomListToPlayer( CPlayer* pTargetPlayer , unsigned char eType, unsigned char cRoomLevel );
	void AddRoomToType(CRoomBase* pRoomBase);
	CRoomBase* GetProperRoomToJoin(unsigned char cSpeed , unsigned char cSeatType, unsigned int nBlindBet , unsigned int nExptedRoomID = 0 );
protected:
	static unsigned int s_RoomID ;
	CGameRooms m_vGames[eRoom_Max] ;
	stSpeedRoom m_vSpeedRooms[eSpeed_Max];
};