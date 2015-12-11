#include "RobotAIConfig.h"
void stAddType::ParseNode(TiXmlElement* pNode)
{
	const char* pType = pNode->Attribute("nType");
	if ( strcmp(pType,"timesBlind") == 0 )
	{
		nAddType = 0 ;
	}
	else if (strcmp(pType,"timesMySelf") == 0)
	{
		nAddType = 1 ;
	}
	else
	{
		printf("Error unknown add type \n") ;
	}

	pNode->QueryFloatAttribute("nRate",&fRate);
	pNode->QueryFloatAttribute("nTimes",&fTimes) ;
}

//stActionType::~stActionType()
//{
//	LIST_ADD_TYPE::iterator iter = vAddType.begin() ;
//	for ( ;iter != vAddType.end(); ++iter )
//	{
//		delete *iter ;
//		*iter = NULL ;
//	}
//	vAddType.clear() ;
//}

//eRobotActionType stActionType::ConverStringToActEnum(const char* pAct )
//{
//	eRobotActionType eTyppe ;
//	if ( strcmp(pAct,"Follow") == 0 )
//	{
//		eTyppe = eRbt_Act_Follow;
//	}
//	else if ( strcmp(pAct,"GiveUp") == 0  )
//	{
//		eTyppe = eRbt_Act_GiveUp;
//	}
//	else if ( strcmp(pAct,"ALLIn") == 0  )
//	{
//		eTyppe = eRbt_Act_AllIn;
//	}
//	else if ( strcmp(pAct,"Pass") == 0  )
//	{
//		eTyppe = eRbt_Act_Pass;
//	}
//	else if ( strcmp(pAct,"Add") == 0  )
//	{
//		eTyppe = eRbt_Act_Add;
//	}
//	else
//	{
//		eTyppe = eRbt_Act_Max ;
//		printf("unknown action Type = %s\n",pAct) ;
//	}
//	return eTyppe ;
//}

void stBetTimesAction::ParseNode(TiXmlElement* pNode)
{
	pNode->QueryIntAttribute("nTimes",&nBetTimes) ;
	const char* pcmd[] = { "Follow","Add","Pass","GiveUp","ALLIn"};
	for ( int i = 0 ; i < eRbt_Act_Max; ++i )
	{
		pNode->QueryFloatAttribute(pcmd[i],&m_ActRate[i]);
	}
	
	TiXmlElement* pChild = pNode->FirstChildElement() ;
	while ( pChild )
	{
		stAddType* pAdd = new stAddType ;
		pAdd->ParseNode(pChild) ;
		m_vBetAddStrages.push_back(pAdd) ;
		pChild = pChild->NextSiblingElement() ;
	}
}

bool stBetTimesAction::Check()
{
	float fValue = 0 ;
	for ( int i = 0 ; i < eRbt_Act_Max ; ++i )
	{
		fValue += m_ActRate[i] ;
	}

	if ( abs(fValue - 100 ) >= 0.00001 )
	{
		printf("All Act Type Rate not equal 100 , in Times = %d\n",nBetTimes ) ;
		return false ;
	}

	fValue = 0 ;
	LIST_ADD_TYPE::iterator iter = m_vBetAddStrages.begin();
	for ( ; iter != m_vBetAddStrages.end(); ++iter )
	{
		fValue += (*iter)->fRate ;
	}

	if ( abs(fValue - 100 ) >= 0.00001 )
	{
		printf("add Strages Rate not equal 100 , in Times = %d\n",nBetTimes ) ;
		return false ;
	}
	return true ;
}

eRobotActionType stBetTimesAction::RandActionType()const
{
	float fAllRate = 0 ;
	for ( int i = 0 ; i < eRbt_Act_Max ; ++i )
	{
		fAllRate += m_ActRate[i] ;
	}

	float fRandRate = (float)rand()/(float)RAND_MAX ;
	fRandRate *= fAllRate ;

	int nStartAct = rand() % eRbt_Act_Max ;
	float fCalRate = 0 ;
	for ( ; nStartAct < eRbt_Act_Max * 2 ; ++nStartAct )
	{
		if ( fCalRate < fRandRate && fRandRate <= fCalRate + m_ActRate[nStartAct%eRbt_Act_Max] )
		{
			return (eRobotActionType)(nStartAct%eRbt_Act_Max);
		}
		else
		{
			fCalRate += m_ActRate[nStartAct%eRbt_Act_Max];
		}
	}
	printf("can not find proper act type \n") ;
	return eRbt_Act_Pass ;
}

eRobotActionType stBetTimesAction::RandActionWithIn(unsigned char cActA, unsigned char cActB )const
{
	float fTotal = m_ActRate[cActA] + m_ActRate[cActB] ;
	float fRandRate = (float)rand() / float(RAND_MAX);
	fRandRate *= fTotal ;
	if ( rand() % 2 == 0 )
	{
		if ( fRandRate <= m_ActRate[cActA] )
		{
			return (eRobotActionType)cActA;
		}
	}
	else
	{
		if ( fRandRate > m_ActRate[cActA] )
		{
			return (eRobotActionType)cActA;
		}
	}
	return (eRobotActionType)cActB;
}

stAddType* stBetTimesAction::RandAddStrage()const
{
	float nRate = (float)rand() / float(RAND_MAX);
	nRate *= 100 ;
	LIST_ADD_TYPE::const_iterator iter = m_vBetAddStrages.begin();
	float nAll = 0 ;
	for ( ; iter != m_vBetAddStrages.end(); ++iter )
	{
		if ( nAll < nRate && nRate <= nAll + (*iter)->fRate )
		{
			return *iter ;
		}
		else
		{
			nAll = nAll + (*iter)->fRate ;
		}
	}
	printf("not find proper add strages\n") ;
	--iter ;
	return *iter;
}

//bool stActionType::Check()
//{
//	if ( eActType != eRbt_Act_Add )
//		return true ;
//	float fTotalRate = 0 ;
//	LIST_ADD_TYPE::iterator iter = vAddType.begin() ;
//	for ( ; iter != vAddType.end(); ++iter )
//	{
//		fTotalRate += (*iter)->fRate ;
//	}
//
//	return abs(fTotalRate - 100) <= 0.000001 ;
//}
//
//void stActionType::ParseNode(TiXmlElement* pNode)
//{
//	int nAct = 0 ;
//	const char* pAct = pNode->Attribute("Act");
//	eActType = stActionType::ConverStringToActEnum(pAct);
//	pNode->QueryFloatAttribute("Rate",&fRate);
//
//	if ( eActType != eRbt_Act_Add || eActType == eRbt_Act_Max )
//	{
//		return ;
//	}
//	/// juset add action ;
//	TiXmlElement* pChild = pNode->FirstChildElement() ;
//	while( pChild )
//	{
//		stAddType* pAddType = new stAddType ;
//		pAddType->ParseNode(pChild) ;
//		pChild = pChild->NextSiblingElement() ;
//		vAddType.push_back(pAddType) ;
//	}
//}

void stCardTypeAction::ParseNode(TiXmlElement* pNode)
{
	pNode->QueryIntAttribute("type",(int*)(&nCard));
	TiXmlElement* pChild = pNode->FirstChildElement() ;
	while ( pChild )
	{
		int nTimes = -1  ;
		pChild->QueryIntAttribute("nTimes",&nTimes);
		if ( nTimes == -1 )
		{
			printf(" can not read nTimes in CardType = %d\n",nCard) ;
			pChild = pChild->NextSiblingElement() ;
			continue;
		}
		
		if ( vAllBetTimesStrages.find(nTimes) != vAllBetTimesStrages.end() )
		{
			printf("double times = %d , in CardType = %d\n",nTimes,nCard) ;
			pChild = pChild->NextSiblingElement() ;
			continue;
		}

		stBetTimesAction* pstTimesBet = new stBetTimesAction ;
		pstTimesBet->ParseNode(pChild) ;
		vAllBetTimesStrages[nTimes] = pstTimesBet ;
		pChild = pChild->NextSiblingElement() ;
	}
}
bool stCardTypeAction::Check()
{
	MAP_BET_TIMES_STRAGES::iterator iter = vAllBetTimesStrages.begin() ;
	for ( ; iter != vAllBetTimesStrages.end() ; ++iter )
	{
		if ( iter->second->Check() == false )
		{
			return false ;
		}
	}
	return true ;
}

const stBetTimesAction* stCardTypeAction::GetBetTimesAction( int nTimes)
{
	stBetTimesAction* pTargetAct = NULL ;
	MAP_BET_TIMES_STRAGES::iterator iter = vAllBetTimesStrages.begin();
	for ( ;iter != vAllBetTimesStrages.end() ; ++iter)
	{
		if ( iter->first >= nTimes )
		{
			MAP_BET_TIMES_STRAGES::iterator iterForward = iter ;
			if ( iterForward == vAllBetTimesStrages.begin() )
			{
				return iterForward->second ;
			}
			--iterForward ;
			if ( abs(nTimes - iterForward->first) > abs(nTimes - iter->first ) )
			{
				return iter->second ;
			}
			return iterForward->second ;
		}
	}

	--iter;
	return iter->second ;
}
//void stCardControl::ParseNode(TiXmlElement* pNode)
//{
//	pNode->QueryFloatAttribute("nTotalRate",&fTotalRate);
//	TiXmlElement* pChild = pNode->FirstChildElement() ;
//	while( pChild )
//	{
//		int nType = -1 ;
//		pChild->QueryIntAttribute("type",&nType);
//		if ( nType < 0 || nType >= CTaxasPokerPeerCard::eCard_Max )
//		{
//			printf("Unknown Card Type = %d\n",nType) ;
//			return ;
//		}
//		m_vAllCardAction[nType].ParseNode(pChild) ;
//		m_vAllCardAction[nType].fTotalRate = fTotalRate ;
//		pChild = pChild->NextSiblingElement() ;
//	}
//}
//
//bool stCardControl::Check()
//{
//	for ( int i = 0 ; i < CTaxasPokerPeerCard::eCard_Max ; ++i )
//	{
//		if ( m_vAllCardAction[i].nCard != (unsigned char)-1 && m_vAllCardAction[i].Check() == false )
//		{
//			printf(" card type check failed nCard = %d \n",i) ;
//			return false ;
//		}
//	}
//	return true ;
//}

//void stTimesTypeAction::ParseNode(TiXmlElement* pNode)
//{
//	pNode->QueryFloatAttribute("nTimes",&fTimes);
//	TiXmlElement* pChild = pNode->FirstChildElement() ;
//	while ( pChild )
//	{
//		int nType = -1  ;
//		const char* pAct = pChild->Attribute("Act");
//		nType = stActionType::ConverStringToActEnum(pAct);
//		if ( nType < 0 || nType >= eRbt_Act_Max )
//		{
//			printf("Unknown Act Type = %d\n",nType) ;
//			return ;
//		}
//		m_vActionType[nType].ParseNode(pChild) ;
//		pChild = pChild->NextSiblingElement() ;
//	}
//}
//
//bool stTimesTypeAction::Check()
//{
//	float fTotal = 0;
//	for ( int i = 0 ; i < eRbt_Act_Max ; ++i )
//	{
//		fTotal += m_vActionType[i].fRate ;
//		if ( m_vActionType[i].Check() == false )
//		{
//			printf("stTimesTypeAction Check Error times = %.3f \n",fTimes) ;
//			return false ;
//		}
//	}
//	return fTotal == fTotalRate ;
//}
//
//void stTimesControl::ParseNode(TiXmlElement* pNode)
//{
//	pNode->QueryFloatAttribute("nTotalRate",&fTotalRate);
//	TiXmlElement* pChild = pNode->FirstChildElement() ;
//	while (pChild)
//	{
//		stTimesTypeAction* pAction = new stTimesTypeAction ;
//		pAction->fTotalRate = fTotalRate ;
//		pAction->ParseNode(pChild) ;
//		m_vAllTimesActions.push_back(pAction) ;
//		pChild = pChild->NextSiblingElement() ;
//	}
//}
//
//bool stTimesControl::Check()
//{
//	LIST_TIMES_ACTION::iterator iter = m_vAllTimesActions.begin() ;
//	for ( ; iter != m_vAllTimesActions.end(); ++iter )
//	{
//		if ( (*iter)->Check() == false )
//		{
//			printf("times control check Error times = %.3f\n",(*iter)->fTimes) ;
//			return false ;
//		}
//	}
//	return true ;
//}
//
//stTimesTypeAction* stTimesControl::GetTypeAction( int nTimes )
//{
//	stTimesTypeAction* pAction = NULL ;
//	LIST_TIMES_ACTION::iterator iter = m_vAllTimesActions.begin() ;
//	for ( ; iter != m_vAllTimesActions.end(); ++iter )
//	{
//		if ( (*iter)->fTimes >= nTimes )
//		{
//			pAction = *iter ;
//		}
//
//		if ( pAction == NULL || (*iter)->fTimes < pAction->fTimes )
//		{
//			pAction = *iter ;
//		}
//	}
//
//	if ( pAction )
//	{
//		return pAction ;
//	}
//	--iter;
//	if ( iter != m_vAllTimesActions.end() )
//	{
//		pAction = *iter;
//	}
//	return pAction;
//}
//
//stTimesControl::~stTimesControl()
//{
//	LIST_TIMES_ACTION::iterator iter = m_vAllTimesActions.begin() ;
//	for ( ; iter != m_vAllTimesActions.end(); ++iter )
//	{
//		delete *iter ;
//		*iter = NULL;
//	}
//	m_vAllTimesActions.clear();
//}
//
//stActionType* stFinalAction::RandMyAction()
//{
//	float fFinal = (float)rand() / float(RAND_MAX) ;
//	fFinal *= fTotal ;
//
//	unsigned char nStart = rand() % eRbt_Act_Max ;
//	float fAll = 0 ;
//	for ( ; nStart < eRbt_Act_Max* 2 ; ++nStart )
//	{
//		if ( fAll < fFinal && fFinal <= m_vActionType[nStart%eRbt_Act_Max].fRate + fAll )
//		{
//			return  &m_vActionType[nStart%eRbt_Act_Max] ;
//		}
//		else
//		{
//			fAll = m_vActionType[nStart%eRbt_Act_Max].fRate + fAll ;
//		}
//	}
//	printf("not find proper rate target ;\n") ;
//	return &m_vActionType[eRbt_Act_Pass];
//}
//
//stActionType* stFinalAction::RandMyActionWithIn(eRobotActionType eTypeA,eRobotActionType eTypeB )
//{
//	float fTotal = m_vActionType[eTypeA].fRate + m_vActionType[eTypeB].fRate ;
//	float fRandRate = (float)rand() / float(RAND_MAX);
//	fRandRate *= fTotal ;
//	if ( rand() % 2 == 0 )
//	{
//		if ( fRandRate <= m_vActionType[eTypeA].fRate )
//		{
//			return &m_vActionType[eTypeA];
//		}
//	}
//	else
//	{
//		if ( fRandRate > m_vActionType[eTypeA].fRate )
//		{
//			return &m_vActionType[eTypeA];
//		}
//	}
//	return &m_vActionType[eTypeB];
//}
//
//stAddType* stFinalAction::RandAddType()
//{
//	if ( m_vpAddTypes == NULL )
//	{
//		printf("have no add choices \n") ;
//		return NULL ;
//	}
//
//	float nRate = (float)rand() / float(RAND_MAX);
//	nRate *= 100 ;
//	stActionType::LIST_ADD_TYPE::iterator iter = m_vpAddTypes->begin();
//	float nAll = 0 ;
//	for ( ; iter != m_vpAddTypes->end(); ++iter )
//	{
//		if ( nAll < nRate && nRate <= nAll + (*iter)->fRate )
//		{
//			return *iter ;
//		}
//		else
//		{
//			nAll = nAll + (*iter)->fRate ;
//		}
//	}
//
//	printf("do not have proper add type \n") ;
//	--iter ;
//	if ( iter != m_vpAddTypes->end() )
//	{
//		return *iter ;
//	}
//	return NULL ;
//}

void stRound::ParseNode(TiXmlElement* pNode)
{
	pNode->QueryIntAttribute("nCount",&nRound);
	TiXmlElement* pChild = pNode->FirstChildElement();
	while ( pChild )
	{
		int nCardType = -1 ;
		pChild->QueryIntAttribute("caType",&nCardType) ;
		if ( nCardType == -1 )
		{
			printf("can not read Card Type in Round = %d \n",nRound) ;
			pChild = pChild->NextSiblingElement() ;
			continue;
		}

		if ( vAllCardTypeStrages.find(nCardType) != vAllCardTypeStrages.end() )
		{
			printf(" in Round = %d ,have double cardType = %d\n",nRound,nCardType);
			pChild = pChild->NextSiblingElement() ;
			continue;
		}

		stCardTypeAction* pActForCardType = new stCardTypeAction ;
		pActForCardType->nCard = (unsigned char)nCardType ;
		vAllCardTypeStrages[pActForCardType->nCard] = pActForCardType ;
		pActForCardType->ParseNode(pChild) ;
		pChild = pChild->NextSiblingElement() ;
	}
 
}

const stBetTimesAction* stRound::GetFinalAction(unsigned char nCardType, int nTimes )
{
	MAP_CARD_TYPE_STRAGES::iterator iter = vAllCardTypeStrages.find(nCardType) ;
	if ( iter == vAllCardTypeStrages.end() )
	{
		printf("NO proper card Type = %d\n",nCardType);
		if ( nCardType == 13 )
		{
			return NULL ;
		}
		else
		{
			return GetFinalAction(13,nTimes) ;
		}
	}
	return iter->second->GetBetTimesAction(nTimes) ;
} 

bool stRound::Check()
{
	MAP_CARD_TYPE_STRAGES::iterator iter = vAllCardTypeStrages.begin() ;
	for ( ; iter != vAllCardTypeStrages.end() ; ++iter )
	{
		if ( iter->second && iter->second->Check() == false )
		{
			return false ;
		}
	}
	return true ;
}

void stRobotAI::ParseNode(TiXmlElement* pNode)
{
	pNode->QueryIntAttribute("nTypeID",(int*)&nRobotAIID);
	int nRound = -1 ;
	TiXmlElement* pChild = pNode->FirstChildElement() ;
	while (pChild )
	{
		pChild->QueryIntAttribute("nCount",&nRound) ;
		if ( nRound < 0 || nRound >= 4 )
		{
			printf("unknown round = %d\n",nRound) ;
			return ;
		}
		m_vRound[nRound].ParseNode(pChild) ;
		pChild = pChild->NextSiblingElement() ;
	}
}

bool stRobotAI::Check()
{
	for ( int i = 0 ; i < 3 ; ++i )
	{
		if ( m_vRound[i].Check() == false )
		{
			printf("AI Check Failed round = %d\n",i) ;
			return false ;
		}
	}
	return true ;
}

void CRobotAIManager::LoadFile(const char* pFileName )
{
	TiXmlDocument tDoc (pFileName);
	tDoc.LoadFile() ;
	TiXmlElement* pRoot = tDoc.RootElement();
	if ( pRoot == NULL )
	{
		printf("root node is NULl for file = %s\n",pFileName) ;
		return ;
	}

	TiXmlElement* pRobotAI = pRoot->FirstChildElement() ;
	while ( pRobotAI )
	{
		stRobotAI* pAi = new stRobotAI ;
		pAi->ParseNode(pRobotAI) ;
		if ( GetRobotAIBy(pAi->nRobotAIID) )
		{
			printf("Double robot Ai have the same id = %d\n",pAi->nRobotAIID) ;
			delete pAi ;
			continue; 
		}
		if ( pAi->Check())
		{
			m_vAllRobotAI.insert(MAP_ROBOT_AI::value_type(pAi->nRobotAIID,pAi)) ;
		}
		else
		{
			printf("AI Check Failed id = %d\n",pAi->nRobotAIID) ;
			delete pAi ;
			pAi = NULL ;
		}
		pRobotAI = pRobotAI->NextSiblingElement() ;
	}
}

stRobotAI* CRobotAIManager::GetRobotAIBy(unsigned short nRobotID )
{
	MAP_ROBOT_AI::iterator iter = m_vAllRobotAI.find(nRobotID) ;
	if ( iter != m_vAllRobotAI.end())
	{
		return iter->second ;
	}
	return NULL ;
}
