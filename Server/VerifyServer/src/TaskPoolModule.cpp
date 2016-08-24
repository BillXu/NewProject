#include "TaskPoolModule.h"
#include "WeChatOrderTask.h"
#include "testTask.h"
#include "Timer.h"
#include "ServerMessageDefine.h"
#include "LogManager.h"
#include "ISeverApp.h"
#include "VerifyRequest.h"
#include "AppleVerifyTask.h"
#include "WeChatVerifyTask.h"
#include "DBVerifyTask.h"
void CTaskPoolModule::init( IServerApp* svrApp )
{
	IGlobalModule::init(svrApp) ;
	m_tTaskPool.init(this,3);

	// test code 
	//static CTimer tTim ;
	//tTim.setInterval(15) ;
	//tTim.setIsAutoRepeat(true);
	//tTim.setCallBack([this](CTimer* p , float f ){ printf("timer invoker\n");testFunc();}) ;
	//tTim.start();
	// test code 
}

void CTaskPoolModule::onExit()
{
	getPool().closeAll() ;
}

bool CTaskPoolModule::onMsg(stMsg* pMsg , eMsgPort eSenderPort , uint32_t nSessionID)
{
	if ( IGlobalModule::onMsg(pMsg,eSenderPort,nSessionID) )
	{
		return true;
	}

	if ( MSG_VERIFY_ITEM_ORDER == pMsg->usMsgType )
	{
		onWechatOrder(pMsg,eSenderPort,nSessionID) ;
		return true ;
	}

	if ( pMsg->usMsgType == MSG_VERIFY_TANSACTION )
	{
		onVerifyMsg(pMsg,eSenderPort,nSessionID) ;
		return true ;
	}

	return  false ;
}

bool CTaskPoolModule::onMsg(Json::Value& prealMsg ,uint16_t nMsgType, eMsgPort eSenderPort , uint32_t nSessionID)
{
	if ( IGlobalModule::onMsg(prealMsg,nMsgType,eSenderPort,nSessionID ) )
	{
		return  true ;
	}

	return false;
}

void CTaskPoolModule::update(float fDeta )
{
	IGlobalModule::update(fDeta) ;
	m_tTaskPool.update();
}

void CTaskPoolModule::testFunc()
{
	printf("task go \n") ;
	uint32_t nCnt = 5 ;
	while (nCnt--)
	{
		auto p = getPool().getReuseTaskObjByID( nCnt );
		CTestTask* pTest = (CTestTask*)p.get();
		pTest->nTimes = rand() % 2 + 3 ;
		pTest->nreal = pTest->nTimes ;
		pTest->setCallBack([](ITask::ITaskPrt ptr ){
			CTestTask* pTest = (CTestTask*)ptr.get();
			printf("id = %u , times = %u , call back \n",ptr->getTaskID(),pTest->nreal);
		} ) ;
		getPool().postTask(p);
	}

	
}

ITask::ITaskPrt CTaskPoolModule::createTask( uint32_t nTaskID )
{
	switch (nTaskID)
	{
	case eTask_WechatOrder:
		{
			std::shared_ptr<CWeChatOrderTask> pTask ( new CWeChatOrderTask(nTaskID)) ;
			return pTask  ;
		}
		break;
	case eTask_WechatVerify:
		{
			std::shared_ptr<CWechatVerifyTask> pTask ( new CWechatVerifyTask(nTaskID)) ;
			return pTask  ;
		}
		break;
	case eTask_AppleVerify:
		{
			std::shared_ptr<CAppleVerifyTask> pTask ( new CAppleVerifyTask(nTaskID)) ;
			return pTask  ;
		}
		break;
	case eTask_DBVerify:
		{
			std::shared_ptr<CDBVerfiyTask> pTask ( new CDBVerfiyTask(nTaskID)) ;
			return pTask  ;
		}
		break;
	default:
		break;
	}
	return nullptr ;
}

// logic  
void CTaskPoolModule::onWechatOrder( stMsg* pMsg, eMsgPort eSenderPort , uint32_t nSessionID )
{
	stMsgVerifyItemOrder* pOrder = (stMsgVerifyItemOrder*)pMsg ;

	auto pTask = getPool().getReuseTaskObjByID(eTask_WechatOrder);
	CWeChatOrderTask* pTaskObj = (CWeChatOrderTask*)pTask.get();
	// set call back 
	if ( pTask->getCallBack() == nullptr )
	{
		pTask->setCallBack([this](ITask::ITaskPrt ptr )
		{
			CWeChatOrderTask* pTask = (CWeChatOrderTask*)ptr.get();
			auto pOrder = pTask->getCurRequest().get(); 
			stMsgVerifyItemOrderRet msgRet ;
			memset(msgRet.cPrepayId,0,sizeof(msgRet.cPrepayId));
			memset(msgRet.cOutTradeNo,0,sizeof(msgRet.cOutTradeNo));
			memcpy(msgRet.cOutTradeNo,pOrder->cOutTradeNo,sizeof(pOrder->cOutTradeNo));
			memcpy(msgRet.cPrepayId,pOrder->cPrepayId,sizeof(msgRet.cPrepayId));
			msgRet.nChannel = pOrder->nChannel ;
			msgRet.nRet = pOrder->nRet ;
			getSvrApp()->sendMsg(pOrder->nSessionID,(char*)&msgRet,sizeof(msgRet));
			CLogMgr::SharedLogMgr()->SystemLog("finish order for sessionid = %d, ret = %d ",pOrder->nSessionID,pOrder->nRet) ;
		}
		) ;
	}

	// set request info 
	std::shared_ptr<stShopItemOrderRequest> pRe = pTaskObj->getCurRequest() ;
	if ( pRe == nullptr )
	{
		pRe = std::shared_ptr<stShopItemOrderRequest>( new stShopItemOrderRequest );
		pTaskObj->setInfo(pRe);
	}
	memset(pRe.get(),0,sizeof(stShopItemOrderRequest)) ;
	sprintf_s(pRe->cShopDesc,50,pOrder->cShopDesc);
	sprintf_s(pRe->cOutTradeNo,32,pOrder->cOutTradeNo);
	pRe->nPrize = pOrder->nPrize;
	sprintf_s(pRe->cTerminalIp,17,pOrder->cTerminalIp);
	pRe->nChannel = pOrder->nChannel ;
	pRe->nFromPlayerUserUID =  0 ;
	pRe->nSessionID = nSessionID ;

	// do the request 
	getPool().postTask(pTask);
	return  ;
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

void CTaskPoolModule::onVerifyMsg( stMsg* pMsg, eMsgPort eSenderPort , uint32_t nSessionID )
{
	stMsgToVerifyServer* pReal = (stMsgToVerifyServer*)pMsg ;

	IVerifyTask::VERIFY_REQUEST_ptr pRequest ( new stVerifyRequest() );
	pRequest->nFromPlayerUserUID = pReal->nBuyerPlayerUserUID ;
	pRequest->nShopItemID = pReal->nShopItemID;
	pRequest->nBuyedForPlayerUserUID = pReal->nBuyForPlayerUserUID ;
	pRequest->nChannel = pReal->nChannel ;  // now just apple ;
	pRequest->nSessionID = nSessionID ;
	pRequest->nMiUserUID = pReal->nMiUserUID ;

	CLogMgr::SharedLogMgr()->PrintLog("received a transfaction need to verify shop id = %u userUID = %u channel = %d\n",pReal->nShopItemID,pReal->nBuyerPlayerUserUID,pReal->nChannel );

	if ( pRequest->nMiUserUID && pRequest->nChannel == ePay_XiaoMi )
	{
		memcpy(pRequest->pBufferVerifyID,((unsigned char*)pMsg) + sizeof(stMsgToVerifyServer),pReal->nTranscationIDLen);
		//m_MiVerifyMgr.AddRequest(pRequest) ;
		CLogMgr::SharedLogMgr()->ErrorLog("we don't have xiao mi channel") ;
		return ;
	}
	
	ITask::ITaskPrt pTask = nullptr ;
	if ( pRequest->nChannel == ePay_AppStore )
	{
		if ( pRequest->nShopItemID > 1 )
		{
			pRequest->nShopItemID -= 1 ;
		}

		std::string str = base64_encode(((unsigned char*)pMsg) + sizeof(stMsgToVerifyServer),pReal->nTranscationIDLen);
		//std::string str = base64_encode(((unsigned char*)pMsg) + sizeof(stMsgToVerifyServer),20);
		memcpy(pRequest->pBufferVerifyID,str.c_str(),strlen(str.c_str()));
		pTask = getPool().getReuseTaskObjByID(eTask_AppleVerify) ;
	}
	else if ( ePay_WeChat == pRequest->nChannel )
	{
		memcpy(pRequest->pBufferVerifyID,((unsigned char*)pMsg) + sizeof(stMsgToVerifyServer),pReal->nTranscationIDLen);
		if ( pRequest->nShopItemID > 1 )
		{
			pRequest->nShopItemID -= 1 ;
		}
		std::string strTradeNo(pRequest->pBufferVerifyID);
		std::string shopItem = strTradeNo.substr(0,strTradeNo.find_first_of('E')) ;
		if ( atoi(shopItem.c_str()) != pRequest->nShopItemID )
		{
			printf("shop id and verify id not the same \n") ;
			pRequest->eResult = eVerify_Apple_Error ;
			sendVerifyResult(pRequest) ;
			return ;
		}
		else
		{
			pTask = getPool().getReuseTaskObjByID(eTask_WechatVerify) ;
		}
	}
	else
	{
		CLogMgr::SharedLogMgr()->ErrorLog("unknown pay channecl = %d, uid = %d",pRequest->nChannel,pReal->nBuyerPlayerUserUID ) ;
		return ;
	}

	if ( !pTask )
	{
		CLogMgr::SharedLogMgr()->ErrorLog("why verify task is null ? ") ;
		return ;
	}

	auto* pVerifyTask = (IVerifyTask*)pTask.get();
	pVerifyTask->setVerifyRequest(pRequest) ;
	pVerifyTask->setCallBack([this](ITask::ITaskPrt ptr ) 
	{
		auto* pAready = (IVerifyTask*)ptr.get();
		auto pResult = pAready->getVerifyResult() ;
		if ( eVerify_Apple_Error == pResult->eResult )
		{
			CLogMgr::SharedLogMgr()->ErrorLog("apple verify Error  uid = %u, channel = %u,shopItem id = %u",pResult->nFromPlayerUserUID,pResult->nChannel,pResult->nShopItemID) ;
			// send to client ;
			sendVerifyResult(pResult) ;
			return ;
		}

		CLogMgr::SharedLogMgr()->SystemLog("apple verify success  uid = %u, channel = %u,shopItem id = %u,go on DB verify",pResult->nFromPlayerUserUID,pResult->nChannel,pResult->nShopItemID) ;
		auto pDBTask = getPool().getReuseTaskObjByID(eTask_DBVerify);
		auto pDBVerifyTask = (IVerifyTask*)pDBTask.get() ;

		pDBVerifyTask->setVerifyRequest(pResult) ;
		pDBVerifyTask->setCallBack([this](ITask::ITaskPrt ptr){ auto pAready = (IVerifyTask*)ptr.get(); sendVerifyResult(pAready->getVerifyResult()) ;} ) ;
		getPool().postTask(pDBTask) ;
	} ) ;
	getPool().postTask(pTask);
}

void CTaskPoolModule::sendVerifyResult(std::shared_ptr<stVerifyRequest> & pResult )
{
	stMsgFromVerifyServer msg ;
	msg.nShopItemID = pResult->nShopItemID ;
	msg.nRet = pResult->eResult ;
	msg.nBuyerPlayerUserUID = pResult->nFromPlayerUserUID ;
	msg.nBuyForPlayerUserUID = pResult->nBuyedForPlayerUserUID ;
	getSvrApp()->sendMsg(pResult->nSessionID,(char*)&msg,sizeof(msg));
	CLogMgr::SharedLogMgr()->SystemLog( "finish verify transfaction shopid = %u ,uid = %d ret = %d",msg.nShopItemID,msg.nBuyerPlayerUserUID,msg.nRet ) ;
}