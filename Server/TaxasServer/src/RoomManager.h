#pragma once
#include "TaxasMessageDefine.h"
#include <map>
class CTaxasRoom ;
class CRoomManager
{
public:
	typedef std::map<uint32_t, CTaxasRoom*> MAP_ID_ROOM;
public:
	CRoomManager();
	~CRoomManager();
	bool Init();
	bool OnMsg( stMsg* prealMsg , eMsgPort eSenderPort , uint32_t nSessionID );
	CTaxasRoom* GetRoomByID(uint32_t nRoomID );
	void SendMsg(stMsg* pmsg, uint32_t nLen , uint32_t nSessionID );
protected:
	MAP_ID_ROOM m_vRooms ;
};