#ifndef CORAL_CORALBASE_MSGREPORTER2_H
#define CORAL_CORALBASE_MSGREPORTER2_H 1

#include "CoralBase/boost_thread_headers.h"
#include "CoralBase/MessageStream.h"

namespace coral
{

  namespace CoralBase
  {

    /** @class MsgReporter2 MsgReporter2.h
     *
     * Thread-safe reporter implementation
     *
     * @author   Markus Frank, Martin Wache, Andrea Valassi
     */

    class MsgReporter2 : public IMsgReporter
    {

    public:

      /// Default constructor
      MsgReporter2();

      /// Destructor
      virtual ~MsgReporter2() {}

      /// Release reference to reporter
      virtual void release()
      {
        delete this;
      }

      /// Access output level
      virtual coral::MsgLevel outputLevel() const;

      /// Modify output level
      virtual void setOutputLevel( coral::MsgLevel lvl );

      /// Report a message
      virtual void report( int lvl,
                           const std::string& src,
                           const std::string& msg );

    private:

      /// The current message level threshold
      coral::MsgLevel m_level;

      /// Use a different format output
      size_t m_format;

      /// The mutex lock
      boost::mutex m_mutex;

    };

  }

}
#endif
