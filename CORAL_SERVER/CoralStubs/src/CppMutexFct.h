#ifndef CORAL_CORALSTUBS_CPPMUTEXIN_H
#define CORAL_CORALSTUBS_CPPMUTEXIN_H

#include "CppMutex.h"

namespace coral {

  namespace CoralStubs {

#ifdef _WIN32

    inline void initMutex(pmutex_t* pMutex)
    {
      InitializeCriticalSection(pMutex);
    };

    inline void lockMutex(pmutex_t* pMutex)
    {
      EnterCriticalSection(pMutex);
    };

    inline void unlockMutex(pmutex_t* pMutex)
    {
      LeaveCriticalSection(pMutex);
    };

#else

    //static pthread_mutex_t semaphore_mutex = PTHREAD_MUTEX_INITIALIZER;

    inline void initMutex(pmutex_t* pMutex)
    {
      pthread_mutex_t semaphore_mutex = PTHREAD_MUTEX_INITIALIZER;
      *pMutex = semaphore_mutex;
    }

    inline void lockMutex(pmutex_t* pMutex)
    {
      pthread_mutex_lock(pMutex);
    }

    inline void unlockMutex(pmutex_t* pMutex)
    {
      pthread_mutex_unlock(pMutex);
    }

#endif

  }

}

#endif
