#include "StaticBufferPool.h"

#include "CppMutexFct.h"

#include <iostream>

#include "CoralBase/Exception.h"
#include "CoralServerBase/CTLPacketHeader.h"

#define LOGGER_NAME "CoralStubs::StaticBufferPool"
#include "logger.h"


namespace coral { namespace CoralStubs {

  StaticBufferPool::StaticBufferPool()
    : m_mutex( new pmutex_t )
  {
    //logger << "Create StaticBufferPool" << endlog;
    initMutex(m_mutex);
    for(size_t i = 0; i < MAXAMOUNT_OF_STATICBUFFERS; i++ ) s_buffer[i] = NULL;
    for(size_t i = 0; i < MAXAMOUNT_OF_STATICBUFFERS; i++ ) s_allocated[i] = false;
  }

  StaticBufferPool::~StaticBufferPool()
  {
    //logger << "Delete StaticBufferPool..." << endlog;

    if(!isEmpty()) {
      throw Exception("StaticBufferPool is not empty","StaticBufferPool::~StaticBufferPool","coral::CoralStubs");
    }
    //logger << "Delete StaticBufferPool... 1" << endlog;
    for(size_t i = 0; i < MAXAMOUNT_OF_STATICBUFFERS; i++ )
    {
      if(s_buffer[i] != NULL) delete s_buffer[i];
    }
    //logger << "Delete StaticBufferPool... 2" << endlog;
    delete m_mutex;
    //logger << "Delete StaticBufferPool... DONE" << endlog;
  }

  ByteBuffer&
  StaticBufferPool::allocateBuffer()
  {
    lockMutex(m_mutex);
    for(size_t i = 0; i < MAXAMOUNT_OF_STATICBUFFERS; i++ )
    {
      if(!s_allocated[i]) {
        s_allocated[i] = true;
        unlockMutex(m_mutex);
        //alocate the default max size plus 10mb extra
        if(s_buffer[i] == NULL) s_buffer[i] = new ByteBuffer(CTLPACKET_MAX_SIZE + 10000000);
        //      std::cout << "Allocate New ByteBuffer " << s_buffer[i] << std::endl;
        //reset the buffer to startpos zero
        s_buffer[i]->setUsedSize(0);
        return *s_buffer[i];
      }
    }
    unlockMutex(m_mutex);
    throw Exception("PANIC!!! No buffers available","StaticBufferPool::allocateBuffer","coral::CoralStubs");
  }

  void
  StaticBufferPool::releaseBuffer( const ByteBuffer& buffer )
  {
    lockMutex(m_mutex);
    for(size_t i = 0; i < MAXAMOUNT_OF_STATICBUFFERS; i++ )
    {
      if(s_buffer[i] == &buffer) {
        //      std::cout << "Delete ByteBuffer " << s_buffer[i] << std::endl;
        s_allocated[i] = false;
      }
    }
    unlockMutex(m_mutex);
  }

  bool
  StaticBufferPool::isEmpty()
  {
    lockMutex(m_mutex);
    for(size_t i = 0; i < MAXAMOUNT_OF_STATICBUFFERS; i++ )
    {
      if(s_allocated[i])
      {
        logger << Debug << "found not freed buffer " << s_buffer[i] << endlog;
        return false;
      }
    }
    unlockMutex(m_mutex);
    return true;
  }

} }
