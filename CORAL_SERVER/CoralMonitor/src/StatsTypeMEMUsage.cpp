#include <cstdlib>
#include <cstdio> // For sprintf on gcc46 (bug #89762)
#include <cstring>
#include <iostream>
#include <sstream>
#include <limits.h>

#include "CoralMonitor/StatsStorage.h"
#include "CoralMonitor/StatsTypeMEMUsage.h"

namespace coral
{

  StatsTypeMEMUsage::StatsTypeMEMUsage()
    : IStatsType( "MEMUsage" )
    , m_mutex( new pthread_mutex_t )
    , m_total( 0 )
    , m_resident( 0 )
    , m_share( 0 )
    , m_text( 0 )
    , m_lib( 0 )
    , m_data( 0 )
  {
    //initialise the mutex
    pthread_mutex_t semaphore_mutex = PTHREAD_MUTEX_INITIALIZER;
    *m_mutex = semaphore_mutex;
#ifdef CORALSERVERBASE_STATSSTORAGE_H
    StatsStorageInstance().add( this );
#endif
  }

  StatsTypeMEMUsage::~StatsTypeMEMUsage()
  {
    delete m_mutex;
  }

  void
  StatsTypeMEMUsage::getFiguresDesc(std::vector<std::string>& desc)
  {
    pthread_mutex_lock(m_mutex);

    desc.push_back("Memory total");
    desc.push_back("Memory resident set size");
    desc.push_back("Memory shared pages");
    desc.push_back("Memory text (code)");
    desc.push_back("Memory lib");
    desc.push_back("Memory data");

    pthread_mutex_unlock(m_mutex);
  }

  int
  parseLine( char* line )
  {
    int i = strlen( line );
    while ( *line < '0' || *line > '9' ) line++;
    line[i-3] = '\0';
    i = atoi( line );
    return i;
  }

  void
  StatsTypeMEMUsage::getFiguresData(std::vector<double>& data)
  {
    pthread_mutex_lock(m_mutex);
    //get the values
    char buf01[30];
    char buf02[128];

    // Retrieving the page size for this system in KB
    const long PAGE_SIZE_KB = sysconf( _SC_PAGE_SIZE ) / 1024;

    // Retrieving information from STATM (unit is 'pages')
    snprintf(buf01, 30, "/proc/%d/statm", getpid() );
    FILE* pf = fopen(buf01, "r");

    if(pf)
    {
      fscanf( pf, "%llu %llu %llu %llu %llu %llu", &m_total, &m_resident, &m_share, &m_text, &m_lib, &m_data );
      fclose( pf );
      m_total = m_total * PAGE_SIZE_KB;
      m_resident = m_resident * PAGE_SIZE_KB;
      m_share = m_share * PAGE_SIZE_KB;
      m_text = m_text * PAGE_SIZE_KB;
      m_data = m_data * PAGE_SIZE_KB;
    }

    // Retrieving information from STATUS (unit is KB)
    pf = fopen( "/proc/self/status", "r" );
    while( fgets( buf02, 128, pf ) != NULL )
    {
      if( strncmp( buf02, "VmLib:", 6 ) == 0 ) m_lib = parseLine( buf02 );
    }
    fclose( pf );

    data.push_back( m_total );
    data.push_back( m_resident );
    data.push_back( m_share );
    data.push_back( m_text );
    data.push_back( m_lib );
    data.push_back( m_data );

    pthread_mutex_unlock(m_mutex);
  }

}
