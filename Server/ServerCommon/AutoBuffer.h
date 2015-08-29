#pragma once
#include <cstring>
class CAutoBuffer
{
public:
	CAutoBuffer( int nSize )
	{
		if ( nSize < 2 )
		{
			nSize = 2 ;
		}
		nAllSize = nSize ;
		pInnerBuffer = new char[nAllSize];
		
		nContentSize = 0 ;
	}

	~CAutoBuffer()
	{
		delete[] pInnerBuffer ;
		pInnerBuffer = nullptr ;
	}

	int addContent(const char* pData, int nLen )
	{
		if ( nLen + nContentSize > nAllSize )
		{
			nAllSize = nLen + nContentSize;
			nAllSize *= 2 ;
			char* pB = new char[nAllSize];
			memcpy(pB,pInnerBuffer,nContentSize);
			delete[] pInnerBuffer ;
			pInnerBuffer = pB ;
		}
		memcpy(pInnerBuffer + nContentSize , pData,nLen);
		nContentSize += nLen ;
		return nContentSize ;
	}

	int getContentSize()
	{
		return nContentSize ;
	}

	const char* getBufferPtr()
	{
		return pInnerBuffer ;
	}

private:
	int nAllSize ;
	int nContentSize ;
	char* pInnerBuffer ;
};