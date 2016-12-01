#pragma once
#include "NativeTypes.h"
#include "json/json.h"
#include <memory>
class IGameRecorderEntry
{
public:
	typedef std::shared_ptr<IGameRecorderEntry> shared_ptr;
public:
	virtual ~IGameRecorderEntry(){}
	virtual uint32_t getPlayerUID() = 0 ;
	virtual int32_t getOffset() = 0 ;
	virtual void serialization(Json::Value& js) = 0;
	virtual bool deserialization(Json::Value& js) = 0;
};

class IGameRecorder
{
public:
	typedef std::shared_ptr<IGameRecorder> shared_ptr;
public:
	virtual ~IGameRecorder();
	virtual uint32_t getRoomID() = 0;
	virtual uint32_t getRoomType() = 0;
	virtual uint32_t getTimestamp() = 0 ;
	virtual IGameRecorderEntry::shared_ptr getRecorderEntry(uint32_t nPlayerUID) = 0;

	virtual void serialization(Json::Value& js) = 0;
	virtual bool deserialization(Json::Value& js) = 0;
};