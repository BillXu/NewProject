#include "tinyxml/tinyxml.h"
#include "Md5.h"
#include "HttpModule.h"
#include "log4z.h"
#include <boost/algorithm/string.hpp>  
#include "VerifyApp.h"
void CHttpModule::init(IServerApp* svrApp)
{
	IGlobalModule::init(svrApp);
	mHttpServer = boost::make_shared<http::server::server>(5006);
	mHttpServer->run();

	registerHttpHandle("/vxpay.php", boost::bind(&CHttpModule::onHandleVXPayResult, this, boost::placeholders::_1));
}

void CHttpModule::update(float fDeta)
{
	IGlobalModule::update(fDeta);
	std::set<http::server::connection_ptr> vOut;
	if (!mHttpServer->getRequestConnects(vOut))
	{
		return;
	}

	for (auto& ref : vOut)
	{
		auto req = ref->getReqPtr();
		auto pReply = ref->getReplyPtr();
		auto iter = vHttphandles.find( req->uri );
		if (iter == vHttphandles.end())
		{
			LOGFMTE("no handle for uri = %s",req->uri.c_str());
			*pReply = http::server::reply::stock_reply(http::server::reply::bad_request);
			ref->doReply();
			continue;
		}
		
		auto pfunc = iter->second;
		if ( !pfunc(ref) )
		{
			*pReply = http::server::reply::stock_reply(http::server::reply::bad_request);
			ref->doReply();
		}
	}
	vOut.clear();
}

bool CHttpModule::registerHttpHandle(std::string strURI, httpHandle pHandle)
{
	auto iter = vHttphandles.find(strURI);
	if (iter != vHttphandles.end())
	{
		LOGFMTE("already register handle for uri = %s",strURI.c_str());
		return false;
	}
	vHttphandles[strURI] = pHandle;
	return true;
}


 std::string getXmlNodeValue(const char* pnodeName, TiXmlNode* pRoot)
{
	std::string str = "";
	TiXmlElement* pValueParent = (TiXmlElement*)pRoot->FirstChild(pnodeName);
	if (pValueParent)
	{
		TiXmlNode* pValue = pValueParent->FirstChild();
		if (pValue)
		{
			str = pValue->Value();
			return str;
		}
	}
	LOGFMTE("xml node = %s value is null", pnodeName);
	return str;
}

// process vx pay result 
bool CHttpModule::onHandleVXPayResult(http::server::connection_ptr ptr)
{
	auto req = ptr->getReqPtr();
	auto res = ptr->getReplyPtr();

	if (req->contentSize == 0)
	{
		return false;
	}

	// parse xml 
	TiXmlDocument t;
	t.Parse(req->reqContent.c_str(), 0, TIXML_ENCODING_UTF8);
	if (t.Error())
	{
		LOGFMTE("vx pay parse xml error : %s",t.ErrorDesc());
		return false;
	}
#ifndef Node_Value_F  
#define node_Value( x ) getXmlNodeValue((x),pRoot)
#define Node_Value_F ;
#endif 
	TiXmlNode* pRoot = t.RootElement();
	uint8_t nRet = 0;
	do
	{
		if (!pRoot)
		{
			nRet = 1;
			break;
		}

		auto retCode = node_Value("return_code");
		if (retCode != "SUCCESS")
		{
			nRet = 3;
			auto strEmsg = node_Value("return_msg");
			LOGFMTE("ret msg : %s", strEmsg.c_str());
			break;
		}
 
		// do parse 
		auto fee = node_Value("total_fee");
		auto strTradeNo = node_Value("out_trade_no");
		auto payResult = node_Value("result_code");
		auto payTime = node_Value("time_end");
		if ("FAIL" == payResult)
		{
			return true ;
		}
		
		std::vector<std::string> vOut;
		boost::split(vOut,strTradeNo,boost::is_any_of("E"));
		if (vOut.size() < 2)
		{
			LOGFMTE("trade out error = %s",strTradeNo.c_str() );
			nRet = 4;
			break;
		}
		auto shopItem = vOut[0];
		auto userUID = vOut[1];
		LOGFMTD( "GO TO DB Verify trade = %s , fee = %s payTime = %s",strTradeNo.c_str(),fee.c_str(),payTime.c_str() );

		// do DB verify ;
		auto pVeirfyModule = ((CVerifyApp*)getSvrApp())->getTaskPoolModule();
		pVeirfyModule->doDBVerify(atoi(userUID.c_str()), atoi(shopItem.c_str()), ePay_WeChat,strTradeNo);
	} while (0);

	std::string str = "";
	if (nRet != 0)
	{
		str = "<xml><return_code><![CDATA[FAIL]]></return_code> <return_msg><![CDATA[unknown]]></return_msg> </xml> ";
	}
	else
	{
		str = "<xml><return_code><![CDATA[SUCCESS]]></return_code> <return_msg><![CDATA[OK]]></return_msg> </xml> ";
	}

	res->setContent(str,"text/xml");
	ptr->doReply();
	t.SaveFile("reT.xml");
	return true;
}

