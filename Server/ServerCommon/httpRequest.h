#pragma once
#include "curl/curl.h"
#include <string>
class CHttpRequestDelegate
{
public:
	virtual ~CHttpRequestDelegate(){}
	virtual void onHttpCallBack(char* pResultData, size_t nDatalen , void* pUserData , size_t nUserTypeArg) = 0 ;
};
class CHttpRequest
{
public:
	~CHttpRequest();
	bool init(const char* pBaseUrl );
	bool performRequest(const char* pAddtionUrl , const char* pData , size_t nLen, void* pUserData , size_t nUserTypeArg = 0 );
	void setDelegate(CHttpRequestDelegate* pDelegate);
	static size_t onRecieveData(void *buffer, size_t size, size_t count, void *user_p);
protected:
	std::string m_strBaseURL ;
	CHttpRequestDelegate* m_pDelegate ;
	CURL* m_pCURL ;
	curl_slist* m_pCurlList ; 
	void* m_pUserData ;
	size_t m_nUserTypeArg ;
};