#include "RoomConfig.h"
#include "CommonDefine.h"
#include "LogManager.h"
#include <assert.h>
bool CRoomConfigMgr::OnPaser(CReaderRow& refReaderRow )
{
	unsigned char cType = refReaderRow["GameType"]->IntValue() ;
	stBaseRoomConfig* pRoomConfig = NULL ;
	switch ( cType )
	{
	case eRoom_NiuNiu:
		{
			stNiuNiuRoomConfig* pConfig = new stNiuNiuRoomConfig ;
			pConfig->nBaseBet = refReaderRow["BigBlind"]->IntValue();
			pConfig->nMiniTakeInCoin = refReaderRow["miniTakeIn"]->IntValue() ;
			pConfig->nMaxSeat = refReaderRow["MaxSeat"]->IntValue();
			pRoomConfig = pConfig ;
		}
		break;
	case eRoom_TexasPoker:
	case eRoom_TexasPoker_Diamoned:
		{
			stTaxasRoomConfig* pConfig = new stTaxasRoomConfig ;
			pConfig->nBigBlind = refReaderRow["BigBlind"]->IntValue();
			pConfig->nMaxTakeInCoin = refReaderRow["maxTakeIn"]->IntValue() ;
			pConfig->nMiniTakeInCoin = refReaderRow["miniTakeIn"]->IntValue() ;
			pConfig->nMaxSeat = refReaderRow["MaxSeat"]->IntValue();
			pRoomConfig = pConfig ;
		}
		break;
	//case eRoom_Gold:
	//	{
	//		stGoldenRoomConfig* pConfig = new stGoldenRoomConfig ;
	//		pConfig->bCanDoublePK = (bool)refReaderRow["CanDoublePK"]->IntValue();
	//		pConfig->nChangeCardRound = refReaderRow["ChangeCardRound"]->IntValue();
	//		pConfig->nMiniBet = refReaderRow["MiniBet"]->IntValue();
	//		pConfig->nTitleNeedToEnter = refReaderRow["TitleNeedToEnter"]->IntValue();
// #ifdef SERVER
// 			char pBuffer[256] = {0};
// 			for ( int i = 0 ; i < GOLDEN_ROOM_COIN_LEVEL_CNT ; ++i )
// 			{
// 				memset(pBuffer,0,sizeof(pBuffer));
// 				sprintf_s(pBuffer,"CoinLevel%d",i);
// 				pConfig->vCoinLevels[i] = refReaderRow[pBuffer]->IntValue();
// 			}
// #endif
			//pRoomConfig = pConfig ;
		//}
	default:
		CLogMgr::SharedLogMgr()->ErrorLog( "unknown room config ,room type = %d",cType ) ;
		return false;
	}
	pRoomConfig->nGameType = cType ;
	pRoomConfig->nConfigID = refReaderRow["configID"]->IntValue();
	pRoomConfig->nRentFeePerDay = refReaderRow["RendFeePerDay"]->IntValue() ;
	pRoomConfig->nDeskFee = refReaderRow["DeskFee"]->IntValue() ;
	pRoomConfig->fDividFeeRate = refReaderRow["DividFeeRate"]->FloatValue() ;
	m_vAllConfig.push_back(pRoomConfig) ;
	return true ;
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
}

int CRoomConfigMgr::GetConfigCnt( )
{
	return m_vAllConfig.size() ;
}

stBaseRoomConfig* CRoomConfigMgr::GetConfigByConfigID( uint16_t nConfigID )
{
	LIST_ITER iter = GetBeginIter() ;
	for ( ; iter != GetEndIter() ; ++iter )
	{
		if ( (*iter)->nConfigID == nConfigID )
		{
			return (stBaseRoomConfig*)(*iter) ;
		}
	}
	return nullptr ;
}

#ifndef SERVER
int CRoomConfigMgr::GetConfigCntByRoomType(int roomType){
    int count = 0;
    LIST_ITER iter = GetBeginIter() ;
    for ( ; iter != GetEndIter() ; ++iter )
    {
        if ( (*iter)->nGameType == roomType)
        {
            count++;
        }
    }
    return count;
}
stBaseRoomConfig* CRoomConfigMgr::GetConfigByConfigID( uint16_t idx , int roomType){
    LIST_ITER iter = GetBeginIter() ;
    for ( ; iter != GetEndIter() ; ++iter )
    {
        if ( (*iter)->nGameType == roomType )
        {
            idx--;
            if (!idx) {
                return (stBaseRoomConfig*)(*iter) ;
            }
        }
    }
    return nullptr ;
}
#endif


