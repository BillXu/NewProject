//
//  VocerMnager_ios.cpp
//  MyPoker
//
//  Created by Xu BILL on 15/1/15.
//
//

#include "VoicerMnager_ios.h"
#import <AVFoundation/AVFoundation.h>
#include "amrFileCodec.h"
#include "SimpleAudioEngine.h"
AVAudioRecorder* m_pRecorder = NULL ;
NSString* pTempRecorderFileNmae = NULL ;
// impletation delegate
@interface RecordeDelegate : NSObject<AVAudioRecorderDelegate> {
    
}
- (void)audioRecorderDidFinishRecording:(AVAudioRecorder *)recorder successfully:(BOOL)flag;

/* if an error occurs while encoding it will be reported to the delegate. */
- (void)audioRecorderEncodeErrorDidOccur:(AVAudioRecorder *)recorder error:(NSError *)error;
@end


@implementation RecordeDelegate
- (void)audioRecorderDidFinishRecording:(AVAudioRecorder *)recorder successfully:(BOOL)flag
{
    if ( flag != YES )
    {
        return ;
    }
    
    // star to compress ;
    NSString* pDefaultName = [NSString stringWithUTF8String:CVoicerManager::GetInstance()->GetFileName()];
    
    NSString* pAmrFile = [NSString stringWithFormat:@"%@.amr",pDefaultName];
    EncodeWAVEFileToAMRFile([pTempRecorderFileNmae UTF8String],[pAmrFile UTF8String],1,16);
    if ( CVoicerManager::GetInstance()->GetDelegate() )
    {
        CVoicerManager::GetInstance()->GetDelegate()->OnFinishRecord([pAmrFile UTF8String], false) ;
    }
    CVoicerManager::GetInstance()->OnSystemStopRecording();
}

- (void)audioRecorderEncodeErrorDidOccur:(AVAudioRecorder *)recorder error:(NSError *)error
{
    if ( CVoicerManager::GetInstance()->GetDelegate() )
    {
        CVoicerManager::GetInstance()->GetDelegate()->OnFinishRecord(NULL, true) ;
    }
    CVoicerManager::GetInstance()->OnSystemStopRecording();
}
@end


RecordeDelegate* pRecordDelegate = NULL ;
CVoicerManager* CVoicerManager::GetInstance()
{
    static CVoicerMager_ios mgr ;
    return &mgr ;
}

int CVoicerMager_ios::StartRecord(const char* pFileName)
{
    if ( CVoicerManager::StartRecord(pFileName) )
        return 1 ;
    
    // start to recorder
    [[AVAudioSession sharedInstance] setCategory: AVAudioSessionCategoryPlayAndRecord error:nil];
    [[AVAudioSession sharedInstance] setActive:YES error:nil];
    
//    if (m_pRecorder)
//    {
//        [m_pRecorder deleteRecording];
//        [m_pRecorder release];
//        m_pRecorder = NULL ;
//        
//        // delete ?
        if ( pTempRecorderFileNmae && [[NSFileManager defaultManager] fileExistsAtPath:pTempRecorderFileNmae])
        {
            NSError* pError ;
            [[NSFileManager defaultManager] removeItemAtPath:pTempRecorderFileNmae error:&pError] ;
        }
//    }
    
    if ( m_pRecorder == NULL )
    {
        m_pRecorder = [AVAudioRecorder alloc] ;
        NSDictionary *recordSetting=[NSDictionary dictionaryWithObjectsAndKeys:
                                     [NSNumber numberWithFloat:8000],AVSampleRateKey,
                                     [NSNumber numberWithInt:kAudioFormatLinearPCM],AVFormatIDKey,
                                     [NSNumber numberWithInt:1],AVNumberOfChannelsKey,
                                     [NSNumber numberWithInt:16],AVLinearPCMBitDepthKey,
                                     [NSNumber numberWithBool:NO],AVLinearPCMIsBigEndianKey,
                                     [NSNumber numberWithBool:NO],AVLinearPCMIsFloatKey,
                                     nil];
        
        if ( pTempRecorderFileNmae == NULL )
        {
            std::string str = GetFileName();
            std::size_t tPos = str.find_last_of('/') ;
            std::string str2 = str.substr(0,tPos+1);
            pTempRecorderFileNmae = [NSString stringWithFormat:@"%stemp_record.wav",str2.c_str()];
            [pTempRecorderFileNmae retain];
        }
        
        NSError *error ;
        [m_pRecorder initWithURL:[NSURL URLWithString:pTempRecorderFileNmae] settings:recordSetting error:&error] ;
        [recordSetting release] ;
        
        m_pRecorder.meteringEnabled = YES ;
        [m_pRecorder peakPowerForChannel:0];
        if ( NULL == pRecordDelegate )
        {
            pRecordDelegate = [[RecordeDelegate alloc] init] ;
        }
        [m_pRecorder setDelegate:pRecordDelegate];
    }

    //[m_pRecorder recordForDuration:30];
    [m_pRecorder prepareToRecord];
    

    [m_pRecorder record];
    return 0 ;
}

void CVoicerMager_ios::StopRecord()
{
    CVoicerManager::StopRecord() ;
    m_tDuration = m_pRecorder.currentTime ;
    [m_pRecorder stop] ;
    //[//m_pRecorder setDelegate:NULL];
    
    m_pRecorder = NULL ;
}

float CVoicerMager_ios::GetWavMertering()
{
    if ( m_pRecorder && m_bRecording )
    {
         [m_pRecorder updateMeters];
         const double alpha=0.5 ;
         static double lowPassResults = 0 ;
         double peakPowerForChannel=pow(10, (0.05)*[m_pRecorder peakPowerForChannel:0]);
         lowPassResults=alpha*peakPowerForChannel+(1.0-alpha)*lowPassResults;
         return lowPassResults ;
    }
    return 0 ;
}

bool CVoicerMager_ios::PlayVoice(const char* pFileName)
{
    NSString* pFile = [NSString stringWithUTF8String:pFileName];
    NSString* pwavName = [pFile stringByReplacingOccurrencesOfString:@".amr" withString:@".wav"];
    BOOL bExist = [[NSFileManager defaultManager] fileExistsAtPath:pwavName];
//    if ( bExist )
//    {
//        NSError* pError ;
//        [[NSFileManager defaultManager] removeItemAtPath:pwavName error:&pError] ;
//        bExist = NO ;
//    }
    
    if ( bExist == NO )
    {
        DecodeAMRFileToWAVEFile(pFileName,[pwavName UTF8String]);
    }
    //CocosDenshion::SimpleAudioEngine::getInstance()->playEffect([pTempRecorderFileNmae UTF8String]);
    CocosDenshion::SimpleAudioEngine::getInstance()->playEffect([pwavName UTF8String]);
    return true ;
}
