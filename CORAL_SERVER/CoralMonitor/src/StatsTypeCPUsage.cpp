#include "CoralMonitor/StatsTypeCPUsage.h"
#include "CoralMonitor/StatsStorage.h"

#include <sstream>
#include <iostream>
#include <limits.h>

#include "CPUsageData.h"


namespace coral {

  StatsTypeCPUsage::StatsTypeCPUsage()
    : IStatsType( "CPUsage" )
    , m_mutex( new pthread_mutex_t )
    , m_data( new CPUsageData )
  {
    //initialise the mutex
    pthread_mutex_t semaphore_mutex = PTHREAD_MUTEX_INITIALIZER;
    *m_mutex = semaphore_mutex;
#ifdef CORALSERVERBASE_STATSSTORAGE_H
    StatsStorageInstance().add( this );
#endif
  }

  StatsTypeCPUsage::~StatsTypeCPUsage()
  {
    delete m_mutex;
    delete m_data;
  }

  void
  StatsTypeCPUsage::getFiguresDesc(std::vector<std::string>& desc)
  {
    desc.push_back("CPU[system] usage in %");
    desc.push_back("CPU[user] usage in %");
    desc.push_back("CPU[nice] usage in %");
    desc.push_back("CPU[total] usage in %");
  }

  void
  StatsTypeCPUsage::getFiguresData(std::vector<double>& data)
  {
    CPUsageContainer container;

    m_data->getUsage(container);

    data.push_back( container.system );
    data.push_back( container.user );
    data.push_back( container.nice );
    data.push_back( container.nice + container.system + container.user );
  }

}
