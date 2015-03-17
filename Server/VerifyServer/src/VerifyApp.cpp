#include "VerifyApp.h"
#include "CommonDefine.h"
#include "ServerMessageDefine.h"
#include "LogManager.h"
CVerifyApp::CVerifyApp()
{
	m_pNetwork = NULL ;
}

CVerifyApp::~CVerifyApp()
{
	if ( m_pNetwork )
	{
		m_pNetwork->ShutDown();
		delete m_pNetwork ;
	}
}

void CVerifyApp::MainLoop()
{
	if ( m_pNetwork )
	{
		m_pNetwork->RecieveMsg() ;
	}

	// check Apple Verify ;
	LIST_VERIFY_REQUEST vOutAppleResult ;
	m_AppleVerifyMgr.GetProcessedRequest(vOutAppleResult) ;
	stVerifyRequest* pVerifyRequest = NULL ;
	LIST_VERIFY_REQUEST::iterator iter = vOutAppleResult.begin() ;
	for ( ; iter != vOutAppleResult.end(); ++iter )
	{
		pVerifyRequest = *iter ;
		if ( pVerifyRequest->eResult == eVerify_Apple_Error )
		{
			CLogMgr::SharedLogMgr()->SystemLog("player uid = %d , shop id = %d apple verfiy error!",pVerifyRequest->nFromPlayerUserUID,pVerifyRequest->nShopItemID);
			FinishVerifyRequest(pVerifyRequest) ;
			PushVerifyRequestToReuse(pVerifyRequest) ;
		}
		else
		{
			CLogMgr::SharedLogMgr()->SystemLog("player uid = %d , shop id = %d apple verfiy success, go on db verfify!",pVerifyRequest->nFromPlayerUserUID,pVerifyRequest->nShopItemID );
			m_DBVerifyMgr.AddRequest(pVerifyRequest) ;
			//pVerifyRequest->eResult = eVerify_Success ;
			//FinishVerifyRequest(pVerifyRequest) ;
			//PushVerifyRequestToReuse(pVerifyRequest) ;
			
		}
	}

	// check DB Verify
	m_DBVerifyMgr.ProcessRequest() ;
	vOutAppleResult.clear() ;
	m_DBVerifyMgr.GetResult(vOutAppleResult) ;
	iter = vOutAppleResult.begin() ;
	for ( ; iter != vOutAppleResult.end(); ++iter )
	{
		pVerifyRequest = *iter ;
		FinishVerifyRequest(pVerifyRequest) ;
		PushVerifyRequestToReuse(pVerifyRequest) ;
	}
	vOutAppleResult.clear() ;
}

void CVerifyApp::Init()
{
	CLogMgr::SharedLogMgr()->SetOutputFile("VerifySvr");
	m_stSvrConfigMgr.LoadFile("../configFile/serverConfig.txt");
	m_pNetwork = new CServerNetwork ;
	m_pNetwork->StartupNetwork(m_stSvrConfigMgr.GetServerConfig(eSvrType_Verify)->nPort,5,m_stSvrConfigMgr.GetServerConfig(eSvrType_Verify)->strPassword) ;
	m_pNetwork->AddDelegate(this);

	m_AppleVerifyMgr.Init() ;
	m_DBVerifyMgr.Init();
	CLogMgr::SharedLogMgr()->SystemLog("START verify server !") ;
}

stVerifyRequest* CVerifyApp::GetRequestToUse()
{
	stVerifyRequest* request = NULL ;
	LIST_VERIFY_REQUEST::iterator iter = m_vListRequest.begin() ;
	if ( iter != m_vListRequest.end() )
	{
		request = *iter ;
		m_vListRequest.erase(iter) ;
	}
	else
	{
		request = new stVerifyRequest ;
	}
	memset(request->pBufferVerifyID,0,sizeof(request->pBufferVerifyID)) ;
	request->pUserData = NULL;
	request->nShopItemID = 0 ;
	return request ;
}

void CVerifyApp::PushVerifyRequestToReuse(stVerifyRequest* pRequest )
{
	m_vListRequest.push_back(pRequest) ;
}

void CVerifyApp::FinishVerifyRequest(stVerifyRequest* pRequest)
{
	stMsgFromVerifyServer msg ;
	msg.nShopItemID = pRequest->nShopItemID ;
	msg.nRet = pRequest->eResult ;
	msg.nBuyerPlayerUserUID = pRequest->nFromPlayerUserUID ;
	msg.nBuyForPlayerUserUID = pRequest->nBuyedForPlayerUserUID ;
	m_pNetwork->SendMsg((char*)&msg,sizeof(msg),pRequest->nFromNetID,false) ;
	CLogMgr::SharedLogMgr()->SystemLog( "finish verify transfaction shopid = %d ,uid = %d ret = %d",msg.nShopItemID,msg.nBuyerPlayerUserUID,msg.nRet ) ;
}

static const std::string base64_chars = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";  
std::string base64_encode(unsigned char const* bytes_to_encode, unsigned int in_len)  
{  
	std::string ret;  
	int i = 0;  
	int j = 0;  
	unsigned char char_array_3[3];  
	unsigned char char_array_4[4];  

	while (in_len--)  
	{  
		char_array_3[i++] = *(bytes_to_encode++);  
		if (i == 3) {  
			char_array_4[0] = (char_array_3[0] & 0xfc) >> 2;  
			char_array_4[1] = ((char_array_3[0] & 0x03) << 4) + ((char_array_3[1] & 0xf0) >> 4);  
			char_array_4[2] = ((char_array_3[1] & 0x0f) << 2) + ((char_array_3[2] & 0xc0) >> 6);  
			char_array_4[3] = char_array_3[2] & 0x3f;  

			for (i = 0; (i <4) ; i++)  
				ret += base64_chars[char_array_4[i]];  
			i = 0;  
		}  
	}  

	if (i)  
	{  
		for (j = i; j < 3; j++)  
			char_array_3[j] = '/0';  

		char_array_4[0] = (char_array_3[0] & 0xfc) >> 2;  
		char_array_4[1] = ((char_array_3[0] & 0x03) << 4) + ((char_array_3[1] & 0xf0) >> 4);  
		char_array_4[2] = ((char_array_3[1] & 0x0f) << 2) + ((char_array_3[2] & 0xc0) >> 6);  
		char_array_4[3] = char_array_3[2] & 0x3f;  

		for (j = 0; (j < i + 1); j++)  
			ret += base64_chars[char_array_4[j]];  

		while ((i++ < 3))  
			ret += '=';  

	}  

	return ret;  

}  

bool CVerifyApp::OnMessage( Packet* pData )
{
	stMsg* pMsg = (stMsg*)pData->_orgdata ;
	if ( pMsg->usMsgType == MSG_VERIFY_TANSACTION )
	{
		stMsgToVerifyServer* pReal = (stMsgToVerifyServer*)pMsg ;
		stVerifyRequest* pRequest = GetRequestToUse() ;
		pRequest->nFromNetID = pData->_connectID ;
		pRequest->nFromPlayerUserUID = pReal->nBuyerPlayerUserUID ;
		pRequest->nShopItemID = pReal->nShopItemID;
		pRequest->nBuyedForPlayerUserUID = pReal->nBuyForPlayerUserUID ;
		pRequest->nRequestType = 0 ;  // now just apple ;
		std::string str = base64_encode(((unsigned char*)pMsg) + sizeof(stMsgToVerifyServer),pReal->nTranscationIDLen);
		//std::string str = base64_encode(((unsigned char*)pMsg) + sizeof(stMsgToVerifyServer),20);
		memcpy(pRequest->pBufferVerifyID,str.c_str(),strlen(str.c_str()));
		m_AppleVerifyMgr.AddRequest(pRequest) ;
		printf("recived a transfaction need to verify shop id = %d useruid = %d\n",pReal->nShopItemID,pReal->nBuyerPlayerUserUID);
	}
	else if ( pMsg->cSysIdentifer == ID_MSG_VERIFY )
	{

	}
	else
	{
		printf("unknown msg type = %d,%s, close connection",pMsg->usMsgType,m_pNetwork->GetIPInfoByConnectID(pData->_connectID)) ;
		m_pNetwork->ClosePeerConnection(pData->_connectID);
	}
	return true ;
}

void CVerifyApp::OnNewPeerConnected(CONNECT_ID nNewPeer, ConnectInfo* IpInfo )
{
	stMsg msg ;
	msg.cSysIdentifer = ID_MSG_VERIFY ;
	msg.usMsgType = MSG_VERIFY_VERIYF ;
	m_pNetwork->SendMsg((char*)&msg,sizeof(msg),nNewPeer,false) ;
	CLogMgr::SharedLogMgr()->SystemLog("a peer connected ip: %s port = %d",IpInfo->strAddress,IpInfo->nPort);
}

void CVerifyApp::OnPeerDisconnected(CONNECT_ID nPeerDisconnected, ConnectInfo* IpInfo )
{
	if ( IpInfo )
	{
		CLogMgr::SharedLogMgr()->SystemLog("a peer disconnected ip: %s port = %d",IpInfo->strAddress,IpInfo->nPort);
	}
	else
	{
		CLogMgr::SharedLogMgr()->SystemLog("a peer disconnected");
	}
}