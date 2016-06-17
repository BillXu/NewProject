#include "commonCmdScene.h"
#include "MessageDefine.h"
#include "LogManager.h"
//#include "ClientRobot.h"
#include "cmdCenter.h"
#include "Client.h"
#define MODULE_COMMON "common"

BEGIN_REG_CMD(hem)

REG_CMD( MODULE_COMMON,"login","[account] [pwd]",[](std::vector<std::string>& vArg,CClientRobot* pCnter)
{
	if ( vArg.size() != 2 )
	{
		return false ;
	}

	stMsgLogin msg ;
	memset(msg.cAccount,0,sizeof(msg.cAccount)) ;
	strncpy(msg.cAccount,vArg[0].c_str(),vArg[0].size()) ;
	memset(msg.cPassword,0,sizeof(msg.cPassword)) ;
	strncpy(msg.cPassword,vArg[1].c_str(),vArg[1].size()) ;
	pCnter->sendMsg(&msg,sizeof(msg)) ;
	return true ;
});




END_REG_CMD(hem)


bool commonCmdScene::onMessage(stMsg* pmsg)
{
	switch (pmsg->usMsgType)
	{
	case MSG_PLAYER_LOGIN:
		{
			stMsgLoginRet* pRetMsg = (stMsgLoginRet*)pmsg;
			// 0 ; success ; 1 account error , 2 password error ;
			const char* pString = NULL ;
			if ( pRetMsg->nRet == 0 )
			{
				pString = "check Account Success !" ;
			}
			else if ( 1 == pRetMsg->nRet )
			{
				pString = "Check Account Error : Account Error !" ;
			}
			else if ( 2 == pRetMsg->nRet )
			{
				pString = "Check Account Error : password Error !" ;
			}
			else
			{
				pString = "Check account Error : unknown Error !" ;
			}

			CLogMgr::SharedLogMgr()->SystemLog("%s ret = %d, account type = %d ",pString,pRetMsg->nRet,pRetMsg->nAccountType ) ;
			return true ;
		}
		break;
	case MSG_PLAYER_BASE_DATA:
		{
			stMsgPlayerBaseData* pRet = (stMsgPlayerBaseData*)pmsg;
			CLogMgr::SharedLogMgr()->PrintLog("received base data : ");
			CLogMgr::SharedLogMgr()->PrintLog("name : %s",pRet->stBaseData.cName) ;
			CLogMgr::SharedLogMgr()->PrintLog("uid : %u",pRet->stBaseData.nUserUID) ;
			CLogMgr::SharedLogMgr()->PrintLog("coin : %u ",pRet->stBaseData.nCoin) ;
			CLogMgr::SharedLogMgr()->PrintLog("diamond: %u" ,pRet->stBaseData.nDiamoned) ;
			CLogMgr::SharedLogMgr()->PrintLog("sessionID: %u" ,pRet->nSessionID) ;
		}
		break; ;
	default:
		return false;
	}
	return true ;
}

bool commonCmdScene::onMessage(Json::Value& jsContent )
{
	return false ;
}