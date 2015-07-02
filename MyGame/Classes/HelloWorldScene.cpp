#include "HelloWorldScene.h"
#include "cocostudio/CocoStudio.h"
#include "ui/CocosGUI.h"

USING_NS_CC;

CNetWorkMgr* g_pNetWork = nullptr ;
Scene* HelloWorld::createScene()
{
    // 'scene' is an autorelease object
    auto scene = Scene::create();
    
    // 'layer' is an autorelease object
    auto layer = HelloWorld::create();

    // add layer as a child to scene
    scene->addChild(layer);

    // return the scene
    return scene;
}

// on "init" you need to initialize your instance
bool HelloWorld::init()
{
    //////////////////////////////
    // 1. super init first
    if ( !Layer::init() )
    {
        return false;
    }
    
    Size visibleSize = Director::getInstance()->getVisibleSize();
    Vec2 origin = Director::getInstance()->getVisibleOrigin();

    /////////////////////////////
    // 2. add a menu item with "X" image, which is clicked to quit the program
    //    you may modify it.

    // add a "close" icon to exit the progress. it's an autorelease object
    auto closeItem = MenuItemImage::create(
                                           "CloseNormal.png",
                                           "CloseSelected.png",
                                           CC_CALLBACK_1(HelloWorld::menuCloseCallback, this));
    
	closeItem->setPosition(Vec2(origin.x + visibleSize.width - closeItem->getContentSize().width/2 ,
                                origin.y + closeItem->getContentSize().height/2));

    // create menu, it's an autorelease object
    auto menu = Menu::create(closeItem, NULL);
    menu->setPosition(Vec2::ZERO);
    this->addChild(menu, 1);

    /////////////////////////////
    // 3. add your codes below...

    // add a label shows "Hello World"
    // create and initialize a label
    
    auto label = Label::createWithTTF("Hello World", "fonts/Marker Felt.ttf", 24);
    
    // position the label on the center of the screen
    label->setPosition(Vec2(origin.x + visibleSize.width/2,
                            origin.y + visibleSize.height - label->getContentSize().height));

    // add the label as a child to this layer
    this->addChild(label, 1);

    // add "HelloWorld" splash screen"
    auto sprite = Sprite::create("HelloWorld.png");

    // position the sprite on the center of the screen
   // sprite->setPosition(Vec2(visibleSize.width/2 + origin.x, visibleSize.height/2 + origin.y));

    // add the sprite as a child to this layer 133 1139 1521
    //this->addChild(sprite, 0);
    

	// read cocostuido 
	Node* pRoot = CSLoader::getInstance()->createNodeWithFlatBuffersFile("res/Game.csb");
	addChild(pRoot,1);
	auto cSize = pRoot->getContentSize();
	pRoot->setContentSize(Director::getInstance()->getWinSize()) ;
	ui::Helper::doLayout(pRoot);

	cocostudio::timeline::ActionTimeline *action = CSLoader::getInstance()->createTimeline("res/PlayerSelf.csb"); 
	////action->retain();
	pRoot->getChildByName("self")->runAction(action);
	action->play("showBtn",true);
	return true;
	auto bg = pRoot->getChildByName("signup_bg_2") ;
	auto winSize = Director::getInstance()->getWinSize();
	auto pos = bg->getPosition();
	auto Btn = dynamic_cast<ui::Button*>(pRoot->getChildByName("signup_bg_2")->getChildByName("btnLogin")) ;    
	Btn->setPressedActionEnabled(true);
	//Btn->addClickEventListener([=](Ref* sender){
 //           CCLOG("Button clicked, position = (%f, %f)", Btn->getPosition().x,
 //                 Btn->getPosition().y);
 //           //cocostudio::ActionManagerEx::getInstance()->playActionByName("res/MainScene.csb","animation0");
	//		action->play("animation0",true);
 //       });
	
	if ( g_pNetWork == nullptr )
	{
		g_pNetWork = new CNetWorkMgr ;
		g_pNetWork->SetupNetwork(1);
		g_pNetWork->ConnectToServer("203.186.75.136",50001);
		g_pNetWork->AddMessageDelegate(this);
	}
	getScheduler()->schedule([=](float fTime){ g_pNetWork->ReciveMessage();},this,0,false,"recievemsg");
    return true;
}


void HelloWorld::menuCloseCallback(Ref* pSender)
{
#if (CC_TARGET_PLATFORM == CC_PLATFORM_WP8) || (CC_TARGET_PLATFORM == CC_PLATFORM_WINRT)
	MessageBox("You pressed the close button. Windows Store Apps do not implement a close button.","Alert");
    return;
#endif

    Director::getInstance()->end();

#if (CC_TARGET_PLATFORM == CC_PLATFORM_IOS)
    exit(0);
#endif
}

bool HelloWorld::OnConnectStateChanged( eConnectState eSate, Packet* pMsg)
{
	if ( eSate == eConnect_Accepted )
	{
		printf("ok\n");
	}
	return true ;
}

bool HelloWorld::OnMessage( Packet* pMsg )
{
	return true ;
}
