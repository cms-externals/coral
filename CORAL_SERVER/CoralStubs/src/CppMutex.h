#ifndef CORAL_CORALSTUBS_CPPMUTEX_H
#define CORAL_CORALSTUBS_CPPMUTEX_H

namespace coral {

  namespace CoralStubs {

#ifdef _WIN32

#include <windows.h>

    typedef CRITICAL_SECTION pmutex_t;

#else

#include <pthread.h>

    typedef pthread_mutex_t pmutex_t;

#endif

  }

}

#endif
