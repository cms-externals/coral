#ifndef UNIQUECONSTRAINT_H
#define UNIQUECONSTRAINT_H

#include "RelationalAccess/IUniqueConstraint.h"

namespace coral {

  /**
   * Class UniqueConstraint
   *
   * Transient implementation of the IUniqueConstraint interface
   */
  class UniqueConstraint : virtual public IUniqueConstraint
  {
  public:
    UniqueConstraint( const std::string& name,
                      const std::vector<std::string>& columnNames,
                      const std::string& tableSpaceName ) :
      m_name( name ),
      m_columnNames( columnNames ),
      m_tableSpaceName( tableSpaceName )
    {}
    virtual ~UniqueConstraint() {}
    std::string name() const { return m_name; }
    const std::vector<std::string>& columnNames() const { return m_columnNames; }
    std::vector<std::string>& columnNames() { return m_columnNames; }
    std::string tableSpaceName() const { return m_tableSpaceName; }

  private:
    std::string m_name;
    std::vector<std::string> m_columnNames;
    std::string m_tableSpaceName;
  };
}

#endif
