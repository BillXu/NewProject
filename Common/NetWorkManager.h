//
//  NetWorkManager.h
//  God
//
//  Created by Xu BILL on 12-10-30.
//
//

#ifndef God_NetWorkManager_h
#define God_NetWorkManager_h
#include <list>
#include "RakNetTypes.h"
class CNetWorkMgr ;
struct stMsg ;
namespace RakNet
{
    class RakPeerInterface ;
}

class CNetMessageDelegate
{
public:
    enum eConnectState
    {
        eConnect_SeverFull,
        eConnect_Accepted,
        eConnect_Failed,
        eConnect_Banned,
        eConnect_Max,
    };
public:
	CNetMessageDelegate(){ m_pNetWorkMgr = NULL ;m_nPriority =0 ;};
	virtual ~CNetMessageDelegate(){} ;
	virtual bool OnMessage( RakNet::Packet* pMsg ) = 0 ;
	virtual bool OnLostSever(RakNet::Packet* pMsg){ return  false; };
	virtual bool OnConnectStateChanged( eConnectState eSate, RakNet::Packet* pMsg){ return false ;} 
	void SetPriority( unsigned int nPriority  );
	unsigned GetPriority(){ return m_nPriority ;}
	void SetNetWorkMgr(CNetWorkMgr* pNetWork ) { m_pNetWorkMgr = pNetWork ;}
protected:
	unsigned short m_nPriority ;  // big first , small later ;
	CNetWorkMgr* m_pNetWorkMgr ;
};

class CNetWorkMgr
{
public:
    enum eConnectType
    {
        eConnectType_None ,
        eConnectType_Connecting,
        eConnectType_Connected,
        eConnectType_Disconnectd ,
        eConnectType_Max,
    };
public:
    typedef std::list<CNetMessageDelegate*> LIST_DELEGATE ;
    typedef bool (CNetWorkMgr::*lpfunc)(CNetMessageDelegate* pDeleate ,void* pData );
public:
    CNetWorkMgr();
    ~CNetWorkMgr();
   // static CNetWorkMgr* SharedNetWorkMgr();
    void SetupNetwork( int nIntendServerCount = 1 );
    bool ConnectToServer( const char* pSeverIP, unsigned short nPort , const char* pPassword = NULL );
    void ReciveMessage();
    bool SendMsg( const char* pbuffer , int iSize );
	bool SendMsg( const char* pbuffer , int iSize,RakNet::RakNetGUID& nServerNetUID );
    
	void AddMessageDelegate(CNetMessageDelegate * pDelegate, unsigned short nPrio );
	void AddMessageDelegate(CNetMessageDelegate *pDelegate) ;
	void RemoveMessageDelegate(CNetMessageDelegate* pDelegate);
	void RemoveAllDelegate();
	void EnumDeleagte( CNetWorkMgr* pTarget, lpfunc pFunc, void* pData );
	void DisconnectServer( RakNet::RakNetGUID& nServerNetUID );
    
    void ShutDown();
public:
    static int s_nCurrentDataSize ;
protected:
	bool OnLostServer( CNetMessageDelegate* pDeleate,void* pData );
	bool OnReciveLogicMessage( CNetMessageDelegate* pDeleate,void* pData );
	bool OnConnectSateChanged( CNetMessageDelegate* pDeleate,void* pData );
    void ProcessDelegateAddRemove();
protected:
    LIST_DELEGATE m_vAllDelegate;
    LIST_DELEGATE m_vWillAddDelegate;
    LIST_DELEGATE m_vWillRemoveDelegate ;
    RakNet::RakPeerInterface* m_pNetPeer;
    RakNet::RakNetGUID m_nCurrentServer ;  // the newest accepted Server ; 
	short m_nConnectedTo ;
	short m_nMaxConnectTo ;
};

#endif
