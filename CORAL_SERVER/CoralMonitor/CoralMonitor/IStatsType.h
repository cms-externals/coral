#ifndef CORALSERVERBASE_ICSSTATSTYPE_H
#define CORALSERVERBASE_ICSSTATSTYPE_H

#include <string>
#include <vector>

namespace coral
{

  /** @class IStatsType
   *
   *  Interface for a generic Statistic Type
   *  for the StatsStorage
   *
   *
   *  @author Alexander Kalkhof
   *  @date   2009-08-21
   */

  class IStatsType {
  private:
    //default constructor kept private
    IStatsType();

  protected:
    //special constructor to setup the type name
    //type name must be unique for all fellow classes of this abstract class
    //can only be called by heritage classes
    explicit IStatsType(const std::string& tname) { m_tname = tname; };

  public:
    //destructor
    virtual ~IStatsType() {};
    //get back the type name
    const std::string& name() { return m_tname; };
    //the abstract method to get the statistic data
    virtual void getFiguresDesc(std::vector<std::string>&) = 0;

    virtual void getFiguresData(std::vector<double>&) = 0;

  private:
    //internal data to store the name
    //can only be changed by construction
    std::string m_tname;

  };

}

#endif
