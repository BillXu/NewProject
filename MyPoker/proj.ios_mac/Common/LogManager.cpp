//
//  LogManager.cpp
//  God
//
//  Created by Xu BILL on 12-10-25.
//
//
#pragma warning(disable:4996)
#include "LogManager.h"
#include <stdarg.h>
#include <time.h>
#ifdef POKER_CLIENT
#include <string>
#else
#include <string.h>
#include <Windows.h>
#endif
CLogMgr* CLogMgr::SharedLogMgr()
{
    static CLogMgr g_sLogMgr ;
    return &g_sLogMgr ;
}

CLogMgr::CLogMgr()
{
    pFile = NULL ;
    bOutPutToFile = false ;
    bEnable = true ;
}

CLogMgr::~CLogMgr()
{
    if ( pFile )
    {
        fclose(pFile) ;
    }
}

void CLogMgr::SetDisablePrint(bool bEnable)
{
    this->bEnable = bEnable ;
}

void CLogMgr::PrintLog(const char *sformate, ...)
{
#ifdef NDEBUG
	return ;
#endif
    va_list va ;
    va_start(va,sformate);
    Print(sformate, va,eLogState_Noraml);
    va_end(va) ;
}

void CLogMgr::ErrorLog(const char *sformate, ...)
{
	va_list va ;
	va_start(va,sformate);
	Print(sformate, va,eLogState_Error);
	va_end(va) ;
}

void CLogMgr::SystemLog(const char* sformate , ...)
{
	va_list va ;
	va_start(va,sformate);
	Print(sformate, va,eLogState_System);
	va_end(va) ;
}

void CLogMgr::SetOutputFile(const char *pFilename)
{
    pFile = fopen(pFilename, "w");
    if ( pFile == NULL )
    {
        bOutPutToFile = false ;
        return ;
    }
    bOutPutToFile = true ;
}

void CLogMgr::Print(const char *sFormate, va_list va , eLogState eSate )
{
    if ( bEnable == false )
        return ;
    
    static char pBuffer[1024] = { 0 } ;
    memset(pBuffer,0,sizeof(pBuffer));
	// time ;
	time_t t;
	time(&t);
	char* pstr = ctime(&t) ;
	*(pstr + strlen(pstr)) = 0 ;
	*(pstr + strlen(pstr)-1) = 0 ;
	// 	
    if ( eSate == eLogState_Error )
    {
        sprintf(pBuffer, "Error: [%s] %s\n",pstr,sFormate);
    }
    else if ( eSate == eLogState_Noraml )
    {
        sprintf(pBuffer, "Log: [%s] %s \n",pstr,sFormate);
    }
    else if ( eSate == eLogState_Warnning )
    {
        sprintf(pBuffer, "Warnning: [%s] %s \n",pstr,sFormate);
    }
	else if ( eSate == eLogState_System )
	{
		sprintf(pBuffer, "System: [%s] %s \n",pstr,sFormate);
	}
#ifndef POKER_CLIENT
	switch ( eSate )
	{
	case eLogState_Error:
		{
			SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE),FOREGROUND_INTENSITY |FOREGROUND_RED);
		}
		break;
	case eLogState_Noraml:
		{
			SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), FOREGROUND_RED|FOREGROUND_GREEN|FOREGROUND_BLUE);
		}
		break;
	case eLogState_System:
		{
			SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE),FOREGROUND_INTENSITY |FOREGROUND_GREEN);
		}
		break;
	case eLogState_Warnning:
		{
			SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE),FOREGROUND_INTENSITY |FOREGROUND_RED|FOREGROUND_GREEN);
		}
		break;
	default:
		{
			SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE),FOREGROUND_INTENSITY |FOREGROUND_RED);
		}
		break;
	}
#endif
	
    if ( bOutPutToFile && pFile )
    {
        vfprintf(pFile, pBuffer, va) ;
    }
    else
    {
        vprintf(pBuffer, va) ;
    }
}

void CLogMgr::CloseFile()
{
    if ( bOutPutToFile && pFile )
    {
        fclose(pFile) ;
        pFile = NULL ;
        bOutPutToFile = false ;
    }
}
