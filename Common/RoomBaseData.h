#pragma once
#include "CommonData.h"
//#include <map>
class CRoomBaseData
{
//public:
	//typedef std::map<unsigned char,stPeerBaseData*> MAP_PEERS_DATA ;  // room idx vs peer data ;
public:
	CRoomBaseData();
	virtual ~CRoomBaseData();
	virtual void Init();

	void AddPeer(stPeerBaseData* peerData); // must new outside
	void RemovePeer(unsigned int nSessionID );
	unsigned char GetEmptySeatCnt();
	unsigned char GetPlayingSeatCnt();
	stPeerBaseData* GetPeerDataBySessionID(unsigned int nSessionID );
	stPeerBaseData* GetPeerDataByIdx(unsigned char cIdx );
	stPeerBaseData* GetPeerDataByUserUID(unsigned int nUserUID);
protected:
	void ClearAllPeers();
public:
	stRoomBaseDataOnly* m_pData;
	stPeerBaseData** m_vPeerDatas ;
};

