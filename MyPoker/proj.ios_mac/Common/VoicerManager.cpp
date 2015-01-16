//
//  VoicerManager.cpp
//  MyPoker
//
//  Created by Xu BILL on 15/1/15.
//
//

#include "VoicerManager.h"
CVoicerManager::CVoicerManager()
{
    m_pDelegate = NULL ;
    m_strFileNameWithoutExt = "";
    m_bRecording = false ;
    m_tDuration = 0 ;
}

CVoicerManager::~CVoicerManager()
{

}

int CVoicerManager::StartRecord(const char* pFileName)
{
    if ( m_bRecording )
    {
        printf("already recording, please try later \n") ;
        return 1 ;
    }
    
    if ( pFileName == nullptr )
    {
        printf("ERROR: RECORD FILE NAME IS NULL %s\n",__FUNCTION__) ;
    }
    else
    {
        m_strFileNameWithoutExt = pFileName ;
        size_t nPos = m_strFileNameWithoutExt.find_last_of('.') ;
        if ( std::string::npos != nPos )
        {
            m_strFileNameWithoutExt = m_strFileNameWithoutExt.substr(0,nPos);
        }
    }
    m_tStartTime = time(NULL) ;
    m_bRecording = true ;
    return 0;
}

void CVoicerManager::StopRecord()
{
    m_bRecording = false ;
    m_tDuration = time(NULL) - m_tStartTime ;
}

bool CVoicerManager::PlayVoice(const char* pFileName)
{
    if ( pFileName == NULL )
    {
        return false ;
    }
    return true;
}

CVoiceRecordDelegate* CVoicerManager::GetDelegate()
{
    return m_pDelegate ;
}

void CVoicerManager::SetDelegate(CVoiceRecordDelegate* pDelegate)
{
    m_pDelegate = pDelegate ;
}

const char* CVoicerManager::GetFileName()
{
    return m_strFileNameWithoutExt.c_str();
}