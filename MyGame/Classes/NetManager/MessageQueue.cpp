#include "MessageQueue.h"
#include <stdlib.h>
	MessageQueue::MessageQueue()
	{
		pthread_mutex_init(&locker, NULL);
	}
	MessageQueue::~MessageQueue()
	{
		clear();
		pthread_mutex_destroy(&locker);
	}
	void MessageQueue::push_back(nullmsg* msg)
	{
		pthread_mutex_lock(&locker);
		msglist.push_back(msg);
		pthread_mutex_unlock(&locker);
	}
	void MessageQueue::push_front(nullmsg* msg)
	{
		pthread_mutex_lock(&locker);
		msglist.push_front(msg);
		pthread_mutex_unlock(&locker);
	}
	nullmsg* MessageQueue::pop_front()
	{
		if (empty()) { return NULL; }

		nullmsg* msg = NULL;
		pthread_mutex_lock(&locker);
		msg = msglist.front();
		msglist.pop_front();
		pthread_mutex_unlock(&locker);

		return msg;
	}
	bool MessageQueue::empty()
	{
		return msglist.empty();
	}
	void MessageQueue::clear()
	{
		nullmsg* msg = pop_front();
		while(msg)
		{
			free(msg);
			msg = pop_front();
		}
	}
