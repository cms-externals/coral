#ifndef CORAL_CORALSTUBS_TESTS_BYTEBUFFERPOOLSIMPLE_H
#define CORAL_CORALSTUBS_TESTS_BYTEBUFFERPOOLSIMPLE_H

#include "CoralServerBase/ByteBuffer.h"

#include "CppMutex.h"

#define MAXAMOUNT_OF_STATICBUFFERS 1000

namespace coral {

  namespace CoralStubs {

    /** @class StaticBufferPool
     *
     *  A static pool of 20 ByteBuffers
     *
     *  @author Alexander Kalkhof
     *  @date   2009-02-09
     */

    class StaticBufferPool {
    public:

      StaticBufferPool();

      ~StaticBufferPool();

      ByteBuffer& allocateBuffer();

      void releaseBuffer( const ByteBuffer& buffer );

      bool isEmpty();

    private:

      ByteBuffer* s_buffer[MAXAMOUNT_OF_STATICBUFFERS + 1];

      bool s_allocated[MAXAMOUNT_OF_STATICBUFFERS + 1];

      pmutex_t* m_mutex;

    };

  }

}

#endif
