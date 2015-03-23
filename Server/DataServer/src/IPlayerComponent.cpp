#include "IPlayerComponent.h"
#include "Player.h"
IPlayerComponent::IPlayerComponent(CPlayer* pPlayer )
	:m_pPlayer(pPlayer),m_eType(ePlayerComponent_None)
{

}

IPlayerComponent::~IPlayerComponent()
{

}

void IPlayerComponent::SendMsg(stMsg* pbuffer , unsigned short nLen , bool bBrocast  )
{
	m_pPlayer->SendMsgToClient((char*)pbuffer,nLen,bBrocast);
}

