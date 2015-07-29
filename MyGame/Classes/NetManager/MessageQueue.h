#ifndef __MESSAGE_QUEUE_H__
#define __MESSAGE_QUEUE_H__
#include <list>
#include "Thread.h"
struct nullmsg;

	typedef std::list<nullmsg*>	msg_list_t;
	class MessageQueue
	{
        private:
	        pthread_mutex_t						locker;
		    msg_list_t							msglist;
        public:
		    MessageQueue();
		    ~MessageQueue();
		    void								push_back(nullmsg* msg);
		    void								push_front(nullmsg* msg);
		    nullmsg*					pop_front();
		    bool								empty();
		    void								clear();
	};


#endif