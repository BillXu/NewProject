#include "RoomConfig.h"
#include "CommonDefine.h"
#include "LogManager.h"
#include <assert.h>

void stSpeedRoomConfigs::AddConfig(stTaxasRoomConfig* pConfig)
{
	VEC_BLIND_ROOM_CONFIG* pVec ;
	if ( pConfig->nMaxSeat == 5 )
	{
		pVec = &m_vPlayerCountRoom[eSeatCount_5] ;
	}
	else
	{
		pVec = &m_vPlayerCountRoom[eSeatCount_9];
	}

	for ( uint32_t i = 0; i < pVec->size(); ++i )
	{
		if ( pConfig->nBigBlind == (*pVec)[i]->nBigBlind )
		{
			return ;
		}
	}
	pVec->push_back(pConfig) ;
}

void stSpeedRoomConfigs::Reset()
{
	for ( int i = 0 ; i < eSeatCount_Max ; ++i )
	{
		m_vPlayerCountRoom[i].clear();
	}
}

bool CRoomConfigMgr::OnPaser(CReaderRow& refReaderRow )
{
	unsigned char cType = refReaderRow["GameType"]->IntValue() ;
	stBaseRoomConfig* pRoomConfig = NULL ;
	switch ( cType )
	{
	case eRoom_TexasPoker:
	case eRoom_TexasPoker_Diamoned:
		{
			stTaxasRoomConfig* pConfig = new stTaxasRoomConfig ;
			pConfig->nBigBlind = refReaderRow["BigBlind"]->IntValue();
			pConfig->nMaxTakeInCoin = refReaderRow["MaxCoin"]->IntValue() ;
			pConfig->nRentFeeOneMonth = 20;
			pConfig->nDeskFee = 0 ;
			pRoomConfig = pConfig ;
		}
		break;
	case eRoom_Gold:
		{
			stGoldenRoomConfig* pConfig = new stGoldenRoomConfig ;
			pConfig->bCanDoublePK = refReaderRow["CanDoublePK"]->IntValue();
			pConfig->nChangeCardRound = refReaderRow["ChangeCardRound"]->IntValue();
			pConfig->nMiniBet = refReaderRow["MiniBet"]->IntValue();
			pConfig->nTitleNeedToEnter = refReaderRow["TitleNeedToEnter"]->IntValue();
#ifndef GAME_SERVER
			char pBuffer[256] = {0};
			for ( int i = 0 ; i < GOLDEN_ROOM_COIN_LEVEL_CNT ; ++i )
			{
				memset(pBuffer,0,sizeof(pBuffer));
				sprintf(pBuffer,"CoinLevel%d",i);
				pConfig->vCoinLevels[i] = refReaderRow[pBuffer]->IntValue();
			}
#endif
			pRoomConfig = pConfig ;
		}
	//case eRoom_Baccarat:
	//case eRoom_PaiJiu:
	//	{
	//		stPaiJiurRoomConfig* pConfig = new stPaiJiurRoomConfig ;
	//		pConfig->nBankerNeedCoin = refReaderRow["BankerNeedCoin"]->IntValue();
	//		pRoomConfig = pConfig ;
	//	}
	//	break;
	default:
		CLogMgr::SharedLogMgr()->ErrorLog( "unknown room config ,room type = %d",cType ) ;
		return false;
	}
	pRoomConfig->nRoomType = cType ;
	pRoomConfig->nMaxSeat = refReaderRow["MaxSeat"]->IntValue();
	pRoomConfig->nMinNeedToEnter = refReaderRow["MiniCoin"]->IntValue();
	pRoomConfig->nRoomLevel = refReaderRow["RoomLevel"]->IntValue() ;
	pRoomConfig->nRoomID = refReaderRow["RoomID"]->IntValue();
	pRoomConfig->nWaitOperateTime = refReaderRow["OperateTime"]->IntValue();
	pRoomConfig->nCreateCount = refReaderRow["CreateCount"]->IntValue();
	m_vAllConfig.push_back(pRoomConfig) ;
	return true ;
}

stBaseRoomConfig* CRoomConfigMgr::GetRoomConfig(unsigned int nRoomID )
{
	LIST_ROOM_CONFIG::iterator iter = m_vAllConfig.begin();
	for ( ; iter != m_vAllConfig.end() ; ++iter )
	{
		stBaseRoomConfig* pRoom = *iter ;
		if ( nRoomID == pRoom->nRoomID )
		{
			return pRoom ;
		}
	}
	return NULL ;
}

stBaseRoomConfig* CRoomConfigMgr::GetRoomConfig( char cRoomType , char cRoomLevel )
{
	LIST_ROOM_CONFIG::iterator iter = m_vAllConfig.begin();
	for ( ; iter != m_vAllConfig.end() ; ++iter )
	{
		stBaseRoomConfig* pRoom = *iter ;
		if ( cRoomType == pRoom->nRoomType && cRoomLevel == pRoom->nRoomLevel )
		{
			return pRoom ;
		}
	}
	return NULL ;
}

void CRoomConfigMgr::Clear()
{
	LIST_ROOM_CONFIG::iterator iter = m_vAllConfig.begin();
	for ( ; iter != m_vAllConfig.end() ; ++iter )
	{
		delete *iter ;
		*iter = NULL ;
	}
	m_vAllConfig.clear() ;
}

void CRoomConfigMgr::OnFinishPaseFile()
{
	IConfigFile::OnFinishPaseFile();
	// clear 
	for ( int i = 0 ; i < eSpeed_Max ; ++i )
	{
		m_vSpeedRoomConfig[i].Reset();
	}

	LIST_ITER  iter = m_vAllConfig.begin();
	for ( ; iter != m_vAllConfig.end(); ++iter )
	{
		stTaxasRoomConfig* pConfig = (stTaxasRoomConfig*)(*iter);
		if ( pConfig->nWaitOperateTime >= TIME_LOW_LIMIT_FOR_NORMAL_ROOM )
		{
			m_vSpeedRoomConfig[eSpeed_Normal].AddConfig(pConfig);
		}
		else
		{
			m_vSpeedRoomConfig[eSpeed_Quick].AddConfig(pConfig);
		}
	}
}

int CRoomConfigMgr::GetConfigCnt(eSpeed speed , eRoomSeat eSeatCn )
{
	if ( speed < 0 || speed >= eSpeed_Max )
	{
		return 0 ;
	}
	return m_vSpeedRoomConfig[speed].m_vPlayerCountRoom[eSeatCn].size() ;
}

stTaxasRoomConfig* CRoomConfigMgr::GetConfig( eSpeed speed , eRoomSeat eSeatCn , unsigned int nIdx )
{
	VEC_BLIND_ROOM_CONFIG& vConfigs = m_vSpeedRoomConfig[speed].m_vPlayerCountRoom[eSeatCn];
	if ( vConfigs.size() <= nIdx )
	{
		return NULL ;
	}
	return vConfigs[nIdx] ;
}

stGoldenRoomConfig* CRoomConfigMgr::GetGoldenConfig(unsigned short cLevel )
{
	LIST_ROOM_CONFIG::iterator iter = m_vAllConfig.begin() ;
	for ( ; iter != m_vAllConfig.end() ; ++iter )
	{
		if ( (*iter)->nRoomType == eRoom_Gold && (*iter)->nRoomLevel == cLevel )
		{
			return (stGoldenRoomConfig*)(*iter);
		}
	}
	return NULL ;
}