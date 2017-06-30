#include "MJPlayerCard.h"
#include "MJCard.h"
#include "XLMJPlayerCard.h"
#include "NJMJPlayerCard.h"
#include "JJQEPlayerCard.h"
#include "GG23PlayerCard.h"
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

		GG23PlayerCard tPeer ;
		tPeer.bindRoom(nullptr, 0);
		//tPeer.addDistributeCard(CMJCard::makeCardNumber(eCT_Tong, 2));
		//tPeer.addDistributeCard(CMJCard::makeCardNumber(eCT_Tong, 3));
		//tPeer.addDistributeCard(CMJCard::makeCardNumber(eCT_Tong, 4));

		//tPeer.addDistributeCard(CMJCard::makeCardNumber(eCT_Tong, 6));
		////tPeer.addDistributeCard(CMJCard::makeCardNumber(eCT_Tong, 7));
		//tPeer.addDistributeCard(CMJCard::makeCardNumber(eCT_Tiao, 3));
		//tPeer.addDistributeCard(CMJCard::makeCardNumber(eCT_Tiao, 4));
		tPeer.addDistributeCard(CMJCard::makeCardNumber(eCT_Tiao, 4));

		tPeer.addDistributeCard(CMJCard::makeCardNumber(eCT_Wan, 1));
		tPeer.addDistributeCard(CMJCard::makeCardNumber(eCT_Wan, 2));
		tPeer.addDistributeCard(CMJCard::makeCardNumber(eCT_Wan, 1));
		tPeer.addDistributeCard(CMJCard::makeCardNumber(eCT_Wan, 2));
		tPeer.addDistributeCard(CMJCard::makeCardNumber(eCT_Wan, 3));
		tPeer.addDistributeCard(CMJCard::makeCardNumber(eCT_Wan, 3));

		tPeer.addDistributeCard(CMJCard::makeCardNumber(eCT_Wan, 5));
		tPeer.addDistributeCard(CMJCard::makeCardNumber(eCT_Wan, 5));

		tPeer.addDistributeCard(CMJCard::makeCardNumber(eCT_Jian, 2));
		tPeer.addDistributeCard(CMJCard::makeCardNumber(eCT_Jian, 2));
		tPeer.addDistributeCard(CMJCard::makeCardNumber(eCT_Jian, 2));

		tPeer.addDistributeCard(CMJCard::makeCardNumber(eCT_Jian, 1));
		tPeer.addDistributeCard(CMJCard::makeCardNumber(eCT_Jian, 1));
		tPeer.onPeng(CMJCard::makeCardNumber(eCT_Jian, 1));

		tPeer.addDistributeCard(CMJCard::makeCardNumber(eCT_Wan, 9));
		tPeer.addDistributeCard(CMJCard::makeCardNumber(eCT_Wan, 9));
		tPeer.onPeng(CMJCard::makeCardNumber(eCT_Wan, 9));


		//tPeer.addDistributeCard(CMJCard::makeCardNumber(eCT_Tong, 2));
		//tPeer.addDistributeCard(CMJCard::makeCardNumber(eCT_Tong, 2));
		//tPeer.addDistributeCard(CMJCard::makeCardNumber(eCT_Tong, 2));
		//tPeer.addDistributeCard(CMJCard::makeCardNumber(eCT_Tong, 2));
		//tPeer.onAnGang(CMJCard::makeCardNumber(eCT_Tong, 2), CMJCard::makeCardNumber(eCT_Tiao, 1));

		//tPeer.addDistributeCard(CMJCard::makeCardNumber(eCT_Wan, 5));
		//tPeer.addDistributeCard(CMJCard::makeCardNumber(eCT_Wan, 5));
		//tPeer.addDistributeCard(CMJCard::makeCardNumber(eCT_Wan, 5));
		//tPeer.addDistributeCard(CMJCard::makeCardNumber(eCT_Wan, 5));
		//tPeer.onAnGang(CMJCard::makeCardNumber(eCT_Wan, 5), CMJCard::makeCardNumber(eCT_Tiao, 2));

		tPeer.addDistributeCard(CMJCard::makeCardNumber(eCT_Jian, 3));
		tPeer.addDistributeCard(CMJCard::makeCardNumber(eCT_Jian, 3));
		tPeer.addDistributeCard(CMJCard::makeCardNumber(eCT_Jian, 3));
		tPeer.addDistributeCard(CMJCard::makeCardNumber(eCT_Jian, 3));
		tPeer.onAnGang(CMJCard::makeCardNumber(eCT_Jian, 3), CMJCard::makeCardNumber(eCT_Tiao, 4));

		//tPeer.addDistributeCard(CMJCard::makeCardNumber(eCT_Wan, 6));

		//tPeer.addDistributeCard(CMJCard::makeCardNumber(eCT_Jian, 2));
		//tPeer.addDistributeCard(CMJCard::makeCardNumber(eCT_Jian, 2));
		//tPeer.addDistributeCard(CMJCard::makeCardNumber(eCT_Jian, 2));


		//tPeer.addDistributeCard(CMJCard::makeCardNumber(eCT_Tong, 5));
		//tPeer.addDistributeCard(CMJCard::makeCardNumber(eCT_Tong, 5));
		//tPeer.onPeng(CMJCard::makeCardNumber(eCT_Tong, 5));

		//tPeer.addDistributeCard(CMJCard::makeCardNumber(eCT_Tiao, 5));
		//tPeer.addDistributeCard(CMJCard::makeCardNumber(eCT_Tiao, 5));
		//tPeer.onPeng(CMJCard::makeCardNumber(eCT_Tiao, 5));
		//tPeer.addDistributeCard(CMJCard::makeCardNumber(eCT_Tiao, 7));
		//tPeer.addDistributeCard(CMJCard::makeCardNumber(eCT_Tiao, 7));
		//tPeer.onPeng(CMJCard::makeCardNumber(eCT_Tiao, 7));
		//tPeer.addDistributeCard(CMJCard::makeCardNumber(eCT_Wan, 7));
		//tPeer.addDistributeCard(CMJCard::makeCardNumber(eCT_Wan, 7));
		//tPeer.onPeng(CMJCard::makeCardNumber(eCT_Wan, 7));
		
		tPeer.onDoHu(CMJCard::makeCardNumber(eCT_Tiao, 4), 0);
		auto nHuCnt = tPeer.getFinalHuCnt(true);
}

#include "MJServer.h"
#include "Application.h"
int main()
{
	tempTest();
	CApplication theAplication(CMJServerApp::getInstance());
	theAplication.startApp();
	return 0;
}