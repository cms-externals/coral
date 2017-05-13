#ifndef CORALBASE_SERVICE_H
#define CORALBASE_SERVICE_H

#include "ILoadableComponent.h"

#include <map>
#include <stdexcept>

namespace coral
{

  class MessageStream;

  class Service : public ILoadableComponent
  {
  protected:
    /// Constructor
    explicit Service( const std::string& name );

    /// Destructor
    virtual ~Service();

    /// Returns the underlying message stream object
    MessageStream& log() {
      return *m_log;
    }

    /// Returns the underlying message stream object
    MessageStream& log() const {
      return *m_log;
    }

  private:
    /// The message stream
    mutable MessageStream*              m_log;
  };

}

#endif
