#include "IPlayerComponent.h"
#include "Player.h"
IPlayerComponent::IPlayerComponent(CPlayer* pPlayer )
	:m_pPlayer(pPlayer),m_eType(ePlayerComponent_None)
{

}

IPlayerComponent::~IPlayerComponent()
{

}

void IPlayerComponent::SendMsgToClient(const char* pbuffer , unsigned short nLen , bool bBrocast  )
{
	m_pPlayer->SendMsgToClient(pbuffer,nLen,bBrocast);
}

void IPlayerComponent::SendMsgToDB(const char* pbuffer , unsigned short nLen)
{
	m_pPlayer->SendMsgToDBServer(pbuffer,nLen) ;
}
