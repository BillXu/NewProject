#include "IRoomDelegate.h"
#include "LogManager.h"
#include <ctime>
IRoomDelegate::~IRoomDelegate()
{
	removeAllRankItemPlayer();
}

void IRoomDelegate::onUpdatePlayerGameResult( IRoom* pRoom, uint32_t nUserUID , int32_t nOffsetGame, int32_t nOtherOffset  )
{
	if ( nOffsetGame == 0 && nOtherOffset == 0 )
	{
		return ;
	}

	m_bDirySorted = true ;

	auto targ = m_vRoomRankHistroy.find(nUserUID) ;
	if ( targ != m_vRoomRankHistroy.end() )
	{
		targ->second->nGameOffset += nOffsetGame ;
		targ->second->nOtherOffset += nOtherOffset ;
		targ->second->bIsDiryt = true ;
		CLogMgr::SharedLogMgr()->PrintLog("uid = %d update offset = %d , final = %d",nUserUID,nOffsetGame,targ->second->nGameOffset) ;
		return ;
	}
	stRoomRankItem* p = new stRoomRankItem ;
	p->nUserUID = nUserUID ;
	p->nGameOffset = nOffsetGame ;
	p->nOtherOffset = nOtherOffset ;
	p->bIsDiryt = true ;
	m_vRoomRankHistroy[p->nUserUID] = p ;
	m_vSortedRankItems.push_back(p) ;

	CLogMgr::SharedLogMgr()->PrintLog("uid = %d update offset = %d , final = %d",nUserUID,nOffsetGame,nOffsetGame) ;
}

uint8_t IRoomDelegate::canPlayerEnterRoom( IRoom* pRoom,stEnterRoomData* pEnterRoomPlayer )  // return 0 means ok ;
{
	return 0 ;
}

bool IRoomDelegate::isRoomShouldClose( IRoom* pRoom)
{
	return false ;
}

bool IRoomDelegate::isOmitNewPlayerHalo(IRoom* pRoom )
{
	return true ;
}

void IRoomDelegate::onRankPlayerChanged( uint32_t nUID , uint16_t nPreIdx , uint16_t nCurIdx )
{

}

bool IRoomDelegate::isPlayerLoseReachMax( IRoom* pRoom, uint32_t nUserUID )
{
	return false ;
}

void IRoomDelegate::removeAllRankItemPlayer()
{
	auto mapIter = m_vRoomRankHistroy.begin() ;
	for ( ; mapIter != m_vRoomRankHistroy.end() ; ++mapIter )
	{
		if ( mapIter->second )
		{
			delete mapIter->second ;
			mapIter->second = nullptr ;
		}
	}
	m_vRoomRankHistroy.clear() ;
	m_vSortedRankItems.clear() ;
}

bool sortFuncRankItem(IRoomDelegate::stRoomRankItem* pLeft , IRoomDelegate::stRoomRankItem* pRight )
{
	if ( (pLeft->nGameOffset + pLeft->nOtherOffset) > (pRight->nGameOffset + pRight->nOtherOffset) )
	{
		return true ;
	}
	return false ;
}

void IRoomDelegate::sortRoomRankItem()
{
	if ( m_bDirySorted && m_vSortedRankItems.size() >= 2 )
	{
		m_vSortedRankItems.sort(sortFuncRankItem);
		checkUpdateRankIdx();
	}
	m_bDirySorted = false ;
}

IRoomDelegate::stRoomRankItem* IRoomDelegate::getRankItemByUID(uint32_t nUID )
{
	auto iter = m_vRoomRankHistroy.find(nUID) ;
	if ( iter == m_vRoomRankHistroy.end() )
	{
		return nullptr ;
	}
	return iter->second ;
}

void IRoomDelegate::checkUpdateRankIdx()
{
	time_t tNow = time(nullptr) ;
	if ( tNow - m_tLastUpdateRankIdx < 60*10 )  // 15 minite check once 
	{
		//IRoom::onRankChanged() ;  // 15 minite update once 
		return ;
	}
	m_tLastUpdateRankIdx = tNow ;
	// check qian san ming shi fou bian hua ;
	auto Iter = m_vSortedRankItems.begin() ;
	auto endIter = m_vSortedRankItems.end() ;
	for ( uint16_t nIdx = 0 ; Iter != endIter ; ++Iter,++nIdx )
	{
		auto pp = *Iter ;
		if ( pp->nRankIdx == nIdx )
		{
			continue; 
		}

		if ( pp->nRankIdx < 0 )
		{
			pp->nRankIdx = nIdx ;
			continue;
		}

		onRankPlayerChanged(pp->nUserUID,pp->nRankIdx,nIdx) ;
		pp->nRankIdx = nIdx ;
	}
}