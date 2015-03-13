#pragma once
#include "INetwork.h"
class CClientNetwork
	:public INetwork
{
public:
	void Start();
	CONNECT_ID ConnectToServer(const char* pIP, unsigned short nPort );
};