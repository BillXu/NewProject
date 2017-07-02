#include "MJPlayerCard.h"
#include "MJCard.h"
#include "XLMJPlayerCard.h"
#include "NJMJPlayerCard.h"
#include "JJQEPlayerCard.h"
void tempTest()
{
	//CMJHuPaiInfo tInfo ;
	//	std::vector<uint8_t> vCards ;
	//	uint8_t nValue ;
	//	
	//	nValue = CMJCard::makeCardNumber(eCT_Wan,2);
	//	vCards.push_back(nValue);
	//	nValue = CMJCard::makeCardNumber(eCT_Wan,2);
	//	vCards.push_back(nValue);
	//	
	//	nValue = CMJCard::makeCardNumber(eCT_Wan, 2);
	//	vCards.push_back(nValue);
	//	
	//	nValue = CMJCard::makeCardNumber(eCT_Wan, 3);
	//	vCards.push_back(nValue);
	//	
	//	nValue = CMJCard::makeCardNumber(eCT_Wan, 4);
	//	vCards.push_back(nValue);
	//	nValue = CMJCard::makeCardNumber(eCT_Wan,3);
	//	vCards.push_back(nValue);
	//	
	//	nValue = CMJCard::makeCardNumber(eCT_Wan,6);
	//	vCards.push_back(nValue);
	//	nValue = CMJCard::makeCardNumber(eCT_Wan,6);
	//	vCards.push_back(nValue);
	//	
	//	nValue = CMJCard::makeCardNumber(eCT_Wan,4);
	//	vCards.push_back(nValue);
	//	nValue = CMJCard::makeCardNumber(eCT_Wan,4);
	//	vCards.push_back(nValue);
	//	
	//	nValue = CMJCard::makeCardNumber(eCT_Wan,5);
	//	vCards.push_back(nValue);
	//	nValue = CMJCard::makeCardNumber(eCT_Wan,5);
	//	vCards.push_back(nValue);
	//	
	//	nValue = CMJCard::makeCardNumber(eCT_Wan,6);
	//	vCards.push_back(nValue);
	//	nValue = CMJCard::makeCardNumber(eCT_Wan,7);
	//	vCards.push_back(nValue);
	//		
	//	bool b = tInfo.parseHuPaiInfo(vCards);
	//	if ( b )
	//	{
	//		printf("real hu \n") ;
	//	}
		JJQEPlayerCard tPeer ;
		tPeer.bindRoom(nullptr, 0);
		//tPeer.addDistributeCard(CMJCard::makeCardNumber(eCT_Tong, 9));
		//tPeer.addDistributeCard(CMJCard::makeCardNumber(eCT_Tong, 9));
		tPeer.addDistributeCard(CMJCard::makeCardNumber(eCT_Tiao, 1));
		tPeer.addDistributeCard(CMJCard::makeCardNumber(eCT_Tiao, 2));
		tPeer.addDistributeCard(CMJCard::makeCardNumber(eCT_Tiao, 3));


		//tPeer.addDistributeCard(CMJCard::makeCardNumber(eCT_Jian, 1));
		tPeer.addDistributeCard(CMJCard::makeCardNumber(eCT_Tiao, 4));
		tPeer.addDistributeCard(CMJCard::makeCardNumber(eCT_Tiao, 4));
		tPeer.onPeng(CMJCard::makeCardNumber(eCT_Tiao, 4));

		tPeer.addDistributeCard(CMJCard::makeCardNumber(eCT_Tong, 1));
		tPeer.addDistributeCard(CMJCard::makeCardNumber(eCT_Tong, 1));
		tPeer.addDistributeCard(CMJCard::makeCardNumber(eCT_Tong, 2));
		tPeer.addDistributeCard(CMJCard::makeCardNumber(eCT_Tong, 3));
		tPeer.addDistributeCard(CMJCard::makeCardNumber(eCT_Tong, 4));

		tPeer.addDistributeCard(CMJCard::makeCardNumber(eCT_Tong, 1));
		tPeer.addDistributeCard(CMJCard::makeCardNumber(eCT_Tong, 2));
		//tPeer.addDistributeCard(CMJCard::makeCardNumber(eCT_Wan, 3));
		//tPeer.addDistributeCard(CMJCard::makeCardNumber(eCT_Wan, 8));

		// qq
		//tPeer.addDistributeCard(CMJCard::makeCardNumber(eCT_Jian, 3));
		//tPeer.addDistributeCard(CMJCard::makeCardNumber(eCT_Jian, 2));
		//tPeer.onPeng(CMJCard::makeCardNumber(eCT_Jian, 2));

		/*tPeer.addDistributeCard(CMJCard::makeCardNumber(eCT_Jian, 1));
		tPeer.addDistributeCard(CMJCard::makeCardNumber(eCT_Jian, 1));
		tPeer.onPeng(CMJCard::makeCardNumber(eCT_Jian, 4));

		tPeer.addDistributeCard(CMJCard::makeCardNumber(eCT_Jian, 2));
		tPeer.addDistributeCard(CMJCard::makeCardNumber(eCT_Jian, 2));
		tPeer.onPeng(CMJCard::makeCardNumber(eCT_Jian, 2));*/

 
		auto isHu = tPeer.onDoHu(CMJCard::makeCardNumber(eCT_Tong, 3), 1);
		bool is3Red = false;
		auto nCnt = tPeer.getFinalHuCnt(true, is3Red);
		if ( isHu )
		{
			printf("right") ;
		}

	////	// test new chard ;
	//XLMJPlayerCard* pPlayerCard = new XLMJPlayerCard();
	//	
	//pPlayerCard->setQueType(eCT_Tong);
	//pPlayerCard->addDistributeCard(CMJCard::makeCardNumber(eCT_Wan, 2));
	//pPlayerCard->addDistributeCard(CMJCard::makeCardNumber(eCT_Wan, 4));

	//pPlayerCard->addDistributeCard(CMJCard::makeCardNumber(eCT_Wan, 3));
	//pPlayerCard->addDistributeCard(CMJCard::makeCardNumber(eCT_Wan, 3));
	//pPlayerCard->addDistributeCard(CMJCard::makeCardNumber(eCT_Wan, 3));

	//pPlayerCard->addDistributeCard(CMJCard::makeCardNumber(eCT_Wan, 5));
	//pPlayerCard->addDistributeCard(CMJCard::makeCardNumber(eCT_Wan, 6));

	////pPlayerCard->addDistributeCard(CMJCard::makeCardNumber(eCT_Wan, 7));
	//
	//	//pPlayerCard->addDistributeCard(CMJCard::makeCardNumber(eCT_Wan, 7));
	////pPlayerCard->addDistributeCard(CMJCard::makeCardNumber(eCT_Wan, 8));
	////pPlayerCard->addDistributeCard(CMJCard::makeCardNumber(eCT_Wan, 9));

	//pPlayerCard->addDistributeCard(CMJCard::makeCardNumber(eCT_Wan, 9));
	//pPlayerCard->addDistributeCard(CMJCard::makeCardNumber(eCT_Wan, 7));
	//pPlayerCard->addDistributeCard(CMJCard::makeCardNumber(eCT_Wan, 8));

	//	//pPlayerCard->onMingGang(CMJCard::makeCardNumber(eCT_Tong, 7), CMJCard::makeCardNumber(eCT_Wan, 1));
	//	/*pPlayerCard->onPeng(CMJCard::makeCardNumber(eCT_Tong, 5));
	//	pPlayerCard->onPeng(CMJCard::makeCardNumber(eCT_Tong, 2));
	//	pPlayerCard->onPeng(CMJCard::makeCardNumber(eCT_Wan, 8));
	//	pPlayerCard->onPeng(CMJCard::makeCardNumber(eCT_Wan, 2));*/
	//	uint32_t nT = 0;
	//	uint8_t nBeiShu;
	//	uint8_t nGen;
	//	std::set<uint8_t> vHu;
	//	pPlayerCard->getCanHuCards(vHu);
	//	auto p = pPlayerCard->onDoHu(false, CMJCard::makeCardNumber(eCT_Wan, 1), nT, nBeiShu, nGen);
	//	auto p2 = pPlayerCard->onDoHu(false, CMJCard::makeCardNumber(eCT_Wan, 7), nT, nBeiShu, nGen);
	//	pPlayerCard->onMoCard(CMJCard::makeCardNumber(eCT_Wan, 3));
	//	IMJPlayerCard::VEC_CARD v;
	//	auto b = pPlayerCard->getHoldCardThatCanAnGang(v);
	//	eFanxingType eType = (eFanxingType)nT;
	//	//if (!p)
	//	{
	//		printf("bug");
	//	}
	//	//printf("beishu = %u\n",p);
	//	if (pPlayerCard->isTingPai())
	//	{
	//		printf("do ting \n");
	//	}

}

#include "MJServer.h"
#include "Application.h"
int main()
{
	//tempTest();
	CApplication theAplication(CMJServerApp::getInstance());
	theAplication.startApp();
	return 0;
}