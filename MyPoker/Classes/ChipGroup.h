//
//  ChipGroup.h
//  MyPoker
//
//  Created by Xu BILL on 14/12/4.
//
//

#ifndef __MyPoker__ChipGroup__
#define __MyPoker__ChipGroup__

#include <stdio.h>
#include "cocos2d.h"
#include <vector>
USING_NS_CC ;
class CChipGroup
:public Node
{
public:
    enum eChipMoveType
    {
        eChipMove_Group2None,  // 从一堆筹码 移动到某一个位置 消失，
        eChipMove_Group2Group, // 从一堆筹码，移动到 另一堆筹码
        eChipMove_Origin2None, // 一堆筹码，原地逐步消失
        eChipMove_Origin2Group, // 一堆筹码，原地累加
        eChipMove_None2Group, // 从无到有移动，到一堆筹码
    };
    typedef std::vector<Sprite*> VEC_SPRITE;
public:
    static CChipGroup* CreateGroup();
    bool init();
    void SetOriginPosition(Point ptStart );
    void SetDestPosition(Point ptEnd );
    void SetFinishCallBack(std::function<void (CChipGroup*)> pFunc);
    void Start(eChipMoveType eType , float fAniTime );
    void SetGroupCoin( uint64_t nAllCoin , bool bVisible = false );
    uint64_t GetGroupCoin();
protected:
    Point m_ptDestPos, m_ptOrigPos ;
    uint64_t m_nCoin;
    VEC_SPRITE m_vAllSprites ;
    std::function<void (CChipGroup*)> m_funAniFinish ;
};
#endif /* defined(__MyPoker__ChipGroup__) */
