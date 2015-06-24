#ifndef __HELLOWORLD_SCENE_H__
#define __HELLOWORLD_SCENE_H__

#include "cocos2d.h"
class CChipGroup ;
//#include "VoicerManager.h"
<<<<<<< HEAD
class HelloWorld : public cocos2d::Layer/*,public CVoiceRecordDelegate*/
=======
class HelloWorld : public cocos2d::Layer//,public CVoiceRecordDelegate
>>>>>>> c04460d1fd07c4e5f8db649853fcc976fc7b9142
{
public:
    // there's no 'id' in cpp, so we recommend returning the class instance pointer
    static cocos2d::Scene* createScene();

    // Here's a difference. Method 'init' in cocos2d-x returns bool, instead of returning 'id' in cocos2d-iphone
    virtual bool init();  
    
    // a selector callback
    void OnFinishRecord(const char* pFileName, bool bInterupted ) ;
    void menuCloseCallback(cocos2d::Ref* pSender);
    
    // implement the "static create()" method manually
    CREATE_FUNC(HelloWorld);
    
    CChipGroup* m_pChipGroup ;
};

#endif // __HELLOWORLD_SCENE_H__
