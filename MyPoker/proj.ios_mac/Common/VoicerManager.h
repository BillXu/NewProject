//
//  VoicerManager.h
//  MyPoker
//
//  Created by Xu BILL on 15/1/15.
//
//

#ifndef __MyPoker__VoicerManager__
#define __MyPoker__VoicerManager__

#include <stdio.h>
#include <string>
class CVoiceRecordDelegate
{
public:
    virtual ~CVoiceRecordDelegate(){}
    virtual void OnFinishRecord(const char* pFileName, bool bInterupted ) = 0;
};

class CVoicerManager
{
public:
    static CVoicerManager* GetInstance();
protected:
    CVoicerManager();
    virtual ~CVoicerManager();
public:
    virtual int StartRecord(const char* pFileName);
    virtual void StopRecord();
    virtual bool PlayVoice(const char* pFileName);
    virtual float GetWavMertering(){return 0 ;}
    CVoiceRecordDelegate* GetDelegate();
    void SetDelegate(CVoiceRecordDelegate* Delegate);
    const char* GetFileName();
    time_t GetRecordDuration(){ if (m_bRecording )return 0; return m_tDuration ; }; // return by second ;
    void OnSystemStopRecording(){ m_bRecording = false ;}
protected:
    CVoiceRecordDelegate* m_pDelegate ;
    std::string m_strFileNameWithoutExt ;
    time_t m_tStartTime ;
    bool m_bRecording ;
    time_t m_tDuration ;  // by second ;
    
};
#endif /* defined(__MyPoker__VoicerManager__) */
