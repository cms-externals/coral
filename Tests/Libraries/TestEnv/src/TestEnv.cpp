// Include files
#include <cstdlib>
#include <cstring>
#include <iostream>
#include <vector>
#include <sys/stat.h>
#include "CoralBase/Exception.h"
#include "CoralBase/MessageStream.h"
#include "TestEnv/TestEnv.h"

// Local include files
#include "CommandOptions.h"
#include "PHash.hpp"

// Use std::cout for info output (instead of MessageStream with 'Always' level)
#define LOG( msg ){ std::cout << "TestEnv " << msg << std::endl; }

//-----------------------------------------------------------------------------

TestEnv::TestEnv(std::string testName)
  : m_testName( testName )
  , m_env( false )
  , m_lookupfile( "" )
{
  setupEnv();
}

//-----------------------------------------------------------------------------

TestEnv::~TestEnv()
{
}

//-----------------------------------------------------------------------------

void
TestEnv::setupEnv()
{
  if( m_env ) return;

  // Get all environment variables
  const char* path_lookup = ::getenv( "CORAL_DBLOOKUP_PATH" );
  const char* path_auth = ::getenv( "CORAL_AUTH_PATH" );
  const char* tns_admin = ::getenv( "TNS_ADMIN" );

  // Set CORAL_AUTH_PATH if not set
  if( !path_auth )
  {
#ifndef _WIN32
    ::putenv( (char*)"CORAL_AUTH_PATH=/afs/cern.ch/sw/lcg/app/pool/db" );
    path_auth = ::getenv( "CORAL_AUTH_PATH" );
    if( !path_auth )
      throw coral::Exception( "CORAL_AUTH_PATH can't be set!",
                              "TestEnv",
                              "TestEnv::setupEnv" );
#else
    //::_putenv( const_cast<char*>( "CORAL_AUTH_PATH=/afs/cern.ch/sw/lcg/app/pool/db" ) ); // This AFS path will not work on Windows...
    throw coral::Exception( "CORAL_AUTH_PATH is not set!",
                            "TestEnv",
                            "TestEnv::setupEnv" );
#endif
  }
  // Set CORAL_DBLOOKUP_PATH if not set
  if ( !path_lookup )
  {
#ifndef _WIN32
    ::putenv( (char*)"CORAL_DBLOOKUP_PATH=/afs/cern.ch/sw/lcg/app/pool/db" );
    path_lookup = ::getenv( "CORAL_DBLOOKUP_PATH" );
    if( !path_lookup )
      throw coral::Exception( "CORAL_DBLOOKUP_PATH can't be set!",
                              "TestEnv",
                              "TestEnv::setupEnv" );
#else
    //::_putenv( const_cast<char*>( "CORAL_LOOKUP_PATH=/afs/cern.ch/sw/lcg/app/pool/db" ) ); // This AFS path will not work on Windows...
    throw coral::Exception( "CORAL_DBLOOKUP_PATH is not set!",
                            "TestEnv",
                            "TestEnv::setupEnv" );
#endif
  }
#ifndef _WIN32
  // Take TNS_ADMIN from AFS on unix
  // (only if it is not already set)
  if ( !tns_admin )
    ::putenv( (char*)"TNS_ADMIN=/afs/cern.ch/project/oracle/admin" );
#else
  // Take TNS_ADMIN from DFS on Windows
  // (override it even if it is already set: fix bug #64017 aka bug #71969)
  ::_putenv( (char*)"TNS_ADMIN=\\\\cern.ch\\dfs\\Applications\\Oracle\\ADMIN" );
#endif
  tns_admin = ::getenv( "TNS_ADMIN" );
  if( !tns_admin )
    throw coral::Exception( "TNS_ADMIN can't be set!",
                            "TestEnv",
                            "TestEnv::setupEnv" );

  /*
  // Test if the specialised dblookup file exists (patch for bug #64446)
  // Disable this for the moment because another patch exists in CoralAccess
  if( slot_name && slot_nmbr )
  {
    std::string filename("");
    // Construct the filename
    filename.append(path_lookup);
    filename.append("/");
    filename.append("dblookup_");
    filename.append(slot_name);
    filename.append(slot_nmbr);
    filename.append(".xml");
    // Check if file exists and can be opened
    struct stat results;
    if( ::stat(filename.c_str(), &results ) == 0 ) m_lookupfile = filename;
  }
  */

  m_env = true;

}

//-----------------------------------------------------------------------------

void
TestEnv::dumpEnv()
{
  if(!m_env)
    throw coral::Exception( "TestEnv",
                            "TestEnv was not initialized",
                            "TestEnv::dumpEnv" );

  // Get all environment variables
  const char* slot_name = ::getenv( "LCG_NGT_SLT_NAME" );
  const char* slot_nmbr = ::getenv( "LCG_NGT_SLT_NUM" );
  const char* cmtconfig = ::getenv( "CMTCONFIG" );
  const char* path_lookup = ::getenv( "CORAL_DBLOOKUP_PATH" );
  const char* path_auth = ::getenv( "CORAL_AUTH_PATH" );
  const char* tns_admin = ::getenv( "TNS_ADMIN" );

  LOG("+------------------------------------------------------------------");
  LOG("| UrlRW : '" << UrlRW() << "'");
  LOG("| UrlRO : '" << UrlRO() << "'");
  LOG("+------------------------------------------------------------------");
  if( m_lookupfile != "" )
  {
    LOG("| CORAL_DBLOOKUP_PATH : '" << path_lookup
        << "' with specialisation '" << m_lookupfile << "'" );
  }
  else if( slot_name && slot_nmbr )
  {
    LOG("| CORAL_DBLOOKUP_PATH : '" << path_lookup
        << "' (no specialisation found)" );
  }
  else
  {
    LOG("| CORAL_DBLOOKUP_PATH : '" << path_lookup
        << "' (no slotname or slotnumber found)" );
  }
  LOG("| CORAL_AUTH_PATH     : '" << path_auth << "'" );
  LOG("| TNS_ADMIN           : '" << tns_admin << "'" );
  LOG("+------------------------------------------------------------------");
  if ( ::getenv( "LD_LIBRARY_PATH" ) )
    LOG("| LD_LIBRARY_PATH     : '" << ::getenv( "LD_LIBRARY_PATH" ) << "'" );
  if ( ::getenv( "DYLD_LIBRARY_PATH" ) )
    LOG("| DYLD_LIBRARY_PATH   : '" << ::getenv( "DYLD_LIBRARY_PATH" ) << "'" );
  if ( ::getenv( "PATH" ) )
    LOG("| PATH                : '" << ::getenv( "PATH" ) << "'" );
  LOG("+------------------------------------------------------------------");
  LOG("| TestEnv name        : " << m_testName);
  LOG("+------------------------------------------------------------------");
  LOG("| LCG_NGT_SLT_NAME    : " << ( slot_name ? slot_name : "" ) );
  LOG("| LCG_NGT_SLT_NUM     : " << ( slot_nmbr ? slot_nmbr : "" ) );
  LOG("| CMTCONFIG           : " << ( cmtconfig ? cmtconfig : "" ) );
  LOG("| --> table prefix    : '" << BuildUniqueTableName( m_testName + "_X" ) );
  LOG("+------------------------------------------------------------------");
}

//-----------------------------------------------------------------------------

void
TestEnv::dumpUsage()
{
  std::cout << "TestEnv framework for the Coral Project, Version 0.2.0" << std::endl;
  std::cout << "Usage: " << " [WRITER:<READER>]" << std::endl << std::endl;
  std::cout << "Possible combinations:" << std::endl;
  std::cout << "\toracle" << std::endl;
  std::cout << "\toracle:coral" << std::endl;
  std::cout << "\toracle:frontier" << std::endl;
  std::cout << "\tmysql" << std::endl;
  std::cout << "\tmysql:coral" << std::endl;
  std::cout << "\tsqlite" << std::endl << std::endl;
}

//-----------------------------------------------------------------------------

bool
TestEnv::check( int argc, char* argv[] )
{
  if ( !ParseArguments( argc, argv ) ) return false;
  /* print the environment variables */
  dumpEnv();
  /* use a user defined lock */
  setupUserLock();
  /* return true to run the test */
  return true;
}

//-----------------------------------------------------------------------------

size_t
TestEnv::addServiceName(const char* writerexp, const char* readerexp)
{
  if( readerexp && writerexp )
  {
    std::string writer_expression = UrlRW(); ///FIXME + '/' + std::string(writerexp);
    std::string reader_expression = UrlRO(); ///FIXME + '/' + std::string(readerexp);
    m_services.push_back(std::pair<std::string, std::string>(writer_expression, reader_expression));
    LOG("| ServiceName [writer]: " << writer_expression);
    LOG("| ServiceName [reader]: " << reader_expression);
    LOG("+------------------------------------------------------------------");
  }
  else if( writerexp )
  {
    std::string writer_expression = UrlRW(); ///FIXME + '/' + std::string(writerexp);
    m_services.push_back(std::pair<std::string, std::string>(writer_expression, writer_expression));
    LOG("| ServiceName [writer]: " << writer_expression);
    LOG("| ServiceName [reader]: " << writer_expression);
    LOG("+------------------------------------------------------------------");
  }
  else
  {
    LOG("| ServiceName [writer]: NONE");
    LOG("| ServiceName [reader]: NONE");
    LOG("+------------------------------------------------------------------");
    throw coral::Exception( "TestEnv", "ServiceName with NULL write or read extension", "TestEnv::addServiceName" );
  }
  return m_services.size() - 1;
}

//-----------------------------------------------------------------------------

const std::string
TestEnv::getServiceName(size_t index, coral::AccessMode mode) const
{
  if( index >= m_services.size() )
    throw coral::Exception( "TestEnv", "Index out of range", "TestEnv::getServiceName" );

  if( mode == coral::ReadOnly ) return m_services[index].second;
  else return m_services[index].first;
}

//-----------------------------------------------------------------------------

void
TestEnv::addTablePrefix( std::string& fullName,
                         const std::string& shortName ) const
{
  fullName = BuildUniqueTableName( m_testName + "_" + shortName );
  LOG("+------------------------------------------------------------------");
  LOG("| Table: " << fullName);
  LOG("+------------------------------------------------------------------");
}

//-----------------------------------------------------------------------------

void
TestEnv::setEnv( const std::string& key, const std::string& value )
{
  m_evars.push_back( std::pair<std::string, std::string>(key, value) );
}

//-----------------------------------------------------------------------------
