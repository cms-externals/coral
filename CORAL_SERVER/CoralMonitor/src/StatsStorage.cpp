#include <cstdio> // For sprintf on gcc46 (bug #89762)
#include <cstdlib>
#include <cstring>
#include <iostream>
#include <list>
#include <sstream>

#include "CoralMonitor/StatsStorage.h"

//#define LOGGER_NAME "StatsStorage"
//#include "CoralServerBase/logger.h"

namespace coral {

  StatsStorage&
  StatsStorageInstance()
  {
    //creates a static instance which global and only once created
    static StatsStorage theInstance;
    //return
    return theInstance;
  }

  StatsStorage::StatsStorage()
  {
  }

  StatsStorage::~StatsStorage()
  {
  }

  void
  StatsStorage::add(IStatsType* st)
  {
    if(!st)
      return;
    //check if the stat type already was entered
    std::map<std::string, IStatsType*>::iterator i = m_stypes.find( st->name() );

    if(i == m_stypes.end())
      //add to the map
      m_stypes.insert( std::pair<std::string, IStatsType*>(st->name(), st) );
  }

  void
  StatsStorage::report(std::vector<std::string>& list)
  {
    std::ostringstream s;

    s << "Global Monitor for (" << m_stypes.size() << ") registered StatsTypes : ";

    list.push_back(s.str());

    std::map<std::string, IStatsType*>::iterator i;
    for(i = m_stypes.begin(); i != m_stypes.end(); ++i)
    {
      std::vector<std::string> desc;
      std::vector<double> data;

      i->second->getFiguresDesc( desc );
      i->second->getFiguresData( data );


      for(size_t j = 0; j < desc.size(); ++j )
      {
        std::ostringstream h;

        h << i->second->name() << " : " << desc[j] << " [" << data[j] << "]";
        //add value to vector
        list.push_back( h.str() );
      }
    }
  }

  void
  StatsStorage::reportDesc(std::vector<std::string>& list)
  {
    std::map<std::string, IStatsType*>::iterator i;
    for(i = m_stypes.begin(); i != m_stypes.end(); ++i)
    {
      std::vector<std::string> desc;

      i->second->getFiguresDesc( desc );

      for(std::vector<std::string>::iterator d = desc.begin(); d != desc.end(); ++d )
      {
        list.push_back(*d);
      }
    }
  }

  void
  StatsStorage::reportData(std::vector<std::string>& list)
  {
    std::map<std::string, IStatsType*>::iterator i;
    for(i = m_stypes.begin(); i != m_stypes.end(); ++i)
    {
      std::vector<double> data;

      i->second->getFiguresData( data );

      for(std::vector<double>::iterator d = data.begin(); d != data.end(); ++d )
      {
        char str[20];
        sprintf(str, "%.2f", *d);
        list.push_back(str);
      }
    }
  }

}
