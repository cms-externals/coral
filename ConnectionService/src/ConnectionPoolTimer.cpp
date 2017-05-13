#include "ConnectionPoolTimer.h"
#include "ConnectionPool.h"
#include "CoralCommon/Utilities.h"

//-----------------------------------------------------------------------------

coral::ConnectionService::ConnectionPoolTimer::ConnectionPoolTimer()
  : m_pool(0)
{
}

//-----------------------------------------------------------------------------

coral::ConnectionService::ConnectionPoolTimer::ConnectionPoolTimer( ConnectionPool& pool )
  : m_pool(&pool)
{
}

//-----------------------------------------------------------------------------

coral::ConnectionService::ConnectionPoolTimer::~ConnectionPoolTimer()
{
}

//-----------------------------------------------------------------------------

void
coral::ConnectionService::ConnectionPoolTimer::operator()()
{
  if(m_pool)
  {
    while( m_pool->waitForTimeout() )
    {
      m_pool->cleanUpTimedOutConnections();
    }
  }
}

//-----------------------------------------------------------------------------
