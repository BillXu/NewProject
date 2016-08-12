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

void CVerifyApp::update(float fDeta )
{
	IServerApp::update(fDeta);

	if ( m_pNetwork )
	{
		m_pNetwork->RecieveMsg() ;
	}

	// check Apple Verify ;
	LIST_VERIFY_REQUEST vOutAppleResult , vMiResult,weChatResult;
	m_AppleVerifyMgr.GetProcessedRequest(vOutAppleResult) ;
	m_MiVerifyMgr.GetProcessedRequest(vMiResult);
	m_tWechtVerifyMgr.GetProcessedRequest(weChatResult);
	vOutAppleResult.insert(vOutAppleResult.begin(),vMiResult.begin(),vMiResult.end());
	vOutAppleResult.insert(vOutAppleResult.begin(),weChatResult.begin(),weChatResult.end());
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

	// check order request ;
	LIST_ORDER_REQUEST vOrderResult;
	m_tWechatOrderMgr.GetProcessedRequest(vOrderResult);
	for ( stShopItemOrderRequest* pOrder : vOrderResult )
	{
		stMsgVerifyItemOrderRet msgRet ;
		memset(msgRet.cPrepayId,0,sizeof(msgRet.cPrepayId));
		memset(msgRet.cOutTradeNo,0,sizeof(msgRet.cOutTradeNo));
		memcpy(msgRet.cOutTradeNo,pOrder->cOutTradeNo,sizeof(pOrder->cOutTradeNo));
		memcpy(msgRet.cPrepayId,pOrder->cPrepayId,sizeof(msgRet.cPrepayId));
		msgRet.nChannel = pOrder->nChannel ;
		msgRet.nRet = pOrder->nRet ;
		sendMsg(pOrder->nSessionID,(char*)&msgRet,sizeof(msgRet));
		CLogMgr::SharedLogMgr()->SystemLog("finish order for sessionid = %d, ret = %d ",pOrder->nSessionID,pOrder->nRet) ;
		delete pOrder ;
		pOrder = nullptr ;
	}
	vOrderResult.clear() ;
}

bool CVerifyApp::init()
{
	IServerApp::init();
	CLogMgr::SharedLogMgr()->SetOutputFile("VerifySvr");

	CSeverConfigMgr stSvrConfigMgr ;
	stSvrConfigMgr.LoadFile("../configFile/serverConfig.txt");
	stServerConfig* pConfig = stSvrConfigMgr.GetServerConfig(eSvrType_Center) ;
	if ( pConfig == NULL )
	{
		CLogMgr::SharedLogMgr()->ErrorLog("center svr config is null , so can not connected to !") ;
		return false;
	}
	setConnectServerConfig(pConfig);

	m_AppleVerifyMgr.Init() ;
	m_MiVerifyMgr.Init();
	m_DBVerifyMgr.Init();
	m_tWechatOrderMgr.Init() ;
	m_tWechtVerifyMgr.Init() ;
	CLogMgr::SharedLogMgr()->SystemLog("START verify server !") ;

	return true;
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
	request->nSessionID = 0 ;
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
	sendMsg(pRequest->nSessionID,(char*)&msg,sizeof(msg));
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

bool CVerifyApp::onLogicMsg( stMsg* pMsg , eMsgPort eSenderPort , uint32_t nSessionID )
{
	if ( IServerApp::onLogicMsg(pMsg,eSenderPort,nSessionID) )
	{
		return true ;
	}

	if ( MSG_VERIFY_ITEM_ORDER == pMsg->usMsgType )
	{
		stMsgVerifyItemOrder* pOrder = (stMsgVerifyItemOrder*)pMsg ;

		stShopItemOrderRequest* pRe = new stShopItemOrderRequest ;
		memset(pRe,0,sizeof(stShopItemOrderRequest)) ;
		sprintf_s(pRe->cShopDesc,50,pOrder->cShopDesc);
		sprintf_s(pRe->cOutTradeNo,32,pOrder->cOutTradeNo);
		pRe->nPrize = pOrder->nPrize;
		sprintf_s(pRe->cTerminalIp,17,pOrder->cTerminalIp);
		pRe->nChannel = pOrder->nChannel ;
		pRe->nFromPlayerUserUID =  0 ;
		pRe->nSessionID = nSessionID ;
		m_tWechatOrderMgr.AddRequest(pRe);
		return true ;
	}

	if ( pMsg->usMsgType == MSG_VERIFY_TANSACTION )
	{
		stMsgToVerifyServer* pReal = (stMsgToVerifyServer*)pMsg ;
		stVerifyRequest* pRequest = GetRequestToUse() ;
		pRequest->nFromPlayerUserUID = pReal->nBuyerPlayerUserUID ;
		pRequest->nShopItemID = pReal->nShopItemID;
		pRequest->nBuyedForPlayerUserUID = pReal->nBuyForPlayerUserUID ;
		pRequest->nChannel = pReal->nChannel ;  // now just apple ;
		pRequest->nSessionID = nSessionID ;
		pRequest->nMiUserUID = pReal->nMiUserUID ;

		if ( pRequest->nMiUserUID && pRequest->nChannel == ePay_XiaoMi )
		{
			memcpy(pRequest->pBufferVerifyID,((unsigned char*)pMsg) + sizeof(stMsgToVerifyServer),pReal->nTranscationIDLen);
			m_MiVerifyMgr.AddRequest(pRequest) ;
		}
		else if ( pRequest->nChannel == ePay_AppStore )
		{
			if ( pRequest->nShopItemID > 1 )
			{
				pRequest->nShopItemID -= 1 ;
			}

			std::string str = base64_encode(((unsigned char*)pMsg) + sizeof(stMsgToVerifyServer),pReal->nTranscationIDLen);
			//std::string str = base64_encode(((unsigned char*)pMsg) + sizeof(stMsgToVerifyServer),20);
			memcpy(pRequest->pBufferVerifyID,str.c_str(),strlen(str.c_str()));
			m_AppleVerifyMgr.AddRequest(pRequest) ;
		}
		else if ( ePay_WeChat == pRequest->nChannel )
		{
			memcpy(pRequest->pBufferVerifyID,((unsigned char*)pMsg) + sizeof(stMsgToVerifyServer),pReal->nTranscationIDLen);

			std::string strTradeNo(pRequest->pBufferVerifyID);
			std::string shopItem = strTradeNo.substr(0,strTradeNo.find_first_of('E')) ;
			if ( atoi(shopItem.c_str()) != pRequest->nShopItemID )
			{
				printf("shop id and verify id not the same \n") ;
				pRequest->eResult = eVerify_Apple_Error ;
				m_tWechtVerifyMgr.AddProcessedResult(pRequest) ;
			}
			else
			{
				m_tWechtVerifyMgr.AddRequest(pRequest) ;
			}
		}
		else
		{
			CLogMgr::SharedLogMgr()->ErrorLog("unknown pay channecl = %d, uid = %d",pRequest->nChannel,pReal->nBuyerPlayerUserUID ) ;
			PushVerifyRequestToReuse(pRequest) ;
		}

		printf("recived a transfaction need to verify shop id = %d useruid = %d channel id = %d\n",pReal->nShopItemID,pReal->nBuyerPlayerUserUID,pReal->nChannel );
	}
	else
	{
		printf("unknown msg type = %d,%d, close connection",pMsg->usMsgType,eSenderPort) ;
	}
	return true ;
}

uint16_t CVerifyApp::getLocalSvrMsgPortType()
{
	return ID_MSG_PORT_VERIFY ;
}