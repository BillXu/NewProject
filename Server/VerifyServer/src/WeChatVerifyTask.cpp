#include "WeChatVerifyTask.h"
#include "VerifyRequest.h"
#include <cassert>
#include "tinyxml/tinyxml.h"
#include "Md5.h"
#include<algorithm>
#include "LogManager.h"
CWechatVerifyTask::CWechatVerifyTask( uint32_t nTaskID ) 
	:IVerifyTask(nTaskID)
{
	m_tHttpRequest.init("https://api.mch.weixin.qq.com/pay/orderquery","text/xml");
	m_tHttpRequest.setDelegate(this);
}

uint8_t CWechatVerifyTask::performTask()
{
	auto pRequest = getVerifyResult() ; 
	TiXmlElement *xmlRoot = new TiXmlElement("xml"); 

	TiXmlElement *pNode = new TiXmlElement("appid"); 
	TiXmlText *pValue = new TiXmlText( "wx66f2837c43330a7b" );
	xmlRoot->LinkEndChild(pNode);
	pNode->LinkEndChild(pValue);
	std::string strForMd5 = "appid=wx66f2837c43330a7b";

	// ok 
	pNode = new TiXmlElement("mch_id"); 
	pValue = new TiXmlText( "1308480601" );
	xmlRoot->LinkEndChild(pNode);
	pNode->LinkEndChild(pValue);
	strForMd5 += "&mch_id=1308480601" ;

	// ok 
	std::string strRandString = "";
	uint8_t nCnt = 32; 
	while ( nCnt-- )
	{
		uint8_t n = rand() % 2 ;
		char tCh = '0' + rand() % 10 ;
		if ( n == 0 )
		{
			tCh = 'A' + rand() % 26 ;
		}
		strRandString.append(1,tCh) ;
	}

	pNode = new TiXmlElement("nonce_str"); 
	pValue = new TiXmlText( strRandString.c_str() );
	xmlRoot->LinkEndChild(pNode);
	pNode->LinkEndChild(pValue);
	strForMd5 += "&nonce_str=" ;
	strForMd5 += strRandString;
	printf("rand str: %s\n",strRandString.c_str()) ;

	// ok 
	pNode = new TiXmlElement("out_trade_no"); 
	pValue = new TiXmlText( pRequest->pBufferVerifyID );
	xmlRoot->LinkEndChild(pNode);
	pNode->LinkEndChild(pValue);
	strForMd5 += "&out_trade_no=" ;
	strForMd5.append(pRequest->pBufferVerifyID);


	static char pBuffer [200] = { 0 } ;
	// and key value 
	strForMd5 += "&key=E97ED2537D229C0E967042D2E7F1C936" ;
	printf("formd5Str: %s\n",strForMd5.c_str());
	CMD5 md5 ;
	md5.GenerateMD5((unsigned char*)strForMd5.c_str(),strlen(strForMd5.c_str())) ;
	std::string strSign = md5.ToString() ;
	transform(strSign.begin(), strSign.end(), strSign.begin(),  toupper);
	// not ok 
	printf("sing: %s\n",strSign.c_str());
	pNode = new TiXmlElement("sign"); 
	pValue = new TiXmlText( strSign.c_str() );
	xmlRoot->LinkEndChild(pNode);
	pNode->LinkEndChild(pValue);

	TiXmlPrinter printer; 
	xmlRoot->Accept( &printer ); 
	std::string stringBuffer = printer.CStr(); 
	printf("finsStr : %s\n",stringBuffer.c_str());
	auto ret = m_tHttpRequest.performRequest(nullptr,stringBuffer.c_str(),stringBuffer.size(),nullptr ) ;
	
	delete xmlRoot ;
	xmlRoot = nullptr ;

	if ( ret )
	{
		return 0 ;
	}
	return 1 ;
}

void CWechatVerifyTask::onHttpCallBack(char* pResultData, size_t nDatalen , void* pUserData , size_t nUserTypeArg )
{
	auto pRequest = getVerifyResult() ;
	assert(pRequest != nullptr && "must not null") ;
	TiXmlDocument t ;
	std::string str( pResultData,nDatalen );
	t.Parse(str.c_str(),0,TIXML_ENCODING_UTF8);
	CLogMgr::SharedLogMgr()->PrintLog("weChatPayRet : %s",str.c_str()) ;
	TiXmlNode* pRoot = t.RootElement();
	if ( pRoot )
	{
		TiXmlElement* pValueParent = (TiXmlElement*)pRoot->FirstChild("return_code");
		if ( pValueParent )
		{
			TiXmlNode* pValue = pValueParent->FirstChild();
			if ( strcmp(pValue->Value(),"SUCCESS") == 0 )
			{
				TiXmlElement* pResultNode = (TiXmlElement*)pRoot->FirstChild("result_code");
				TiXmlNode* pRet = pResultNode->FirstChild();
				if ( strcmp(pRet->Value(),"SUCCESS") == 0 )
				{
					TiXmlElement* pState = (TiXmlElement*)pRoot->FirstChild("trade_state");
					TiXmlNode* pStateRet = pState->FirstChild();
					if ( strcmp(pStateRet->Value(),"SUCCESS") == 0 )
					{
						CLogMgr::SharedLogMgr()->PrintLog("weChatVerfiy success ") ;
						// success ;
						pRequest->eResult = eVerify_Apple_Success ;
						return  ;
					}
				}
			}
		}
	}
	pRequest->eResult = eVerify_Apple_Error ;
	printf("we chat verify failed \n") ;
	t.SaveFile("reTCheck.xml");
	return ;
}
