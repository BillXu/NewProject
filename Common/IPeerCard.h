#pragma once
#include "NativeTypes.h"
#include <cassert>
class IPeerCard
{
public:
	enum  PK_RESULT
	{
		PK_RESULT_FAILED,
		PK_RESULT_WIN,
		PK_RESULT_EQUAL,
	};
public:
	virtual ~IPeerCard(){}
	virtual void reset() = 0 ;
	virtual void addCompositCardNum( uint8_t nCardCompositNum ) = 0 ;
	virtual  const char* getNameString() = 0 ;
	virtual  uint32_t getWeight() = 0 ;
	PK_RESULT pk( IPeerCard* pTarget )
	{
		assert(pTarget && "pk target is null" );
		if ( getWeight() > pTarget->getWeight() )
		{
			return PK_RESULT_WIN ;
		}
		else if ( getWeight() == pTarget->getWeight() )
		{
			return PK_RESULT_EQUAL ;
		}
		return PK_RESULT_FAILED ;
	}

	bool operator < (IPeerCard& refTarget )
	{
		return getWeight() < refTarget.getWeight() ;
	}

	bool operator > (IPeerCard& refTarget)
	{
		return getWeight() > refTarget.getWeight() ;
	}

	bool operator == (IPeerCard& refTarget)
	{
		return getWeight() == refTarget.getWeight() ;
	}
};