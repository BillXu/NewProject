#include "TaxasRobotAIConfig.h"
void CTaxasAINode::clearMapNode(MAP_TAXASAINODE& vNodes)
{
	MAP_TAXASAINODE::iterator iter = vNodes.begin();
	for ( ; iter != vNodes.end(); ++iter )
	{
		delete iter->second ;
		iter->second = nullptr ;
	}
	vNodes.clear();
}

bool CTaxasAINode::addNodeToMap(MAP_TAXASAINODE& vMap,CTaxasAINode* pNode )
{
	if ( pNode == nullptr )
	{
		return false ;
	}

	MAP_TAXASAINODE::iterator iter = vMap.find(pNode->getKeyValue());
	if ( iter != vMap.end() )
	{
		return false ;
	}
	vMap.insert(MAP_TAXASAINODE::value_type(pNode->getKeyValue(),pNode));
	return true ;
}

CTaxasAINode* CTaxasAINode::getNodeFromMap(MAP_TAXASAINODE& vMap, int nKey )
{
	// 因为所有的map key 都是 int ， 所以排序天然就是按照 < 进行排序的。
	// 而我们需要的查找也是 要从左往右，从小到大，最接近的一个

	if ( vMap.empty() )
	{
		return nullptr ;
	}

	MAP_TAXASAINODE::iterator iter = vMap.begin();
	CTaxasAINode* pNodeFind = iter->second ;

	for ( ; iter != vMap.end(); ++iter  )
	{
		if ( iter->first <= nKey )
		{
			pNodeFind = iter->second ;
		}
		else
		{
			break; 
		}
	}
	return pNodeFind ;
}


void CTaxasAIBetTimesForAction::parseNode(TiXmlElement* pNode)
{
	if ( pNode == nullptr )
	{
		printf("CTaxasAIBetTimesForAction node is null\n");
		return ;
	}

	pNode->Attribute("nTimes",&nBetTimes);
	pNode->QueryUnsignedAttribute("addLowLimit",&nAddLowLimit);
	pNode->QueryUnsignedAttribute("addTopLimit",&nAddTopLimit);
	pNode->QueryFloatAttribute("Follow",&vAIActionRate[eAIAct_Follow]);
	pNode->QueryFloatAttribute("Pass",&vAIActionRate[eAIAct_Pass]);
	pNode->QueryFloatAttribute("Add",&vAIActionRate[eAIAct_Add]);
	pNode->QueryFloatAttribute("ALLIn",&vAIActionRate[eAIAct_AllIn]);
	if ( nAddLowLimit > nAddTopLimit )
	{
		nAddLowLimit |= nAddTopLimit ;
		nAddTopLimit = nAddLowLimit^nAddTopLimit ;
		nAddLowLimit = nAddLowLimit^nAddTopLimit ;
	}
}

CTaxasAINode::eAIActionType CTaxasAIBetTimesForAction::getAction(int& addTimesBlind )
{
	addTimesBlind = 0 ;
	float nRate = (float)rand() / float(RAND_MAX);
	nRate *= 100 ;
	float fStepCursor = 0 ;
	eAIActionType eAct = CTaxasAINode::eAIAct_Max ;
	for ( int idx = 0 ; idx < eAIAct_Max ; ++idx )
	{
		if ( vAIActionRate[idx] < 0.00001 )  // = 0 , do not check
		{
			continue;
		}

		fStepCursor += vAIActionRate[idx];
		if ( nRate <= fStepCursor )
		{
			eAct = (eAIActionType)idx ;
			break;
		}
	}

	if ( eAIAct_Max == eAct )
	{
		printf("can not find act nrate = %.5f\n",nRate);
		return eAIAct_Pass ;
	}

	if ( eAIAct_Add == eAct )
	{
		nRate = (float)rand() / float(RAND_MAX);
		nRate *= (nAddTopLimit - nAddLowLimit) ;
		addTimesBlind = (nAddLowLimit + nRate + 0.5f) ;
	}
	return eAct ;
}

void CTaxasAIHoldCardContribute::parseNode(TiXmlElement* pNode)
{
	if ( pNode == nullptr )
	{
		printf("CTaxasAIHoldCardContribute node is null\n");
		return ;
	}

	pNode->Attribute("count",&nContributeCardCnt);
	TiXmlElement* pchid = pNode->FirstChildElement();
	for ( ; pchid != nullptr; pchid = pchid->NextSiblingElement() )
	{
		CTaxasAIBetTimesForAction* pTimesBet = new CTaxasAIBetTimesForAction;
		pTimesBet->parseNode(pchid);
		if ( addNodeToMap(vAllBetTimes,pTimesBet) == false )
		{
			printf("already add this bet times = %d\n",pTimesBet->nBetTimes);
			delete pTimesBet ;
		}
	}
}

CTaxasAINode::eAIActionType CTaxasAIHoldCardContribute::getAction(unsigned int nMostBetTimesBlind,int& addTimesBlind )
{
	CTaxasAINode* pNode = getNodeFromMap(vAllBetTimes,nMostBetTimesBlind);
	CTaxasAIBetTimesForAction* pBetTimes = dynamic_cast<CTaxasAIBetTimesForAction*>(pNode);
	if ( pBetTimes == nullptr )
	{
		printf("can not find card contribute = %d , betTimes = %d\n",nContributeCardCnt,nMostBetTimesBlind);
		return CTaxasAINode::eAIAct_Pass ;
	}
	return pBetTimes->getAction(addTimesBlind);
}

void CTaxasAIKeyCard::parseNode(TiXmlElement* pNode)
{
	if ( pNode == nullptr )
	{
		printf("CTaxasAIKeyCard node is null\n");
		return ;
	}

	pNode->Attribute("faceValue",&nFaceValue);
	TiXmlElement* pchid = pNode->FirstChildElement();
	for ( ; pchid != nullptr; pchid = pchid->NextSiblingElement() )
	{
		CTaxasAIHoldCardContribute* pTimesBet = new CTaxasAIHoldCardContribute;
		pTimesBet->parseNode(pchid);
		if ( addNodeToMap(vHoldCardContributes,pTimesBet) == false )
		{
			printf("already add this contribute cnt = %d\n",pTimesBet->nContributeCardCnt);
			delete pTimesBet ;
		}
	}
}

CTaxasAINode::eAIActionType CTaxasAIKeyCard::getAction(unsigned short nContributeCardCnt,unsigned int nMostBetTimesBlind,int& addTimesBlind )
{
	CTaxasAINode* pNode = getNodeFromMap(vHoldCardContributes,nContributeCardCnt);
	CTaxasAIHoldCardContribute* pContribute = dynamic_cast<CTaxasAIHoldCardContribute*>(pNode);
	if ( pContribute == nullptr )
	{
		printf("can not find card faceValue = %d contribute = %d \n",nFaceValue,nContributeCardCnt);
		return CTaxasAINode::eAIAct_Pass ;
	}
	return pContribute->getAction(nMostBetTimesBlind,addTimesBlind);
}

void CTaxasAICardType::parseNode(TiXmlElement* pNode)
{
	if ( pNode == nullptr )
	{
		printf("CTaxasAICardType node is null\n");
		return ;
	}

	pNode->Attribute("caType",&nCardType);
	TiXmlElement* pchid = pNode->FirstChildElement();
	for ( ; pchid != nullptr; pchid = pchid->NextSiblingElement() )
	{
		CTaxasAIKeyCard* pTimesBet = new CTaxasAIKeyCard;
		pTimesBet->parseNode(pchid);
		if ( addNodeToMap(vKeyCards,pTimesBet) == false )
		{
			printf("already add this contribute cnt = %d\n",pTimesBet->nFaceValue);
			delete pTimesBet ;
		}
	}
}

CTaxasAINode::eAIActionType CTaxasAICardType::getAction(unsigned short nContributeCardCnt,unsigned short nkeyCardFaceValue,unsigned int nMostBetTimesBlind,int& addTimesBlind )
{
	CTaxasAINode* pNode = getNodeFromMap(vKeyCards,nkeyCardFaceValue);
	CTaxasAIKeyCard* pCard = dynamic_cast<CTaxasAIKeyCard*>(pNode);
	if ( pCard == nullptr )
	{
		printf("can not find cardType = %d, card faceValue = %d\n",nCardType,nkeyCardFaceValue);
		return CTaxasAINode::eAIAct_Pass ;
	}
	return pCard->getAction(nContributeCardCnt,nMostBetTimesBlind,addTimesBlind);
}

void CTaxasAIRound::parseNode(TiXmlElement* pNode)
{
	if ( pNode == nullptr )
	{
		printf("CTaxasAICardType node is null\n");
		return ;
	}

	pNode->Attribute("nCount",&nRound);
	TiXmlElement* pchid = pNode->FirstChildElement();
	for ( ; pchid != nullptr; pchid = pchid->NextSiblingElement() )
	{
		CTaxasAICardType* pTimesBet = new CTaxasAICardType;
		pTimesBet->parseNode(pchid);
		if ( addNodeToMap(vCardTypes,pTimesBet) == false )
		{
			printf("already add this cardtype cnt = %d\n",pTimesBet->nCardType);
			delete pTimesBet ;
		}
	}
}

CTaxasAINode::eAIActionType CTaxasAIRound::getAction(unsigned int nCardType ,unsigned short nContributeCardCnt,unsigned short nkeyCardFaceValue,unsigned int nMostBetTimesBlind,int& addTimesBlind )
{
	CTaxasAINode* pNode = getNodeFromMap(vCardTypes,nCardType);
	CTaxasAICardType* pCardType = dynamic_cast<CTaxasAICardType*>(pNode);
	if ( pCardType == nullptr )
	{
		printf("can not find round = %d, card type = %d\n",nRound,nCardType);
		return CTaxasAINode::eAIAct_Pass ;
	}
	return pCardType->getAction(nContributeCardCnt,nkeyCardFaceValue,nMostBetTimesBlind,addTimesBlind);
}


bool CTaxasRobotAI::init(const char* pAiConfigFile )
{
	TiXmlDocument tDoc (pAiConfigFile);
	tDoc.LoadFile() ;
	TiXmlElement* pRoot = tDoc.RootElement();
	if ( pRoot == NULL )
	{
		printf("root node is NULl for file = %s\n",pAiConfigFile) ;
		return false;
	}

	TiXmlElement* pRobotAI = pRoot->FirstChildElement() ;
	for ( ; pRobotAI != nullptr; pRobotAI = pRobotAI->NextSiblingElement() )
	{
		int nRound = -1 ; 
		pRobotAI->QueryIntAttribute("nCount",&nRound);
		if (nRound < 0 || nRound > 3 )
		{
			printf("round cofiger error round = %d ",nRound);
		}
		else
		{
			 vRound[nRound].parseNode(pRobotAI);
		}
	}
	return true ;
}

CTaxasAINode::eAIActionType CTaxasRobotAI::getAction( unsigned short nRound, unsigned int nCardType ,unsigned short nContributeCardCnt,unsigned short nkeyCardFaceValue,unsigned int nMostBetTimesBlind,int& addTimesBlind )
{
	if ( nRound >= 4 )
	{
		printf("invalid round = %d\n",nRound);
		return CTaxasAINode::eAIAct_Pass ;
	}

	CTaxasAIRound& refRound = vRound[nRound];
	return refRound.getAction(nCardType,nContributeCardCnt,nkeyCardFaceValue,nMostBetTimesBlind,addTimesBlind);
}