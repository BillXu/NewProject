#include "PeerCard.h"
#include <memory>
#include <assert.h>
#include "LogManager.h"
CPeerCard::CPeerCard()
{
	for ( int i = 0 ; i < PEER_CARD_COUNT ; ++i )
	{
		m_vCard[i] = NULL ;
	}
	 m_eCardType = ePeerCard_None ;
	 m_vRepacedCard.clear();
	 m_vShowedCard.clear() ;
}

CPeerCard::~CPeerCard()
{
	for ( int i = 0 ; i < PEER_CARD_COUNT ; ++i )
	{
		if ( m_vCard[i])
			delete m_vCard[i] ;
		m_vCard[i] = NULL ;
	}
}

void CPeerCard::SetPeerCardByNumber( unsigned char nNumber , unsigned char nNum, unsigned nNum2 )
{
	unsigned char vNum[] = {nNumber,nNum,nNum2 };
	for ( int i = 0 ; i < PEER_CARD_COUNT ; ++i )
	{
		if ( m_vCard[i] == NULL )
		{
			m_vCard[i] = new CCard(vNum[i]) ;
		}
		else 
		{
			m_vCard[i]->RsetCardByCompositeNum(vNum[i]) ;
		}
	}
	ArrangeCard();
	m_vRepacedCard.clear();
	m_vShowedCard.clear() ;
}

bool CPeerCard::PKPeerCard(CPeerCard* pPeerCard)
{
	if ( GetType() != pPeerCard->GetType() )
	{
		return GetType() > pPeerCard->GetType() ;
	}

	int myBig = 0 ;
	int nOtherBig = 0 ;
	for ( int i = PEER_CARD_COUNT -1 ; i >= 0 ; --i )
	{
		myBig = m_vCard[i]->GetCardFaceNum() == 1 ? 14 : m_vCard[i]->GetCardFaceNum() ;
		nOtherBig = pPeerCard->m_vCard[i]->GetCardFaceNum() == 1 ? 14 : pPeerCard->m_vCard[i]->GetCardFaceNum() ;
		if ( myBig != nOtherBig )
			return myBig > nOtherBig ;
	}
	return false ;	
}

void CPeerCard::ReplaceCardByNumber( unsigned char nOld , unsigned char nNew )
{
	for ( int i = 0 ; i < PEER_CARD_COUNT ; ++i )
	{
		if ( m_vCard[i]->GetCardCompositeNum() == nOld )
		{
			m_vCard[i]->RsetCardByCompositeNum(nNew) ;
			ArrangeCard();
			m_vRepacedCard.push_back(nOld) ;
			return ;
		}
	}
	assert(0);
	CLogMgr::SharedLogMgr()->ErrorLog("this is no card in my peer card : old = %d ",nOld ) ;
}

void CPeerCard::ShowCardByNumber(unsigned char nCardNum)
{
	for ( int i = 0 ; i < PEER_CARD_COUNT ; ++i )
	{
		if ( m_vCard[i]->GetCardCompositeNum() == nCardNum )
		{
			m_vShowedCard.push_back(nCardNum) ;
			return ;
		}
	}
	assert(0);
	CLogMgr::SharedLogMgr()->ErrorLog("Can not show Card : %d . do not exist it ",nCardNum ) ;
	return  ;
}

void CPeerCard::ArrangeCard()
{
	int iNum[PEER_CARD_COUNT] = { 0 } ;
	for ( int i = 0 ; i < PEER_CARD_COUNT ; ++i )
	{
		iNum[i] = m_vCard[i]->GetCardFaceNum() == 1 ? 14 : m_vCard[i]->GetCardFaceNum() ;
	}

	// 0 and 1 
	if ( iNum[0] > iNum[1] ) // switch 
	{
		CCard* pCard = m_vCard[1] ;
		m_vCard[1]= m_vCard[0] ;
		m_vCard[0]=pCard ;

		iNum[0] = iNum[0] + iNum[1] ;
		iNum[1] = iNum[0] - iNum[1] ;
		iNum[0] = iNum[0] - iNum[1];
	}

	// 1 and 2 
	if ( iNum[1] > iNum[2] )
	{
		CCard* pCard = m_vCard[2] ;
		m_vCard[2]= m_vCard[1] ;
		m_vCard[1]=pCard ;

		iNum[1] = iNum[1] + iNum[2] ;
		iNum[2] = iNum[1] - iNum[2] ;
		iNum[1] = iNum[1] - iNum[2];
	}

	// 0 and 1 
	if ( iNum[0] > iNum[1] ) // switch 
	{
		CCard* pCard = m_vCard[1] ;
		m_vCard[1]= m_vCard[0] ;
		m_vCard[0]=pCard ;

		iNum[0] = iNum[0] + iNum[1] ;
		iNum[1] = iNum[0] - iNum[1] ;
		iNum[0] = iNum[0] - iNum[1];
	}

	// decide type ;
	if ( m_vCard[0]->GetCardFaceNum() == m_vCard[1]->GetCardFaceNum() && m_vCard[1]->GetCardFaceNum() == m_vCard[2]->GetCardFaceNum() )
	{
		m_eCardType = ePeerCard_Bomb ;
	}
	else if ( m_vCard[0]->GetType() == m_vCard[1]->GetType() && m_vCard[1]->GetType() == m_vCard[2]->GetType() )
	{
		m_eCardType = ePeerCard_SameColor ;
		if ( iNum[0] +1 == iNum[1] && iNum[1] + 1 == iNum[2] )
		{
			m_eCardType =  ePeerCard_SameColorSequence ;
		}
	}
	else if ( iNum[0] +1 == iNum[1] && iNum[1] + 1 == iNum[2] )
	{
		m_eCardType = ePeerCard_Sequence ;
	}
	else if ( iNum[0] == iNum[1] || iNum[0] == iNum[2] || iNum[2] == iNum[1]) 
	{
		m_eCardType = ePeerCard_Pair ;
	}
	else
	{
		m_eCardType = ePeerCard_None ;
	}
}

void CPeerCard::LogInfo()
{
	const char* pType = NULL;
	switch( GetType() )
	{
		case ePeerCard_None:
			{
				pType = "普通牌型";
			}
			break;;
		case ePeerCard_Pair:
			{
				pType = "对子";
				break;
			}
		case  ePeerCard_Sequence:
			{
				pType = "顺子";
				break;
			}
		case ePeerCard_SameColor:
			{
				pType = "金花";
				break;
			}
		case ePeerCard_SameColorSequence:
			{
				pType = "顺金";
				break;
			}
		case ePeerCard_Bomb:
			{
				pType = "豹子";
				break;
			}
		default:
			pType = "未知牌型" ;
			break;

	}
	CLogMgr::SharedLogMgr()->PrintLog("%s",pType) ;
	for ( int i = 0 ; i < PEER_CARD_COUNT ; ++i )
	{
		if ( m_vCard[i])
			m_vCard[i]->LogCardInfo();
	}
}

void CPeerCard::Reset()
{
	m_eCardType = ePeerCard_None ;
	m_vRepacedCard.clear();
	m_vShowedCard.clear() ;
}

void CPeerCard::GetCompositeCardRepresent(char* vCard )
{
	for ( int i = 0 ; i < PEER_CARD_COUNT ; ++i )
	{
		vCard[i] = m_vCard[i]->GetCardCompositeNum() ;
	}
}