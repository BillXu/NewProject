#include "loginScene.h"
#include "MessageDefine.h"
#include "ClientApp.h"
cocos2d::Scene* CLoginScene::createLoginScene()
{
	auto scene = Scene::create();
	auto layer = CLoginScene::create() ;
	scene->addChild(layer);
	return scene ;
}

bool CLoginScene::init()
{
	IBaseScene::init() ;
	// as when MSG_PlAYER_OTHER_LOGIN event occured , other scene may disconnect from svr 
	// so here ,we will try to connect svr ;
	if ( CClientApp::getInstance()->isConnecting() == false )
	{
		CClientApp::getInstance()->connectToSvr() ;
	}
	// read cocostuido 
	Node* pRoot = CSLoader::getInstance()->createNodeWithFlatBuffersFile("res/login.csb");
	addChild(pRoot);
	auto cSize = pRoot->getContentSize();
	pRoot->setContentSize(Director::getInstance()->getWinSize()) ;
	ui::Helper::doLayout(pRoot);

	auto bg = pRoot->getChildByName("signup_bg_2") ;
	m_pAccount = dynamic_cast<ui::EditBox*>(bg->getChildByName("account"));
	m_pName = dynamic_cast<ui::EditBox*>(bg->getChildByName("name"));
	m_pPassword = dynamic_cast<ui::EditBox*>(bg->getChildByName("password"));

	auto Btn = dynamic_cast<ui::Button*>(bg->getChildByName("btnLogin")) ;    
	Btn->setPressedActionEnabled(true);
	Btn->addClickEventListener([=](Ref* sender){
			// on login pressed
			if ( strlen(m_pPassword->getText()) == 0 || 0 == strlen(m_pAccount->getText()) )
			{
				cocos2d::MessageBox("account or password is null","error") ;
				return ;
			}
			else
			{
				stMsgLogin msgLogin ;
				memset(msgLogin.cAccount,0,sizeof(msgLogin.cAccount)) ;
				sprintf_s(msgLogin.cAccount,sizeof(msgLogin.cAccount),"%s",m_pAccount->getText()) ;

				memset(msgLogin.cPassword,0,sizeof(msgLogin.cPassword)) ;
				sprintf_s(msgLogin.cPassword,sizeof(msgLogin.cPassword),"%s",m_pPassword->getText()) ;
				sendMsg(&msgLogin,sizeof(msgLogin)) ;
			}

	       });

	Btn = dynamic_cast<ui::Button*>(bg->getChildByName("btnRegister")) ;    
	Btn->setPressedActionEnabled(true);
	Btn->addClickEventListener([=](Ref* sender){
		// on register pressed
		if ( strlen(m_pPassword->getText()) == 0 || 0 == strlen(m_pAccount->getText()) )
		{
			cocos2d::MessageBox("account or password is null","error") ;
			return ;
		}
		else
		{
			stMsgRegister msgRegister ;
			msgRegister.cRegisterType = 1 ;
			msgRegister.nChannel = 0 ;
			memset(msgRegister.cAccount,0,sizeof(msgRegister.cAccount)) ;
			sprintf_s(msgRegister.cAccount,sizeof(msgRegister.cAccount),"%s",m_pAccount->getText()) ;

			memset(msgRegister.cPassword,0,sizeof(msgRegister.cPassword)) ;
			sprintf_s(msgRegister.cPassword,sizeof(msgRegister.cPassword),"%s",m_pPassword->getText()) ;
			sendMsg(&msgRegister,sizeof(msgRegister)) ;
		}
	});
	return true ;
}

bool CLoginScene::onMsg(stMsg* pmsg )
{
	switch ( pmsg->usMsgType )
	{
	case MSG_PLAYER_LOGIN:
		{
			stMsgLoginRet* pRet = (stMsgLoginRet*)pmsg ;
			const char* pError = nullptr ;
			if ( pRet->nRet == 0 )
			{

			}
			else if ( 1 == pRet->nRet )
			{
				pError = "account error ";
			}
			else if ( 2 == pRet->nRet )
			{
				pError = "password error" ;
			}
			else if ( 3 == pRet->nRet )
			{
				pError = "state error" ;
			}
			else
			{
				pError = "unknown error";
			}

			if ( pError )
			{
				cocos2d::MessageBox(pError,"Warnning");
			}
		}
		break;
	case MSG_PLAYER_REGISTER:
		{
			stMsgRegisterRet* pRet = (stMsgRegisterRet*)pmsg ;
			if ( pRet->nRet == 1 )
			{
				cocos2d::MessageBox("account already exsit, please try another one","Warnning");
				break;;
			}
			else
			{
				printf("register success\n");
			}
		}
		break;
	case MSG_PLAYER_BASE_DATA:
		{
			printf("recieved base data\n");
		}
		break;
	default:
		return false;
	}
	return true ;
}

void CLoginScene::editBoxReturn(ui::EditBox* editBox)
{

}