#pragma once 
#include "IGlobalModule.h"
#include "./HttpServer/server.hpp"
class CHttpModule
	:public IGlobalModule
{
public:
	typedef boost::function<bool (http::server::connection_ptr)> httpHandle;
public:
	void init(IServerApp* svrApp)override;
	void update(float fDeta)override;
	bool registerHttpHandle(std::string strURI, httpHandle pHandle );
protected:
	bool onHandleVXPayResult(http::server::connection_ptr ptr );
protected:
	boost::shared_ptr<http::server::server> mHttpServer;
	std::map<std::string, httpHandle> vHttphandles;
};