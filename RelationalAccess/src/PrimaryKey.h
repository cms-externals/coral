#ifndef PRIMARY_KEY_H
#define PRIMARY_KEY_H

#include "RelationalAccess/IPrimaryKey.h"

namespace coral {

  /**
   * Class PrimaryKey
   *
   * Transient implementation of the IPrimaryKey interface
   */
  class PrimaryKey : virtual public IPrimaryKey
  {
  public:
    PrimaryKey( const std::vector< std::string >& columnNames,
                std::string tableSpaceName = "" ) :
      m_columnNames( columnNames ),
      m_tableSpaceName( tableSpaceName )
    {}
    virtual ~PrimaryKey() {}
    const std::vector<std::string>& columnNames() const { return m_columnNames; }
    std::vector<std::string>& columnNames() { return m_columnNames; }
    std::string tableSpaceName() const { return m_tableSpaceName; }
  private:
    std::vector<std::string> m_columnNames;
    std::string m_tableSpaceName;
  };

}

#endif
