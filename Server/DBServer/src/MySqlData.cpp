#include "MySqlData.h"

unsigned int stMysqlField::IntValue()
{ 
	return strtol(pBuffer,(char**)NULL,10) ; 
}

unsigned __int64 stMysqlField::IntValue64()
{ 
	return _atoi64(pBuffer); 
}

float stMysqlField::FloatValue()
{
	return atof(pBuffer);  
}

double stMysqlField::DoubleValue()
{ 
	return atof(pBuffer);
}

const char* stMysqlField::BufferData()
{
	return pBuffer ; 
}

void stMysqlField::VecString( VEC_STRING& vOutString ,char cSplit)
{
	StringSplit(CStringValue(), cSplit, vOutString );
}

void stMysqlField::VecInt( std::vector<int>& vOutInt,char cSplit  )
{
	VEC_STRING vOutString ;
	VecString(vOutString ,cSplit);
	VEC_STRING::iterator iter = vOutString.begin() ;
	for ( ; iter != vOutString.end(); ++iter )
	{
		vOutInt.push_back(atoi(iter->c_str()));
	}
}

std::string stMysqlField::Int64ArraryToString( int64_t vInt[] , uint16_t nCnt )
{
	std::string strResult = "" ;
	char pBuffer[100] = { 0 } ;
	for ( uint16_t nIdx = 0 ; nIdx < nCnt ; ++nIdx )
	{
		memset(pBuffer,0,sizeof(pBuffer)) ;
		if ( nIdx == 0 )
		{
			sprintf_s(pBuffer,sizeof(pBuffer),"%I64d",vInt[nIdx] ) ;
		}
		else
		{
			sprintf_s(pBuffer,sizeof(pBuffer),"%c%I64d",FILIED_SPLIT,vInt[nIdx] ) ;
		}
		strResult.append(pBuffer) ;
	}
	return strResult ;
}

void stMysqlField::VecFloat( std::vector<float>& vOutFloat,char cSplit ) 
{
	VEC_STRING vOutString ;
	VecString(vOutString ,cSplit);
	VEC_STRING::iterator iter = vOutString.begin() ;
	for ( ; iter != vOutString.end(); ++iter )
	{
		vOutFloat.push_back((float)atof(iter->c_str()));
	}
}

stMysqlField::~stMysqlField()
{
	delete[] pBuffer;
}

// row 
CMysqlRow::CMysqlRow()
{
	ClearAllFiled();
}

CMysqlRow::~CMysqlRow()
{
	ClearAllFiled() ;
}

void CMysqlRow::PushFiled(stMysqlField* pFiled )
{
	if ( pFiled )
		m_vField.push_back(pFiled);
}

stMysqlField* CMysqlRow::GetFiledByName( const char* pFiledName )
{
	if ( !pFiledName )
		return NULL ;
	LIST_FIELD::iterator iter = m_vField.begin() ;
	for ( ; iter != m_vField.end() ; ++iter )
	{
		stMysqlField* pField = *iter ;
		if ( pField == NULL )
			continue ;
		if ( strcmp(pFiledName,pField->strFieldName.c_str()) == 0 )
		{
			return pField ;
		}
	}
	return NULL ;
}

stMysqlField* CMysqlRow::operator[](const char* pFieldname )
{
	return GetFiledByName(pFieldname) ;
}

void CMysqlRow::ClearAllFiled()
{
	LIST_FIELD::iterator iter = m_vField.begin() ;
	for ( ; iter != m_vField.end() ; ++iter )
	{
		delete *iter ;
		*iter = NULL ;
	}
	m_vField.clear() ;
}