#ifndef INDEX_H
#define INDEX_H

#include "RelationalAccess/IIndex.h"

namespace coral {

  /**
   * Class Index
   *
   * Transient implementation of the IIndex interface
   */
  class Index : virtual public IIndex
  {
  public:
    Index( const std::string& name,
           const std::vector<std::string>& columnNames,
           bool unique,
           const std::string& tableSpaceName ) :
      m_name( name ),
      m_columnNames( columnNames ),
      m_unique( unique ),
      m_tableSpaceName( tableSpaceName )
    {}
    virtual ~Index() {}
    std::string name() const { return m_name; }
    const std::vector<std::string>& columnNames() const { return m_columnNames; }
    std::vector<std::string>& columnNames() { return m_columnNames; }
    bool isUnique() const { return m_unique; }
    std::string tableSpaceName() const { return m_tableSpaceName; }
  private:
    std::string m_name;
    std::vector<std::string> m_columnNames;
    bool m_unique;
    std::string m_tableSpaceName;

  };

}

#endif
