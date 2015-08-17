//
//  chatLayer.h
//  MyGame
//
//  Created by Xu BILL on 15/7/29.
//
//

#ifndef __MyGame__chatLayer__
#define __MyGame__chatLayer__

#include <stdio.h>
#include "cocos2d.h"
#include "cocos-ext.h"
#include <deque>
USING_NS_CC ;
USING_NS_CC_EXT ;
struct stChatItem
{
    std::string strName ;
    uint8_t nContentType ;
    std::string strContent ;
};

class CChatItemCell
:public TableViewCell
{
public:
    CChatItemCell(){ m_pText = nullptr; }
    void refreshCellByChatItem(stChatItem* pitem );
protected:
    Label* m_pText;
};

class CChatLayer
:public Layer
,public TableViewDataSource
,public ui::EditBoxDelegate
{
public:
    typedef std::deque<stChatItem*> VEC_CHAT_ITEM;
	typedef std::function<void(int contentType,const char* pContent)> lpfuncCallBack ;
public:
    CREATE_FUNC(CChatLayer);
    bool init();
	void onExit();
    virtual Size tableCellSizeForIndex(TableView *table, ssize_t idx);
    TableViewCell* tableCellAtIndex(TableView *table, ssize_t idx);
    ssize_t numberOfCellsInTableView(TableView *table);
    void pushChatItem(stChatItem* pItem);
    void editBoxReturn(ui::EditBox* editBox) ;
    void editBoxEditingDidEnd(ui::EditBox* editBox);
    void requestFocust(){ m_pEditBox->requestFocus() ; m_pEditBox->touchDownAction(nullptr,ui::Widget::TouchEventType::ENDED);}
	void setCallBack(lpfuncCallBack callBack ){ lpInputCallBack = callBack ; };
protected:
    TableView* m_pTableView ;
    VEC_CHAT_ITEM m_vChatItems ;
    CChatItemCell m_tSizeCaculator ;
    ui::EditBox*  m_pEditBox ;
	lpfuncCallBack lpInputCallBack ;
};

#endif /* defined(__MyGame__chatLayer__) */
