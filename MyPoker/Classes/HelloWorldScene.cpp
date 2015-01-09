#include "HelloWorldScene.h"
#include "ChipGroup.h"
USING_NS_CC;

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
    
    auto label = LabelTTF::create("Hello World", "Arial", 24);
    
    // position the label on the center of the screen
    label->setPosition(Vec2(origin.x + visibleSize.width/2,
                            origin.y + visibleSize.height - label->getContentSize().height));

    // add the label as a child to this layer
    this->addChild(label, 1);

    // add "HelloWorld" splash screen"
    auto sprite = Sprite::create("HelloWorld.png");

    // position the sprite on the center of the screen
    sprite->setPosition(Vec2(visibleSize.width/2 + origin.x, visibleSize.height/2 + origin.y));
    sprite->setScale(2);

    // add the sprite as a child to this layer
    this->addChild(sprite, 0);
    m_pChipGroup = nullptr ;
    return true;
}


void HelloWorld::menuCloseCallback(Ref* pSender)
{
    if ( m_pChipGroup )
    {
        m_pChipGroup->removeFromParent();
        m_pChipGroup = nullptr ;
    }
    m_pChipGroup = CChipGroup::CreateGroup();
    addChild(m_pChipGroup);
    m_pChipGroup->setPosition(Point(330,330));
    //m_pChipGroup->SetDestPosition(Point(568,320));
    //m_pChipGroup->SetDestPosition(Point::ZERO);
    //m_pChipGroup->SetDestPosition(Point(130,130));
    m_pChipGroup->SetGroupCoin(10,true);
    static std::function<void(CChipGroup*p)> f = [](CChipGroup*p){
        CCLOG("finish");
        p->SetFinishCallBack(nullptr);
        //p->SetDestPosition(Point(130,130));
        p->SetGroupCoin(5);
        p->Start(CChipGroup::eChipMoveType::eChipMove_Origin2Group, 0);
    } ;
    m_pChipGroup->SetFinishCallBack(f);
    m_pChipGroup->Start(CChipGroup::eChipMoveType::eChipMove_Origin2None, 0);
    return ;
#if (CC_TARGET_PLATFORM == CC_PLATFORM_WP8) || (CC_TARGET_PLATFORM == CC_PLATFORM_WINRT)
	MessageBox("You pressed the close button. Windows Store Apps do not implement a close button.","Alert");
    return;
#endif

    Director::getInstance()->end();

#if (CC_TARGET_PLATFORM == CC_PLATFORM_IOS)
    exit(0);
#endif
}
