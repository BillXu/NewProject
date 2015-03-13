#pragma once
#include "header.h"
class INetwork ;
class CSession 
	:public ITcpSocketCallback
{
public:
	CSession();
	virtual ~CSession();
	void InitSocketForServer(INetwork *proc, ITcpSocket *s, CONNECT_ID nConnectID);
	bool InitSocketForClient(INetwork *proc, zsummer::network::IIOServer* pIO,const char* pip, unsigned short nPort, CONNECT_ID nConnectID);
	virtual bool OnRecv(unsigned int n);
	virtual bool OnConnect(bool bConnected);
	virtual bool OnSend(unsigned int nSentLen);
	virtual bool OnClose();
	void Send(char * buf, unsigned int len);
	char* GetIPString(){ return m_pIPString ; }
	void Close();
	bool CheckHeatbet();
protected:
	INetwork  * m_pNetwork;
	ITcpSocket * m_socket;
	bool m_bSeverMode ;

	//! 每个消息包分两次分别读取头部和包体
	unsigned char m_type;
	//! 读包
	Packet m_recving;
	unsigned short m_curRecvLen;
	//! 写包队列
	std::queue<Packet *> m_sendque;

	//! 当前写包
	Packet m_sending;
	unsigned short m_curSendLen;

	CONNECT_ID m_nConnectID ;

	time_t m_nHeatBetTimeOut ;
	bool m_bWaitDelete ;

	time_t m_nTimeSendBet;

	char m_pIPString[32];
};