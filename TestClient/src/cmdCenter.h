#pragma once
#include <functional>
#include <cstring>
#include <vector>
#include <map>
#include "json/json.h"
#include "NativeTypes.h"
#include "mutex.h"
#ifndef REG_CMD
	#define  REG_CMD(strModule,strCmd,strArg,func) CCmdCenter::getInstance()->registerCmd(strModule,strCmd,strArg,(func))
#endif // !REG_CMD

#ifndef BEGIN_REG_CMD
	#define BEGIN_REG_CMD( uniqueNmae )  \
	struct uniqueNmae  {  \
	uniqueNmae() \
	{
#endif

#ifndef END_REG_CMD
	#define END_REG_CMD( uniqueNmae )  \
	} \
}; \
static uniqueNmae t ;
#endif

class CCmdCenter ;
class CClientRobot ;
struct stCmd 
{
	std::string strName ;
	std::string strArgDetail ;
	std::function<bool (std::vector<std::string>& vArg,CClientRobot* pCnter ) > lpFunc ;
	stCmd()
	{
		lpFunc = nullptr ;
		strName = "null";
		strArgDetail = "null" ;
	}
};


struct stBufferItem
{
	stCmd* pWillInvokecmd ;
	std::vector<std::string> vArg ;
	stBufferItem(stCmd* pcmd,std::vector<std::string>& vttArg )
	{
		pWillInvokecmd = pcmd ;
		vArg.clear();
		vArg.assign(vttArg.begin(),vttArg.end()) ;
	}

	~stBufferItem()
	{
		vArg.clear() ; 
		//delete pWillInvokecmd  ;
		pWillInvokecmd = nullptr ;
	}
};

class CCmdCenter
{
public:
	struct stCmdModule
	{
		std::string strModuleName ;
		std::map<std::string,stCmd*> vCmds ;
		
		stCmd* getCmdByName( std::string& strName )
		{
			auto iter = vCmds.find(strName);
			if ( iter != vCmds.end() )
			{
				return iter->second ;
			}
			return nullptr ;
		}

		~stCmdModule()
		{
			for ( auto& ref : vCmds )
			{
				delete ref.second ;
				ref.second = nullptr ;
			}
			vCmds.clear() ;
		}
	};
public:
	static CCmdCenter* getInstance();
protected:
	CCmdCenter(){ m_isRunnning = true ; }
public:
	~CCmdCenter() ;
	bool init();
	bool registerCmd(std::string strMoudle , stCmd* pcmd );
	bool registerCmd(std::string strMoudle , std::string strCmdName , std::string strArgDetail , std::function<bool (std::vector<std::string>& vArg,CClientRobot* pCnter ) > lpInvokeFunc  );
	void onGetUserInput(std::string strInput );
	stCmd* getCmdByName(std::string& strCmdName );
	void showModules();
	void showCmd( std::string& strModule , std::string& cmdName );
	void ShowModule( std::string& strModule  );
	void sendMsg(Json::Value& jsMsg, uint16_t nTargetPort );
	void sendMsg(char* pBuffer , uint16_t nBufferLen );
	void MainThreadUpdate(CClientRobot* p );
	void stop(){ m_isRunnning = false ;}
	bool isRunning(){ return m_isRunnning ;}
public:
	static CCmdCenter* s_gCmd ;
protected:
	std::map<std::string,stCmdModule*> m_vMoudles ;
	std::vector<stBufferItem*> m_vWillSendBuffer ;
	Mutex m_tBufferLock ;
	bool m_isRunnning ;
};