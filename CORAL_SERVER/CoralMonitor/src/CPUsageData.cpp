#include "CPUsageData.h"

#include <fcntl.h>
#include <stdlib.h>

#include <vector>
#include <iostream>
#include <fstream>


namespace coral {

  void
  lineTokenizer(const std::string& line, std::vector<std::string>& v)
  {
    std::string h;

    size_t len = line.size();

    for(size_t i = 0; i < len; ++i )
    {
      if(line[i] == ' ')
      {
        if(!h.empty())
        {
          v.push_back(h);
          h.clear();
        }
      }
      else
        h += line[i];
    }

    if(!h.empty())
    {
      v.push_back(h);
      h.clear();
    }
  }

  CPUsageData::CPUsageData()
    : _user( 0 )
    , _system( 0 )
    , _nice( 0 )
    , _idle( 0 )
    , _last_user( 0 )
    , _last_system( 0 )
    , _last_nice( 0 )
    , _last_idle( 0 )
  {
    getDataLinux();
  }

  CPUsageData::~CPUsageData()
  {

  }

  void
  CPUsageData::getDataLinux()
  {
    std::ifstream myfile;

    std::string line;

    myfile.open("/proc/stat", std::ios::in);

    if(myfile.is_open())
    {
      getline(myfile, line);

      myfile.close();

      std::vector<std::string> columns;

      lineTokenizer(line, columns);

      if(columns.size() == 9)
      {
        size_t current_user = atoi(columns[1].c_str());
        _user = current_user - _last_user;
        _last_user = current_user;

        size_t current_nice = atoi(columns[2].c_str());
        _nice = current_nice - _last_nice;
        _last_nice = current_nice;

        size_t current_system = atoi(columns[3].c_str());
        _system = current_system - _last_system;
        _last_system = current_system;

        size_t current_idle = atoi(columns[4].c_str());
        _idle = current_idle - _last_idle;
        _last_idle = current_idle;

        //std::cout << "user: " << _user << " nice: " << _nice << " system: " << _system << " idle: " << _idle;
      }
    }

  }

  void
  CPUsageData::getUsage(CPUsageContainer& data)
  {
    getDataLinux();

    double total = _user + _system + _nice + _idle;

    if(total > 0)
    {
      data.system = _system / total * 100;
      data.user = _user / total * 100;
      data.nice = _nice / total * 100;
    }
    else
    {
      data.system = 0;
      data.user = 0;
      data.nice = 0;
    }
  }

}
