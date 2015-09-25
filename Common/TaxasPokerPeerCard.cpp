#include "TaxasPokerPeerCard.h"
#include <algorithm>
#include "LogManager.h"
bool CompFunction(CCard* left , CCard* right )
{
	unsigned char nNumLeft = left->GetCardFaceNum(true) ;
	unsigned char nNumRight = right->GetCardFaceNum(true) ;
	if(nNumLeft > nNumRight)
		return false; 

	if (nNumLeft < nNumRight)
		return true;
	return false;
}

void CTaxasPokerPeerCard::AddCardByCompsiteNum(unsigned char nCardNum )
{
	CCard* pCard = new CCard(nCardNum) ;
	if ( m_vDefaul.size() < 2 )
	{
		m_vDefaul.push_back(pCard) ;
		if ( m_vDefaul.size() == 2 )
		{
			sort(m_vDefaul.begin(),m_vDefaul.end(),CompFunction );
		}
	}

	m_vAllCard.push_back(pCard) ;
}

const char* CTaxasPokerPeerCard::GetTypeName()
{ 
	if ( nCardCountWhenCaculate < m_vAllCard.size() )
	{
		CaculateFinalCard() ;
	}
	return m_strCardName.c_str();
}

CTaxasPokerPeerCard::eCardType CTaxasPokerPeerCard::GetCardType()
{ 
	if ( nCardCountWhenCaculate < m_vAllCard.size() )
	{
		CaculateFinalCard() ;
	}
	return m_eType ; 
}

void CTaxasPokerPeerCard::adjustPosForSpecailShunZi()
{
	if ( m_vFinalCard.size() != 5 )
	{
		return ;
	}

	if ( GetCardType() == eCard_ShunZi || eCard_TongHuaShun == GetCardType() )
	{
		sort(m_vFinalCard.begin(),m_vFinalCard.end(),CompFunction );
		if (m_vFinalCard[0]->GetCardFaceNum(false) == 2 && m_vFinalCard[4]->GetCardFaceNum(false) == 1 )
		{
			CCard* pCard = m_vFinalCard[4];
			VEC_CARD::iterator iter = m_vFinalCard.end();
			--iter ;
			m_vFinalCard.erase(iter);
			m_vFinalCard.insert(m_vFinalCard.begin(),pCard);
		}
	}
}

char CTaxasPokerPeerCard::PK(CTaxasPokerPeerCard* pPeerCard )
{
	if ( nCardCountWhenCaculate < m_vAllCard.size() )
	{
		CaculateFinalCard() ;
	}

	// first compare card type ;
	if ( pPeerCard->GetCardType() < GetCardType() )  // failed
	{
		return -1 ;
	}
	else if ( pPeerCard->GetCardType() > GetCardType() )
	{
		return 1 ;
	}

	// the same card  type
	switch ( GetCardType() )
	{
	case eCard_YiDui:
	case eCard_SiTiao:
	case eCard_SanTiao:
		{
			if ( m_vPairs[0][0]->GetCardFaceNum(true) > pPeerCard->m_vPairs[0][0]->GetCardFaceNum(true) )
			{
				return 1 ;
			}
			else if ( m_vPairs[0][0]->GetCardFaceNum(true) < pPeerCard->m_vPairs[0][0]->GetCardFaceNum(true) )
			{
				return -1 ;
			}
		}
		break;
	case eCard_LiangDui:
	case eCard_HuLu:
		{
			if ( m_vPairs[0][0]->GetCardFaceNum(true) > pPeerCard->m_vPairs[0][0]->GetCardFaceNum(true) )
			{
				return 1 ;
			}
			else if (m_vPairs[0][0]->GetCardFaceNum(true) < pPeerCard->m_vPairs[0][0]->GetCardFaceNum(true))
			{
				return -1 ;
			}

			if ( m_vPairs[1][0]->GetCardFaceNum(true) > pPeerCard->m_vPairs[1][0]->GetCardFaceNum(true) )
			{
				return 1 ;
			}
			else if ( m_vPairs[1][0]->GetCardFaceNum(true) < pPeerCard->m_vPairs[1][0]->GetCardFaceNum(true) )
			{
				return -1 ;
			}
		}
		break;
	}

	// sort common final card 
	sort(m_vFinalCard.begin(),m_vFinalCard.end(),CompFunction );
	sort(pPeerCard->m_vFinalCard.begin(),pPeerCard->m_vFinalCard.end(),CompFunction );
	adjustPosForSpecailShunZi();
	pPeerCard->adjustPosForSpecailShunZi();
	for ( int i = m_vFinalCard.size() -1  ; i >= 0 ; --i )
	{
		unsigned char nNumSelf = m_vFinalCard[i]->GetCardFaceNum(true) ;
		unsigned char nNumOther = pPeerCard->m_vFinalCard[i]->GetCardFaceNum(true) ;

		if ( nNumSelf > nNumOther )
		{
			return 1 ;
		}
		else if ( nNumSelf < nNumOther )
		{
			return -1 ;
		}
	}
	return 0 ;
}

void CTaxasPokerPeerCard::Reset()
{
	nCardCountWhenCaculate = 0 ;
	ClearVecCard(m_vAllCard) ;
	m_vDefaul.clear() ;
	m_vFinalCard.clear() ;
	m_eType = eCard_Max ;
	m_strCardName = "Wait";
	m_vPairs[0].clear();
	m_vPairs[1].clear(); 
}

void CTaxasPokerPeerCard::LogInfo()
{
	CLogMgr::SharedLogMgr()->SystemLog("card Type = %s",m_strCardName.c_str() ) ;
	CLogMgr::SharedLogMgr()->SystemLog("All card is :") ;

	
	for ( unsigned int i= 0 ; i < m_vAllCard.size() ; ++i )	
	{
		m_vAllCard[i]->LogCardInfo();
	}

	CLogMgr::SharedLogMgr()->SystemLog("Final card is :") ;
	for ( unsigned int i= 0 ; i < m_vFinalCard.size() ; ++i )
	{
		m_vFinalCard[i]->LogCardInfo();
	}
}

void CTaxasPokerPeerCard::GetFinalCard( unsigned char vMaxCard[5])
{
    if ( nCardCountWhenCaculate < m_vAllCard.size() )
	{
		CaculateFinalCard() ;
	}
    
	if ( m_vFinalCard.size() != 5 )
	{

	}
	else
	{
		for ( int i = 0 ; i < m_vFinalCard.size() ; ++i )
		{
			vMaxCard[i] = m_vFinalCard[i]->GetCardCompositeNum();
		}
	}
}

void CTaxasPokerPeerCard::GetHoldCard(unsigned char vHoldeCard[2] )
{
	if ( m_vDefaul.size() != 2 )
		return ;
	vHoldeCard[0] = m_vDefaul[0]->GetCardCompositeNum();
	vHoldeCard[1] = m_vDefaul[1]->GetCardCompositeNum();
}

unsigned char CTaxasPokerPeerCard::GetCardTypeForRobot(unsigned char& nContriButeCnt,unsigned char& nKeyCardFaceNum )
{
	unsigned char nRobotCardType = 0 ;
	nKeyCardFaceNum = 0 ;
	if ( nCardCountWhenCaculate < m_vAllCard.size() )
	{
		CaculateFinalCard() ;
	}

	if ( eCard_GaoPai == m_eType )
	{
		// check gao pai 4 tong hua 
		VEC_CARD vColor[CCard::eCard_Max] ;
		for ( size_t i = 0 ; i < m_vAllCard.size() ; ++i  )
		{
			CCard* pcard = m_vAllCard[i] ;
			vColor[pcard->GetType()].push_back(pcard) ;
		}

		for ( int i = 0 ; i < CCard::eCard_Max ; ++i )
		{
			if ( vColor[i].size() == 4 )
			{
				nRobotCardType = eCard_Robot_GaoPai4TongHua ;
				nContriButeCnt = robotGetContribute(vColor[i],m_vDefaul,nKeyCardFaceNum) ;
				return nRobotCardType ;
			}
		}

		// check gaoPia 4 shun zi 
		VEC_CARD vNewCard ;
		vNewCard.assign(m_vAllCard.begin(),m_vAllCard.end()) ;
		VEC_CARD vResult ;
		CheckShunZi(vNewCard,true,vResult) ;
		if ( vResult.size() != 4)
		{
			CheckShunZi(vNewCard,false,vResult) ;
		}

		if ( vResult.size() == 4 )
		{
			nRobotCardType = eCard_Robot_GaoPai4ShunZi ;
			nContriButeCnt = robotGetContribute(vResult,m_vDefaul,nKeyCardFaceNum) ;
			return nRobotCardType ;
		}

	} 
	nRobotCardType = m_eType ;
	switch (nRobotCardType)
	{
	case eCard_HuangJiaTongHuaShun:
	case eCard_TongHuaShun:
	case eCard_TongHua:
	case eCard_ShunZi:
	case eCard_GaoPai:
		{
			nContriButeCnt = robotGetContribute(m_vFinalCard,m_vDefaul,nKeyCardFaceNum) ;
		}
		break;
	case eCard_SiTiao:
	case eCard_YiDui:
	case eCard_SanTiao:
		{
			nContriButeCnt = robotGetContribute(m_vPairs[0],m_vDefaul,nKeyCardFaceNum) ;
			if ( nContriButeCnt == 0 )
			{
				robotGetContribute(m_vFinalCard,m_vDefaul,nKeyCardFaceNum) ;
			}
		}
		break;
	case eCard_HuLu:
	case eCard_LiangDui:
		{
			nContriButeCnt = robotGetContribute(m_vPairs[0],m_vDefaul,nKeyCardFaceNum) ;
			if ( nContriButeCnt == 0 )
			{
				nContriButeCnt = robotGetContribute(m_vPairs[1],m_vDefaul,nKeyCardFaceNum) ;
			}

			if ( nContriButeCnt == 0 )
			{
				robotGetContribute(m_vFinalCard,m_vDefaul,nKeyCardFaceNum) ;
			}
			
		}	
		break;
	default:
		nContriButeCnt = robotGetContribute(m_vFinalCard,m_vDefaul,nKeyCardFaceNum) ;
		return nRobotCardType ;
	}
	return nRobotCardType ;
}

void CTaxasPokerPeerCard::CaculateFinalCard()
{
	nCardCountWhenCaculate = m_vAllCard.size() ;
	if ( nCardCountWhenCaculate < 5 )
	{
		CLogMgr::SharedLogMgr()->ErrorLog("Do nothing , card count < 5 ; ") ;
		return ;
	}
	m_vFinalCard.clear() ;
	m_vPairs[0].clear();
	m_vPairs[1].clear();
	// cacluate card types ;
	sort(m_vAllCard.begin(),m_vAllCard.end(),CompFunction );
	/// specail situation A may be 1 .
//	if ( m_vAllCard[0]->GetCardFaceNum() == 2 && m_vAllCard[m_vAllCard.size()-1]->GetCardFaceNum() == 1 )
//	{
//		m_vFinalCard.push_back(m_vAllCard[0]) ;
//		for ( int i = 0 ; i < m_vAllCard.size() - 1 ; ++i )
//		{
//			if ( m_vAllCard[i]->GetCardFaceNum() + 1 == m_vAllCard[i+1]->GetCardFaceNum() )
//			{
//				m_vFinalCard.push_back(m_vAllCard[i+1]) ;
//				if ( m_vFinalCard.size() == 4 )
//				{
//					m_vFinalCard.insert(m_vFinalCard.begin(),m_vAllCard[m_vAllCard.size()-1]);
//					break; 
//				}
//			}
//			else if ( m_vAllCard[i]->GetCardFaceNum() == m_vAllCard[i+1]->GetCardFaceNum() )
//			{
//
//			}
//			else
//			{
//				m_vFinalCard.clear();
//				break;
//			}
//		}
//
//		if ( m_vFinalCard.size() == 5 )
//		{
//			// check is tong hua
//			bool bSameType = true ;
//			for ( int i = 0 ; i < m_vFinalCard.size() -1 ; ++i )
//			{
//				if ( m_vFinalCard[i]->GetType() != m_vFinalCard[i+1]->GetType() )
//				{
//					bSameType = false ;
//					break;
//				}
//			}
//			if ( bSameType )
//			{
//				m_eType =  eCard_TongHuaShun ;
//#if (!defined(GAME_SERVER)) && (!defined(ROBOT))
//				m_strCardName = "ͬ��˳";
//#endif	
//			}
//			else
//			{
//				m_eType =  eCard_ShunZi ;
//				m_strCardName = "˳��";
//			}
//			return ;
//		}
//	}
	///specail end ;
	// find same color type 
	m_vFinalCard.clear();
	VEC_CARD vColor[CCard::eCard_Max] ;
	for ( size_t i = 0 ; i < m_vAllCard.size() ; ++i  )
	{
		CCard* pcard = m_vAllCard[i] ;
		vColor[pcard->GetType()].push_back(pcard) ;
	}

	for ( int i = 0 ; i < CCard::eCard_Max ; ++i )
	{
		if ( vColor[i].size() >= 5 )
		{
			VEC_CARD& vC = vColor[i] ;
			std::sort(vC.begin(),vC.end(),CompFunction );
			m_eType =  eCard_TongHua ;
			m_strCardName = "paixing_tonghua";
			// add final ;
			for ( int j = vC.size() -1  ;j >= 0 ; --j )
			{
				m_vFinalCard.push_back(vC[j]) ;
				if ( m_vFinalCard.size() == 5 )
				{
					break;
				}
			}
			// if have shun zi ;
			VEC_CARD vNewCard ;
			vNewCard.assign(vC.begin(),vC.end()) ;
			VEC_CARD vResult ;
			CheckShunZi(vNewCard,true,vResult) ;
			if ( vResult.size() == 5 )
			{
				m_eType =  eCard_TongHuaShun ;
				m_vFinalCard.clear();
				m_vFinalCard.assign(vResult.begin(),vResult.end()) ;
#if (!defined(SERVER)) && (!defined(ROBOT))
				m_strCardName = "tong hua shun";
#endif	
				return ;
			}

			CheckShunZi(vNewCard,false,vResult) ;
			if ( vResult.size() == 5 )
			{
				m_eType =  eCard_TongHuaShun ;
				m_vFinalCard.clear();
				m_vFinalCard.assign(vResult.begin(),vResult.end()) ;
#if (!defined(SERVER)) && (!defined(ROBOT))
				m_strCardName = "paixing_tonghuashun";
#endif	
				return ;
			}
			return ;
		}
	}

	// find shun zi ;
	VEC_CARD vNewCard ;
	vNewCard.assign(m_vAllCard.begin(),m_vAllCard.end()) ;
	VEC_CARD vResult ;
	CheckShunZi(vNewCard,true,vResult) ;
	if ( vResult.size() == 5 )
	{
		m_vFinalCard.clear();
		m_vFinalCard.assign(vResult.begin(),vResult.end()) ;
		m_eType =  eCard_ShunZi ;
		m_strCardName = "paixing_shunzi";
		return ;
	}

	CheckShunZi(vNewCard,false,vResult) ;
	if ( vResult.size() == 5 )
	{
		m_vFinalCard.clear();
		m_vFinalCard.assign(vResult.begin(),vResult.end()) ;
		m_eType =  eCard_ShunZi ;
		m_strCardName = "paixing_shunzi";
		return ;
	}

	VEC_CARD vPairs[3] ;
	VEC_CARD vAllCardHelper;
	int nPairIdx = 0 ;
//	CCard* pCardIndicator = NULL ;
	vAllCardHelper.assign(m_vAllCard.begin(),m_vAllCard.end()) ;
	for ( int i = vAllCardHelper.size() -1  ; i > 0 ; --i )
	{
		if ( vAllCardHelper[i]->GetCardFaceNum() == vAllCardHelper[i -1]->GetCardFaceNum() )
		{
			vPairs[nPairIdx].push_back(vAllCardHelper[i]) ;
			vAllCardHelper[i] = NULL ;
			if ( i - 1 == 0 )
			{
				vPairs[nPairIdx].push_back(vAllCardHelper[i-1]) ;
				vAllCardHelper[i-1] = NULL ;
			}
		}
		else
		{
			if ( vPairs[nPairIdx].size() >= 1 )
			{
				vPairs[nPairIdx].push_back(vAllCardHelper[i]) ;
				vAllCardHelper[i] = NULL ;
				++nPairIdx ;
			}
		}
	}
	// anlays pairs ;
	for ( int i = 0 ; i < 3 ; ++i )
	{
		for ( int j = i + 1 ; j < 3 ; ++j )
		{
			if (vPairs[j].size() < vPairs[i].size() || vPairs[j].empty() )
			{
				vPairs[j].swap(vPairs[i]) ;
			}
			else if ( vPairs[j].size() == vPairs[i].size() && vPairs[j][0]->GetCardFaceNum(true) < vPairs[i][0]->GetCardFaceNum(true) )
			{
				vPairs[j].swap(vPairs[i]) ;
			}
		}
	}

	if ( vPairs[2].empty() )
	{
		// no pair 
		m_vFinalCard.clear();
		for ( int i = m_vAllCard.size() -1 ; i >= 0 ; --i )
		{
			m_vFinalCard.push_back(m_vAllCard[i]) ;
			if ( m_vFinalCard.size() == 5 )
			{
				std::sort(m_vFinalCard.begin(),m_vFinalCard.end(),CompFunction );
				break;
			}
		}
		m_eType = eCard_GaoPai;
		m_strCardName = "paixing_gaopai" ;
		return ;
	}

	// have pairs ;
	if ( vPairs[2].size() == 4 )  // si tiao 
	{
		if ( vPairs[1].size() > 0 ) // avoid  1 1 1 1 3 3 3 , if not add back to helpper ,  finaly card will be only 4 card ;
		{
			vAllCardHelper.insert(vAllCardHelper.begin(),vPairs[1].begin(),vPairs[1].end()) ;
			// clear NULL node 
			VEC_CARD::iterator iter = vAllCardHelper.begin();
			while ( iter != vAllCardHelper.end() )
			{
				if ( *iter == NULL )
				{
					vAllCardHelper.erase(iter) ;
					iter = vAllCardHelper.begin() ;
					continue;
				}
				++iter ;
			}
			std::sort(vAllCardHelper.begin(),vAllCardHelper.end(),CompFunction );
		}

		m_vFinalCard.assign(vPairs[2].begin(),vPairs[2].end()) ;
		m_vPairs[0].assign(vPairs[2].begin(),vPairs[2].end()) ;
		for ( int i = vAllCardHelper.size() -1  ; i >= 0 ; --i )
		{
			if ( vAllCardHelper[i] )
			{
				m_vFinalCard.push_back(vAllCardHelper[i]) ;
				if ( m_vFinalCard.size() != 5 )
				{
					continue;
				}
				m_eType = eCard_SiTiao;
				m_strCardName = "paixing_sitiao" ;
				return ;
			}
		}
		CLogMgr::SharedLogMgr()->ErrorLog("No enough card to fill !") ; 
	}
	else if ( vPairs[2].size() == 3 ) 
	{
		m_vFinalCard.assign(vPairs[2].begin(),vPairs[2].end() ) ;
		m_vPairs[0].assign(vPairs[2].begin(),vPairs[2].end()) ;
		if ( vPairs[1].size() >= 2 )  // hu lu 
		{
			m_vFinalCard.push_back(vPairs[1][0]);
			m_vFinalCard.push_back(vPairs[1][1]);
			m_vPairs[1].push_back(vPairs[1][0]);
			m_vPairs[1].push_back(vPairs[1][1]);
			//m_vFinalCard.insert(m_vFinalCard.begin(),vPairs[1].begin(),vPairs[1].end() ) ;
		}
		else  // san tiao ;
		{
			for ( int i = vAllCardHelper.size() -1 ; i >= 0 ; --i )
			{
				if ( vAllCardHelper[i] )
				{
					m_vFinalCard.push_back(vAllCardHelper[i]) ;
					if ( m_vFinalCard.size() == 5 )
					{
						m_eType = eCard_SanTiao ;
						m_strCardName = "paixing_santiao" ;
						return ;
					}
				}
			}
		}
		m_eType = eCard_HuLu ;
		m_strCardName = "paixing_hulu" ;
		return ;
	}
	else if ( vPairs[2].size() == 2 )
	{
		m_vFinalCard.assign(vPairs[2].begin(),vPairs[2].end()) ;
		m_vPairs[0].assign(vPairs[2].begin(),vPairs[2].end()) ;
		if ( vPairs[1].size() == 2 ) // two pairs
		{
			m_vFinalCard.insert(m_vFinalCard.begin(),vPairs[1].begin(),vPairs[1].end() ) ;
			m_vPairs[1].insert(m_vPairs[1].begin(),vPairs[1].begin(),vPairs[1].end() ) ;
			m_eType = eCard_LiangDui ;
			m_strCardName = "paixing_liangdui" ;
		}
		else  // one pairs ;
		{
			m_eType = eCard_YiDui ;
#if defined(SERVER) || defined(ROBOT) 

#else
			m_strCardName = "paixing_yidui";
#endif
		}

		for ( int i = vAllCardHelper.size() -1 ; i >= 0 ; --i )
		{
			if (vAllCardHelper[i] )
			{
				m_vFinalCard.push_back(vAllCardHelper[i]) ;
				if ( m_vFinalCard.size() == 5 )
				{
					return ;
				}
			}
		}

	}
	CLogMgr::SharedLogMgr()->ErrorLog( "analys card error , unknown error !" ) ;
}

void CTaxasPokerPeerCard::ClearVecCard(VEC_CARD& vCards )
{
	VEC_CARD::iterator iter = vCards.begin() ;
	for ( ;iter != vCards.end() ; ++iter )
	{
		delete *iter ;
		*iter = NULL ;
	}
	vCards.clear() ;
}

void CTaxasPokerPeerCard::CheckShunZi(VEC_CARD& AllCard , bool bSpecailA, VEC_CARD& vResultChardOut )
{
	CCard* vCard = AllCard[AllCard.size()-1] ;
	if ( bSpecailA == false ) // only for check A 2 3 4 5 
	{
		if (  AllCard[AllCard.size()-1]->GetCardFaceNum() != 1 || AllCard[0]->GetCardFaceNum() != 2 )
		{
			vResultChardOut.clear();
			return ;
		}

		VEC_CARD::iterator iter = AllCard.end();
		--iter ;
		AllCard.erase(iter) ;
		AllCard.insert(AllCard.begin(),vCard) ;
	}

	vResultChardOut.clear();
	unsigned char nShunCount = 0 ;
	for ( int i = AllCard.size() -1  ; i > 0 ; --i )
	{
		if ( AllCard[i]->GetCardFaceNum(bSpecailA) == AllCard[i-1]->GetCardFaceNum(bSpecailA) + 1 )
		{
			++nShunCount ;
			if ( nShunCount == 4 )
			{
				for ( size_t j = i - 1 ;j < AllCard.size() ; ++j )
				{
					if ( (j+1) < AllCard.size() && AllCard[j]->GetCardFaceNum(bSpecailA) == AllCard[j+1]->GetCardFaceNum(bSpecailA) )
					{
						// avoid pairs ;
						continue;
					}

					vResultChardOut.push_back(AllCard[j]) ;
					if ( vResultChardOut.size() == 5 )
					{
						return;
					}
				}
			}
		}
		else if ( AllCard[i]->GetCardFaceNum(bSpecailA) == AllCard[i-1]->GetCardFaceNum(bSpecailA) )  // may be pair ;
		{
			// do nothing 
		}
		else
		{
			nShunCount = 0 ;
		}
	}
	vResultChardOut.clear();
}

void CTaxasPokerPeerCard::robotCheck4ShunZi( VEC_CARD& AllCard , bool bSpecailA, VEC_CARD& vResultChardOut )
{
	CCard* vCard = AllCard[AllCard.size()-1] ;
	if ( bSpecailA == false ) // only for check A 2 3 4 5 
	{
		if (  AllCard[AllCard.size()-1]->GetCardFaceNum() != 1 || AllCard[0]->GetCardFaceNum() != 2 )
		{
			vResultChardOut.clear();
			return ;
		}

		VEC_CARD::iterator iter = AllCard.end();
		--iter ;
		AllCard.erase(iter) ;
		AllCard.insert(AllCard.begin(),vCard) ;
	}

	vResultChardOut.clear();
	unsigned char nShunCount = 0 ;
	for ( int i = AllCard.size() -1  ; i > 0 ; --i )
	{
		if ( AllCard[i]->GetCardFaceNum(bSpecailA) == AllCard[i-1]->GetCardFaceNum(bSpecailA) + 1 )
		{
			++nShunCount ;
			if ( nShunCount == 3 )
			{
				for ( size_t j = i - 1 ;j < AllCard.size() ; ++j )
				{
					if ( (j+1) < AllCard.size() && AllCard[j]->GetCardFaceNum(bSpecailA) == AllCard[j+1]->GetCardFaceNum(bSpecailA) )
					{
						// avoid pairs ;
						continue;
					}

					vResultChardOut.push_back(AllCard[j]) ;
					if ( vResultChardOut.size() == 4 )
					{
						return;
					}
				}
			}
		}
		else if ( AllCard[i]->GetCardFaceNum(bSpecailA) == AllCard[i-1]->GetCardFaceNum(bSpecailA) )  // may be pair ;
		{
			// do nothing 
		}
		else
		{
			nShunCount = 0 ;
		}
	}
	vResultChardOut.clear();
}

unsigned char CTaxasPokerPeerCard::robotGetContribute(VEC_CARD& vFinalCard, VEC_CARD& vCheckCard, unsigned char& vOutKeyCardFaceNum )
{
	vOutKeyCardFaceNum = 0 ;
	VEC_CARD vInBothCard ;
	for ( int nCheckIdx = 0; nCheckIdx < vCheckCard.size() ; ++nCheckIdx )
	{
		unsigned char nCompsitNum = vCheckCard[nCheckIdx]->GetCardCompositeNum();
		for ( int nTargetIdx = 0 ; nTargetIdx < vFinalCard.size() ; ++nTargetIdx )
		{
			if ( nCompsitNum == vFinalCard[nTargetIdx]->GetCardCompositeNum() )
			{
				vInBothCard.push_back(vFinalCard[nTargetIdx]);
			}
		}
	}

	for ( int nIdx = 0 ;  nIdx < vInBothCard.size() ; ++nIdx )
	{
		if ( vInBothCard[nIdx]->GetCardFaceNum(true) > vOutKeyCardFaceNum )
		{
			vOutKeyCardFaceNum = vInBothCard[nIdx]->GetCardFaceNum(true) ;
		}
	}
	
	return vInBothCard.size() ;
}