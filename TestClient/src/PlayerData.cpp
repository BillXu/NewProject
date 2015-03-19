#include "PlayerData.h"
#include "MessageDefine.h"
#include "LogManager.h"
bool CPlayerData::OnMessage( Packet* pMsg )
{
	stMsg* pmsg = (stMsg*)pMsg->_orgdata ;
	if ( pmsg->usMsgType == MSG_PLAYER_BASE_DATA )
	{
		//stMsgPlayerBaseData* pMsgBaseData = (stMsgPlayerBaseData*)pmsg ;
		//nMyCoin = pMsgBaseData->nCoin ;
		//nDiamoned = pMsgBaseData->nDiamoned;
		//nSessionID = pMsgBaseData->nSessionID ;
		//char* pBuffer = new char[pMsgBaseData->nNameLen + 1 ] ;
		//memset(pBuffer,0,pMsgBaseData->nNameLen + 1 ) ;
		//memcpy(pBuffer,((char*)pMsgBaseData) + sizeof(stMsgPlayerBaseData),pMsgBaseData->nNameLen );
		//strName = pBuffer ;
		//delete [] pBuffer ;
		//printf("Name = %s Received base data, coin = %I64d \n", strName.c_str(),nMyCoin ) ;
		//srand(nSessionID) ;
		//nTakeInCoin = 0 ;
		//nTakeInDiamoned = 0 ;
	}
	return false ;
}

void CPlayerData::OnWinCoin( int64_t nCoinOffset, bool bDiamoned  )
{
	if ( bDiamoned )
	{
		nTakeInDiamoned += nCoinOffset ;
	}
	else
	{
		nTakeInCoin += nCoinOffset ;
	}
}

int64_t CPlayerData::GetCoin(bool bDiamond )
{ 
	return ( bDiamond ?nTakeInDiamoned: nMyCoin) ;
}
void CPlayerData::TakeIn(uint64_t nCoin , bool bDimoned)
{
	nMyCoin += nTakeInCoin ;
	nTakeInCoin = 0 ;
	nDiamoned += nTakeInDiamoned ;
	nTakeInDiamoned = 0 ;

	if ( !bDimoned )
	{
		nTakeInCoin = nCoin < nMyCoin ? nCoin : nMyCoin ;
		nMyCoin -= nTakeInCoin ;
		printf("%s take in %d, leftAll = %I64d\n",strName.c_str(),nTakeInCoin,nMyCoin) ;
	}
	else
	{
		nTakeInDiamoned = nCoin < nDiamoned ? nCoin : nDiamoned ;
		nDiamoned -= nTakeInDiamoned ;
		printf("%s take in %d, leftAll = %d\n",strName.c_str(),nTakeInDiamoned,nDiamoned) ;
	}
}

void CPlayerData::BetCoin(bool bDiamoned , int nOffset )
{
	if ( bDiamoned )
	{
		if ( nTakeInDiamoned >= nOffset )
		{
			nTakeInDiamoned -= nOffset ;
		}
		else
		{
			nTakeInDiamoned = 0 ;
		}
	}
	else
	{
		if ( nTakeInCoin >= nOffset )
		{
			nTakeInCoin -= nOffset ;
		}
		else
		{
			nTakeInCoin = 0 ;
		}
	}
}

bool CPlayerData::IsMustWin( bool bDiamoned )
{
	if ( bDiamoned )
	{
		if ( nDiamoned + nTakeInDiamoned < pRobotItem->nMinLeftDiamond  )
		{
			float frate = (float)rand() / float(RAND_MAX) ;
			frate *= 100 ;
			if ( frate <= pRobotItem->fMustWinRate )
			{
				return true ;
			}
			else
			{
				return false ;
			}
		}
		else
		{
			return false ;
		}
	}
	else
	{
		if ( nTakeInCoin + nMyCoin < pRobotItem->nMinLeftCoin  )
		{
			float frate = (float)rand() / float(RAND_MAX) ;
			frate *= 100 ;
			if ( frate <= pRobotItem->fMustWinRate )
			{
				return true ;
			}
			else
			{
				return false ;
			}
		}
		else
		{
			return false ;
		}
	}
	return true ;
}