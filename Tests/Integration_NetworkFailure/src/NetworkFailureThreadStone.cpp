#include "NetworkFailureThreadStone.h"
#include "TestEnv/TestingConfig.h"
#include "StoneHandler.h"
#include "TestEnv/SealSwitch.h"
#include "TestEnv/Testing.h"
#include "RelationalAccess/ISchema.h"

#include <stdexcept>
#include <sstream>
#include <memory>
#include <cmath>
#include "CoralBase/MessageStream.h"

#define LOG( msg ){ coral::MessageStream myMsg("NETFAULT"); myMsg << coral::Always << msg << coral::MessageStream::endmsg; }

void NetworkFailureThreadStone::operator()()
{
  LOG( "***** ThreadStone started *****" );

  coral::sleepSeconds( 4 );

  m_sh.deactivate();


  coral::sleepSeconds( 10 );

  m_sh.activate();

  coral::sleepSeconds( 2 );

  m_sh.deactivate();


  coral::sleepSeconds( 10 );

  m_sh.activate();
}
