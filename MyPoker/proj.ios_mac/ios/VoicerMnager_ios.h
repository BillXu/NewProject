//
//  VocerMnager_ios.h
//  MyPoker
//
//  Created by Xu BILL on 15/1/15.
//
//

#ifndef __MyPoker__VocerMnager_ios__
#define __MyPoker__VocerMnager_ios__

#include <stdio.h>
#include "VoicerManager.h"
class CVoicerMager_ios
:public CVoicerManager
{
public:
    virtual int StartRecord(const char* pFileName);
    virtual void StopRecord();
    virtual bool PlayVoice(const char* pFileName);
    virtual float GetWavMertering();
};

#endif /* defined(__MyPoker__VocerMnager_ios__) */
