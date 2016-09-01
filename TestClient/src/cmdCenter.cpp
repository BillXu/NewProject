#include "cmdCenter.h"
#include "log4z.h"
#include "MessageDefine.h"
#include "Client.h"
#include "NetWorkManager.h"
#include <cassert>
CCmdCenter::~CCmdCenter() 
{
	for ( auto & ref : m_vMoudles )
	{
		delete ref.second ;
	}
	m_vMoudles.clear() ;
}

CCmdCenter* CCmdCenter::s_gCmd = nullptr;

CCmdCenter* CCmdCenter::getInstance() 
{
	if ( s_gCmd == nullptr )
	{
		s_gCmd = new CCmdCenter ;
	}
	return s_gCmd ;
}

bool CCmdCenter::init()
{



	//if ( s_gCmd == nullptr )
	//{
	//	s_gCmd = this ;
	//}
	//else
	//{
	//	LOGFMTE("can not create one more cmd center") ;
	//	assert(0 && "can not create more than one ccmd center");
	//	return false;
	//}

	//registerCmd(MODULE_COMMON,"login","[account] [pwd]",[](std::vector<std::string>& vArg,CClientRobot* pCnter)
	//{
	//	if ( vArg.size() != 2 )
	//	{
	//		return false ;
	//	}

	//	stMsgLogin msg ;
	//	memset(msg.cAccount,0,sizeof(msg.cAccount)) ;
	//	strncpy(msg.cAccount,vArg[0].c_str(),vArg[0].size()) ;
	//	memset(msg.cPassword,0,sizeof(msg.cPassword)) ;
	//	strncpy(msg.cPassword,vArg[1].c_str(),vArg[1].size()) ;
	//	pCnter->sendMsg(&msg,sizeof(msg)) ;
	//	return true ;
	//}) ;

	return true ;
}

bool CCmdCenter::registerCmd(std::string strMoudle , stCmd* pcmd )
{
	auto iter = m_vMoudles.find(strMoudle) ;
	stCmdModule* pmodule = nullptr ;
	if ( iter == m_vMoudles.end() )
	{
		pmodule = new stCmdModule ;
		pmodule->strModuleName = strMoudle ;
		m_vMoudles[pmodule->strModuleName] = pmodule ;
	}
	else
	{
		pmodule = iter->second ;
	}

	if ( pmodule->getCmdByName(pcmd->strName) )
	{
		LOGFMTE("already added cmd = %s , to module = %s",pcmd->strName.c_str(),pmodule->strModuleName.c_str()) ;
		return false ;
	}
	pmodule->vCmds.insert(std::make_pair(pcmd->strName,pcmd)) ;
	return true ;
}

bool CCmdCenter::registerCmd(std::string strMoudle , std::string strCmdName , std::string strArgDetail , std::function<bool (std::vector<std::string>& vArg,CClientRobot* pCnter ) > lpInvokeFunc  )
{
	stCmd* p = new stCmd ;
	p->strName = strCmdName;
	p->strArgDetail = strArgDetail ;
	p->lpFunc = lpInvokeFunc ;

	if ( registerCmd(strMoudle,p) == false )
	{
		delete p ;
		p = nullptr ;
		return false ;
	}
	return true ;
}

void CCmdCenter::onGetUserInput(std::string strInput )
{
	std::vector<std::string> vecSplit ;
	std::string strCur ;
	for ( char n : strInput )
	{
		if ( n != ' ' && n != '\t' )
		{
			strCur.push_back(n);
		}
		else
		{
			if ( strCur.empty() == false )
			{
				vecSplit.push_back(strCur) ;
			}
			strCur.clear() ;
		}
	}

	if ( strCur.empty() == false )
	{
		vecSplit.push_back(strCur) ;
	}
	strCur.clear() ;

	if ( vecSplit.empty() )
	{
		printf("parse input error , str = %s\n",strInput.c_str() ) ;
		return ;
	}

	// if is help commond 
	if ( "help" == vecSplit[0] )
	{
		vecSplit.erase(vecSplit.begin()) ;
		if ( vecSplit.empty() )
		{
			showModules();
			return;
		}

		if ( vecSplit.size() == 1 )
		{
			ShowModule(vecSplit[0]) ;
			return;
		}

		showCmd(vecSplit[0],vecSplit[1]) ;
		return ;
	}

	auto pcmd = getCmdByName(vecSplit[0]);
	if ( pcmd == nullptr )
	{
		printf("can not find cmd name = %s\n",vecSplit[0].c_str()) ;
		return ;
	}

	vecSplit.erase(vecSplit.begin()) ;

	stBufferItem * p = new stBufferItem(pcmd,vecSplit);
	m_tBufferLock.Lock() ;
	m_vWillSendBuffer.push_back(p) ;
	m_tBufferLock.Unlock() ;
}

stCmd* CCmdCenter::getCmdByName(std::string& strCmdName )
{
	stCmd* pm = nullptr ;
	for ( auto refIter : m_vMoudles )
	{
		pm = refIter.second->getCmdByName(strCmdName);
		if ( pm )
		{
			return pm ;
		}
	}
	return nullptr ;
}

void CCmdCenter::showModules()
{
	printf("\nmodules: \n");
	for ( auto ref : m_vMoudles )
	{
		printf("\t %s\n",ref.first.c_str()) ;
	}
	printf("\n");
}

void CCmdCenter::showCmd( std::string& strModule , std::string& cmdName )
{
	stCmd* pcmd = nullptr ;
	auto iter = m_vMoudles.find(strModule) ;
	if ( iter != m_vMoudles.end() )
	{
		pcmd = iter->second->getCmdByName(cmdName);
	}

	if ( pcmd == nullptr )
	{
		pcmd = getCmdByName(cmdName) ;
	}

	if ( nullptr == pcmd )
	{
		LOGFMTE("can not find cmd with name = %s ",cmdName.c_str()) ;
		return ;
	}
	printf("\ncmd : %s\n",cmdName.c_str());
	printf("arg : %s \n",pcmd->strArgDetail.c_str()) ;
}

void CCmdCenter::ShowModule( std::string& strModule  )
{
	stCmdModule* pModule = nullptr ;
	auto iter = m_vMoudles.find(strModule) ;
	if ( iter != m_vMoudles.end() )
	{
		pModule = iter->second ;
	}
	else
	{
		LOGFMTD("can not find module = %s",strModule.c_str()) ;
		return ;
	}

	printf("module %s cmds : \n",strModule.c_str()) ;
	for ( auto& ref : pModule->vCmds )
	{
		printf(": %s \n",ref.first.c_str()) ;
	}
	printf("\n");
}

void CCmdCenter::MainThreadUpdate(CClientRobot* p )
{
	m_tBufferLock.Lock() ;
	if ( m_vWillSendBuffer.empty() )
	{
		m_tBufferLock.Unlock() ;
		return ;
	}

	auto pp = m_vWillSendBuffer.front();
	m_vWillSendBuffer.erase(m_vWillSendBuffer.begin()) ;
	m_tBufferLock.Unlock() ;

	auto pcmdItem = pp ;
	bool b = pcmdItem->pWillInvokecmd->lpFunc(pcmdItem->vArg,p);
	if ( b == false )
	{
		printf("arg error : cmd = %s arg : %s \n" , pcmdItem->pWillInvokecmd->strName.c_str(),pcmdItem->pWillInvokecmd->strArgDetail.c_str());
	}

	delete pp ;
	pp = nullptr ;
}