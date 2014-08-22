#pragma once
#include "CommonData.h"
#include <vector>
class CRoomBaseData
{
public:
	typedef std::vector<unsigned int> VEC_KICKED_SESSIONID ;  // session ids will be clicked ;
public:
	CRoomBaseData();
	virtual ~CRoomBaseData();
	virtual void Init();

	virtual void OnStartGame() = 0 ;
	virtual void OnEndGame() ;
	void AddPeer(stPeerBaseData* peerData); // must new outside
	void RemovePeer(unsigned int nSessionID );
	unsigned char GetEmptySeatCnt();
	unsigned char GetPlayingSeatCnt();
	unsigned char GetMaxSeat();
	stPeerBaseData* GetPeerDataBySessionID(unsigned int nSessionID );
	stPeerBaseData* GetPeerDataByIdx(unsigned char cIdx );
	stPeerBaseData* GetPeerDataByUserUID(unsigned int nUserUID);
	char GetRoomIdxBySessionID(unsigned int nSessionID );
	unsigned int GetSessionIDByIdx(unsigned char nIdx);
	virtual void SetRoomState(unsigned short nRoomState );
	unsigned char OnPlayerKick(unsigned int nSessionID,unsigned char nTargetIdx,bool* bRightKicked );

	virtual bool CheckCanPlayerBeKickedRightNow(unsigned char nTargetIdx );
	bool DoBeKickedPlayer(unsigned int nSessionID );
protected:
	void ClearAllPeers();
public:
	stRoomBaseDataOnly* m_pData;
	stPeerBaseData** m_vPeerDatas ;
	VEC_KICKED_SESSIONID m_vSessionIDs ;
};

