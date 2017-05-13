#include "StoneHandler.h"

#include "TestEnv/TestingConfig.h"
#include <iostream>

#ifndef _WIN32
#include <sys/wait.h>
#endif

#include <signal.h>

#include "CoralBase/MessageStream.h"

#define LOG( msg ){ coral::MessageStream myMsg("NETFAULT"); myMsg << coral::Always << msg << coral::MessageStream::endmsg; }

//-----------------------------------------------------------------------------

StoneHandler::StoneHandler()
  : m_cmd( "" )
  , m_pid( 0 )
{
#ifndef _WIN32
  // Set the command
  m_cmd = "coral_stone";
  // Kill all other versions
  if( system ( "killall coral_stone" ) == 0)
  {
    LOG( "Start StoneHandler (killed previous stone instance)" );
  }
  else
  {
    LOG( "Start StoneHandler" );
  }
#endif
}

//-----------------------------------------------------------------------------

StoneHandler::~StoneHandler()
{
  if( m_pid )
  {
    deactivate();
  }
}

//-----------------------------------------------------------------------------

void
StoneHandler::activate()
{
#ifndef _WIN32
  // Do a fork
  m_pid = fork();
  if( m_pid == 0 )
  {
    // Child process
    // Start the stone program
    int ret = execl(m_cmd.c_str(), m_cmd.c_str(), "lcgaar1-v.cern.ch:10121", "localhost:45000", (char *) 0);
    if( ret != 0 )
    {
      LOG( "Can't execute " << m_cmd << " with error " << strerror(ret));
      exit(1);
    }
  }
#endif
}

//-----------------------------------------------------------------------------

void
StoneHandler::deactivate()
{
#ifndef _WIN32
  if( m_pid )
  {
    // Send interupt signal
    kill( m_pid, SIGINT);

    int counter = 0;
    while( (waitpid( m_pid, NULL, WNOHANG) < 1) && (counter < 10) )
    {
      sleep(1);
      counter++;
    }
    if(waitpid( m_pid, NULL, WNOHANG) != 0)
      return;

    // Send kill signal
    kill( m_pid, SIGKILL);

    counter = 0;
    while( (waitpid( m_pid, NULL, WNOHANG) < 1) && (counter < 5) )
    {
      sleep(1);
      counter++;
    }
    if(waitpid( m_pid, NULL, WNOHANG) != 0)
      return;
    // We produced a Zombie process
    throw;
  }
  else
  {
    LOG( "No Child PID exists");

    throw;
  }
#endif
}

//-----------------------------------------------------------------------------
