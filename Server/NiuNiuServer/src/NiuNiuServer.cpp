#include "NiuNiuServer.h"
#include "MessageDefine.h"
uint16_t CNiuNiuServerApp::getLocalSvrMsgPortType()
{
	return ID_MSG_PORT_NIU_NIU ;
}

bool CNiuNiuServerApp::onLogicMsg( stMsg* prealMsg , eMsgPort eSenderPort , uint32_t nSessionID )
{
	return false ;
}