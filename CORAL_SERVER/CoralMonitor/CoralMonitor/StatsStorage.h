#ifndef CORALSERVERBASE_STATSSTORAGE_H
#define CORALSERVERBASE_STATSSTORAGE_H

#include "CoralMonitor/IStatsType.h"

//Standard C++ Includes
#include <map>
#include <string>
#include <vector>

namespace coral
{

  /** @class StatsStorage
   *
   *  Instance for a Statistic Collection
   *
   *  As Collection IStatsType Objects are used
   *  Stored in StatsStorage and destroyed
   *
   *  Reports a current Statistic Summary
   *
   *  @author Alexander Kalkhof
   *  @date   2009-08-21
   */

  class StatsStorage {
  public:
    //constructor
    //refresh rate handles if refresh() has an effect
    StatsStorage();
    //destructor
    ~StatsStorage();
    //add a new IStatsType to the storage
    void add(IStatsType*);
    //fills up the vector with the reports of each stats type
    void report(std::vector<std::string>&);

    void reportDesc(std::vector<std::string>&);

    void reportData(std::vector<std::string>&);

  private:
    //vector to store the stats types
    std::map<std::string, IStatsType*> m_stypes;

  };
  //predefinition
  //create a global instance of the StatsStorage object
  StatsStorage& StatsStorageInstance();

}

#endif
