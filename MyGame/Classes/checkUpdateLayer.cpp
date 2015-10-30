#include "checkUpdateLayer.h"
Scene* CCheckUpdateLayer::scene()
{
	Scene* pscen = Scene::create() ;
	Layer* player = CCheckUpdateLayer::create() ;
	pscen->addChild(player);
	return pscen ;
}

bool CCheckUpdateLayer::init()
{
	Layer::init() ;

	std::string strWr = FileUtils::getInstance()->getWritablePath() ;
	AssetsManagerEx* pmg = AssetsManagerEx::create("project.manifest",strWr.c_str());
	pmg->retain() ;
	LabelTTF* ptxt = LabelTTF::create("","Arial",28);
	addChild(ptxt) ;
   Size ptw = Director::getInstance()->getWinSize();
	ptxt->setPosition(ptw*0.5) ;
	if ( !pmg->getLocalManifest()->isLoaded())
	{
		ptxt->setString("can not find local manifest file");
		return false;
	}

	EventListenerAssetsManagerEx* pEventListener = EventListenerAssetsManagerEx::create(pmg,CC_CALLBACK_1(CCheckUpdateLayer::updateCallBack,this));
	getEventDispatcher()->addEventListenerWithSceneGraphPriority(pEventListener,this) ;

	pmg->checkUpdate();

	m_pmg = pmg ;
	
	m_pText = ptxt ;
	return true ;
}

void CCheckUpdateLayer::updateCallBack(EventAssetsManagerEx* pEvent)
{
	const char* pText = nullptr ;
	switch (pEvent->getEventCode())
	{
	case EventAssetsManagerEx::EventCode::ERROR_NO_LOCAL_MANIFEST:
		{
			pText = "can not find local manifest"; 
		}
		break;
	case EventAssetsManagerEx::EventCode::ERROR_DOWNLOAD_MANIFEST:
		{
			pText = "download manifest error";
		}
		break;
	case EventAssetsManagerEx::EventCode::ERROR_PARSE_MANIFEST:
		{
			pText = "Pase manifest error";
		}
		break;
 
	case EventAssetsManagerEx::EventCode::NEW_VERSION_FOUND:
		{
			pText = "find new version should download";
			static int a = 0 ;
			if ( !a )
			{
				m_pmg->update(); ;
				a = 1 ;
			}
		}
		break;
	case EventAssetsManagerEx::EventCode::ALREADY_UP_TO_DATE:
		{
			pText = "already update so , do not update";
			onUpdateFinish();
		}
		break;
	case EventAssetsManagerEx::EventCode::UPDATE_PROGRESSION:
		{
			pText = "updating , should have progess tip" ;
		}
		break;
	case EventAssetsManagerEx::EventCode::ASSET_UPDATED:
		{
			pText = "one asset finish update";
			onUpdateFinish();
		}
		break;
	case EventAssetsManagerEx::EventCode::ERROR_UPDATING:
		{
			pText = "error occured while update";
		}
		break;
	case EventAssetsManagerEx::EventCode::UPDATE_FINISHED:
		{
			pText = "update finish ";
			onUpdateFinish();
		}
		break;
	case EventAssetsManagerEx::EventCode::UPDATE_FAILED:
		{
			pText = "update failed , will try ?";
		}
		break;
	case EventAssetsManagerEx::EventCode::ERROR_DECOMPRESS:
		{
			pText = "decompress";
		}
		break;
	default:
		pText = "default";
		break;
	}
	m_pText->setString(pText);
	std::string strMessage = pEvent->getMessage() ;
	std::string strAsset = pEvent->getAssetId() ;
	float fPe = pEvent->getPercent() ;
	float fPecByFile = pEvent->getPercentByFile();
	CCLOG("messag:%s, assetId = %s pers %.2f, byFile percent %0.2f",strMessage.c_str(),strAsset.c_str(),fPe,fPecByFile);
	CCLOG("text: %s",pText);
}

void CCheckUpdateLayer::onUpdateFinish()
{
	Sprite* ps = Sprite::create("LobbyScene/MainLobbyTipLayer_mv.png") ;
	addChild(ps) ;
	ps->setPosition(200,50) ;
}