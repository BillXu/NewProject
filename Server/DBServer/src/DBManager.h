#pragma once
#include "RakNetTypes.h"
#include "ServerMessageDefine.h"
#include <list>
#include <map>
#include <assert.h>
struct stDBResult ;
class CDBServerApp ;
struct stPlayerDetailData;
struct stPlayerBrifData;
class CMysqlRow;
class CDBManager
{
public:
	struct stArgData
	{
		RakNet::RakNetGUID m_nReqrestFromAdd ;
		unsigned int nSessionID ; // always , refer to client session with serveper , used in GameServer and LoginServer , to rresent a Player ;
		unsigned int nExtenArg1 ; // reserver argument , for later use ;
		unsigned int nExtenArg2 ; // reserver argument , for later use ;
		void* pUserData ;   // maybe need data ;
		stArgData(){ nSessionID = 0 ; nExtenArg2 = nExtenArg1 = 0 ; pUserData = NULL ;}
		void Reset(){nSessionID = 0 ; nExtenArg2 = nExtenArg1 = 0 ; assert(pUserData==NULL);}
	};
	typedef std::list<stArgData*> LIST_ARG_DATA ;
public:
	CDBManager(CDBServerApp* theApp);
	~CDBManager();
	void Init();
	void OnMessage(RakNet::Packet* packet);
	void OnDBResult(stDBResult* pResult);
	stArgData* GetReserverArgData();
	unsigned int GenerateUserUID(){ return ++nCurUserUID ;}
	void GetPlayerDetailData(stPlayerDetailData* pData, CMysqlRow&prow);
	void GetPlayerBrifData(stPlayerBrifData*pData,CMysqlRow&prow);
protected:
	LIST_ARG_DATA m_vReserverArgData ;
	CDBServerApp* m_pTheApp ;
	unsigned int nCurUserUID ;   // when server start will get this value from db ;
};