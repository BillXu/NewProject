#include "MJPlayer.h"
#include "MessageIdentifer.h"
#include "ServerDefine.h"
#include "log4z.h"
#include "MJServer.h"
#include "AsyncRequestQuene.h"
void MJPlayer::init(stEnterRoomData* pData)
{
	setState(eRoomPeer_WaitNextGame);
	m_nUserUID = pData->nUserUID;
	m_nSessioID = pData->nUserSessionID;
	m_nCoin = pData->nCoin;
	m_nIdx = -1;
	m_nOffset = 0;
	m_nPlayerType = pData->nPlayerType;
	clearDecareBuGangFlag();
}

void MJPlayer::onComeBackRoom(stEnterRoomData* pData)
{
	m_nSessioID = pData->nUserSessionID;
	m_nCoin = pData->nCoin;
	m_nPlayerType = pData->nPlayerType;
}

void MJPlayer::onWillStartGame()
{
	m_nOffset = 0;
	clearGangFlag();
}

void MJPlayer::onStartGame()
{
	setState(eRoomPeer_CanAct);
	clearDecareBuGangFlag();
}

void MJPlayer::onGameDidEnd()
{
	setState(eRoomPeer_WaitNextGame);
	clearDecareBuGangFlag();
}

void MJPlayer::onGameEnd()
{
	setState(eRoomPeer_WaitNextGame);
	clearGangFlag();
	clearDecareBuGangFlag();
}

uint8_t MJPlayer::getIdx()
{
	return m_nIdx;
}

void MJPlayer::setIdx(uint8_t nIdx)
{
	m_nIdx = nIdx;
}

int32_t MJPlayer::getCoin()
{
	return m_nCoin;
}

void MJPlayer::setCoin( int32_t nNewCoin)
{
	m_nCoin = nNewCoin;
}

void MJPlayer::addOffsetCoin(int32_t nOffset)
{
	if (nOffset < 0 && (-1 * nOffset) >(int32_t)getCoin())
	{
		LOGFMTE( "do not have so much money to offset = %d ,have = %u",nOffset,getCoin() );
		m_nOffset -= m_nCoin;
		m_nCoin = 0;
		return;
	}
	m_nOffset += nOffset;
	m_nCoin = (int32_t)m_nCoin + nOffset;

	// sync coin to data svr ;
	Json::Value jsReq;
	jsReq["uid"] = getUID();
	jsReq["coin"] = getCoin();
	CMJServerApp::getInstance()->getAsynReqQueue()->pushAsyncRequest(ID_MSG_PORT_DATA, eAsync_SyncPlayerRoomCoin, jsReq);
}

int32_t MJPlayer::getOffsetCoin()
{
	return m_nOffset;
}

void MJPlayer::setState(uint32_t eState)
{
	m_eState = eState;
}

bool MJPlayer::haveState(uint32_t eState)
{
	return (getState() & eState) == eState;
}

uint32_t MJPlayer::getState()
{
	return m_eState;
}

uint32_t MJPlayer::getSessionID()
{
	return m_nSessioID;
}

uint32_t MJPlayer::getUID()
{
	return m_nUserUID;
}

void MJPlayer::signGangFlag()
{
	m_isHaveGang = true;
}

void MJPlayer::clearGangFlag()
{
	m_isHaveGang = false;
}

bool MJPlayer::haveGangFalg()
{
	return m_isHaveGang;
}

void MJPlayer::signDecareBuGangFlag()
{
	m_isDeclareBuGang = true;
}

void MJPlayer::clearDecareBuGangFlag()
{
	m_isDeclareBuGang = false;
}

bool MJPlayer::haveDecareBuGangFalg()
{
	return m_isDeclareBuGang;
}

int32_t MJPlayer::onRecievedSupplyCoin(uint32_t nSupplyCoin)
{
	m_nCoin += nSupplyCoin;
	return m_nCoin;
}

bool MJPlayer::isRobot()
{
	return ePlayer_Robot == m_nPlayerType;
}
