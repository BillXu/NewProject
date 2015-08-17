//
//  chatLayer.cpp
//  MyGame
//
//  Created by Xu BILL on 15/7/29.
//
//

#include "chatLayer.h"
void CChatItemCell::refreshCellByChatItem(stChatItem* pitem )
{
    if ( m_pText == nullptr )
    {
        m_pText = Label::createWithTTF("text","fonts/Marker Felt.ttf",30,Size(Director::getInstance()->getWinSize().width,0));
        m_pText->setAnchorPoint(Vec2::ZERO);
        addChild(m_pText);
    }
    
    if ( pitem->strContent.size() == 0 )
    {
        setContentSize(Size::ZERO) ;
        m_pText->setVisible(false);
        return ;
    }
    m_pText->setVisible(true) ;
    // temp set ;
    m_pText->setString(String::createWithFormat("%s%s",pitem->strName.c_str(),pitem->strContent.c_str())->getCString()) ;
    setContentSize(m_pText->getContentSize()) ;
}

bool CChatLayer::init()
{
    Layer::init();
    m_vChatItems.clear();
    m_pTableView = TableView::create(this, Size(Director::getInstance()->getWinSize().width, Director::getInstance()->getWinSize().height * 0.5)) ;
    addChild(m_pTableView) ;
    //m_pTableView->setVerticalFillOrder(cocos2d::extension::TableView::VerticalFillOrder::TOP_DOWN);
    m_pTableView->setPosition(0, Director::getInstance()->getWinSize().height * 0.5);
    Sprite* ps = Sprite::create("image/login_button1.png");
    m_pEditBox = ui::EditBox::create(ps->getContentSize(),Scale9Sprite::create("image/login_button1.png"));
    addChild(m_pEditBox);
    m_pEditBox->setDelegate(this);
    //m_pEditBox->setPosition(Vec2(0.0, m_pTableView->getPosition().y - m_pEditBox->getContentSize().height ));
    m_pEditBox->setPosition(Vec2::ZERO);
    m_pEditBox->setAnchorPoint(Vec2::ZERO) ;
    
    stChatItem* pItem = new stChatItem ;
    pItem->strContent = "";
    pItem->strName = "";
    pushChatItem(pItem);  // insertIndext 0 的时候，会导致重影，所以默认防止一个空的，看不见的item；

	lpInputCallBack = nullptr ;
    return true ;
}

void CChatLayer::onExit()
{
	Layer::onExit() ;
	VEC_CHAT_ITEM::iterator iter = m_vChatItems.begin() ;
	for ( ; iter != m_vChatItems.end() ; ++iter )
	{
		delete *iter ;
		*iter = nullptr ;
	}
	m_vChatItems.clear() ;
}

Size CChatLayer::tableCellSizeForIndex(TableView *table, ssize_t idx)
{
    if ( idx >= m_vChatItems.size() )
    {
        CCLOG("error: idx >= m_vChatItems.size()");
        return Size::ZERO ;
    }
    m_tSizeCaculator.refreshCellByChatItem(m_vChatItems[idx]);
    return m_tSizeCaculator.getContentSize() ;
}

TableViewCell* CChatLayer::tableCellAtIndex(TableView *table, ssize_t idx)
{
    CChatItemCell* pCell = (CChatItemCell*)table->dequeueCell() ;
    if ( pCell == nullptr )
    {
        pCell = new CChatItemCell ;
        pCell->autorelease() ;
    }
    pCell->refreshCellByChatItem(m_vChatItems[idx]) ;
    return pCell ;
}

ssize_t CChatLayer::numberOfCellsInTableView(TableView *table)
{
    return m_vChatItems.size() ;
}

void CChatLayer::pushChatItem(stChatItem* pItem)
{
    m_vChatItems.push_front(pItem);
	m_pTableView->insertCellAtIndex(0) ;
	m_pTableView->setContentOffset(m_pTableView->maxContainerOffset());
}

void CChatLayer::editBoxReturn(ui::EditBox* editBox)
{
    if ( strlen(editBox->getText()) == 0  )
    {
        return ;
    }
    
	if ( lpInputCallBack )
	{
		lpInputCallBack(0,editBox->getText());
	}
	editBox->setText("");
}

void CChatLayer::editBoxEditingDidEnd(ui::EditBox* editBox)
{
    printf("invoked");
}