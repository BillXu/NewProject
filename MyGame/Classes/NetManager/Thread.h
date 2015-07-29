#ifndef __THREAD_H__
#define __THREAD_H__

#ifdef WIN32
    #include <windows.h>
    #include <process.h>
#else
    #include <pthread.h>
#endif

#include <errno.h>

namespace Misc
{
    #ifdef WIN32
        #define SLEEP_MS(t) Sleep(t)
    #else
        #define SLEEP_MS(t) usleep((t)*1000)
    #endif

    #ifdef _WIN32
        // to be
        //typedef DWORD  threadfunc_t;
        //typedef LPVOID threadparam_t;
        //#define STDPREFIX WINAPI
        typedef unsigned threadfunc_ex_t;
        typedef void * threadparam_ex_t;
        #define STDPREFIX_EX __stdcall
    #else
        typedef void * threadfunc_ex_t;
        typedef void * threadparam_ex_t;
        #define STDPREFIX_EX
    #endif

    class Thread
    {
	    protected:
		    //-- 子类需要实现此线程函数 --//
		    virtual void Run() = 0;     
	    public:
		    //-- 用户代码开始执行线程时调用此函数 --//
		    virtual bool runThread()
		    {
	    #ifdef _WIN32
			    //	m_thread = ::CreateThread(NULL, 0, StartThread, this, 0, &m_dwThreadId);
			    m_thread = (HANDLE)_beginthreadex(NULL, 0, &StartThread, this, 0, &m_dwThreadId);
	    #else
			    pthread_attr_init(&m_attr);
			    pthread_attr_setdetachstate(&m_attr, PTHREAD_CREATE_DETACHED);
			    if (pthread_create(&m_thread, &m_attr, StartThread, this) == -1)
			    {
				    //perror("Thread: create failed");
				    //SetRunning(false);
			    }
	    #endif
			    return true;
		    }

		    void stopThread()
		    {
	    #ifdef _WIN32
			    if (m_thread)
				    CloseHandle(m_thread);
	    #else
			    pthread_attr_destroy(&m_attr);
	    #endif
	    //#ifdef WIN32
	    //        if (m_hHandle)
	    //        {
	    //            CloseHandle(m_hHandle);
	    //            m_hHandle =NULL;
	    //        }
	    //#else
	    //        // ... TODO
	    //#endif
		    }

	    public:
		    Thread(void) {};
		    virtual ~Thread(void)
		    {
			    stopThread(); 
		    };

	    private:
		    static threadfunc_ex_t STDPREFIX_EX StartThread(threadparam_ex_t pParam)
		    {
			    Thread* pThread = (Thread*)pParam;

			    if(pThread != NULL)
			    {
				    pThread->Run();
			    }

	    #ifdef _WIN32
			    _endthreadex(0);
	    #endif

			    return (threadfunc_ex_t)NULL;
		    }

	    private:
	    #ifdef _WIN32
		    HANDLE m_thread;
		    unsigned m_dwThreadId;
	    #else
		    pthread_t m_thread;
		    pthread_attr_t m_attr;
	    #endif
    };
};

#ifdef WIN32
    typedef CRITICAL_SECTION pthread_mutex_t;
    inline int pthread_mutex_lock(pthread_mutex_t *m)
    {
        EnterCriticalSection(m);
        return 0;
    }
    inline int pthread_mutex_unlock(pthread_mutex_t *m)
    {
        LeaveCriticalSection(m);
        return 0;
    }
    inline int pthread_mutex_trylock(pthread_mutex_t *m)
    {
        return TryEnterCriticalSection(m) ? 0 : EBUSY; 
    }
    inline int pthread_mutex_init(pthread_mutex_t *m, void *a)
    {
        (void) a;
        InitializeCriticalSection(m);

        return 0;
    }
    inline int pthread_mutex_destroy(pthread_mutex_t *m)
    {
        DeleteCriticalSection(m);
        return 0;
    }
#endif

#endif // __NETMANAGER_SOCKET_THREAD_T_H__
