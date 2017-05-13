#include "CoralKernel/Service.h"
#include "CoralKernel/Context.h"

#include "CoralBase/MessageStream.h"

coral::Service::Service( const std::string& name ) :
  coral::ILoadableComponent( name ),
  m_log( new coral::MessageStream( name ) )
{
}

coral::Service::~Service()
{
  if ( m_log )
    delete m_log;
  m_log = 0;
}
