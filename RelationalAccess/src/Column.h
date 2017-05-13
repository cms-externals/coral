#ifndef COLUMN_H
#define COLUMN_H

#include "RelationalAccess/IColumn.h"

namespace coral {

  /** Class Column
   *
   * Transient implementation of the column interface
   */

  class Column : virtual public IColumn
  {
  public:
    Column( std::string name = "",
            std::string type = "",
            long size = 0,
            bool sizeFixed = true,
            std::string tableSpaceName = "" ) :
      m_name( name ),
      m_type( type ),
      m_index(0),
      m_notNull( false ),
      m_unique( false ),
      m_size( size ),
      m_sizeFixed( sizeFixed ),
      m_tableSpaceName( tableSpaceName )
    {}

    virtual ~Column() {}

    std::string name() const { return m_name; }
    void setName( const std::string& name ) { m_name = name; }

    std::string type() const { return m_type; }
    void setType( const std::string& type ) { m_type = type; }

    int indexInTable() const {return m_index; }
    void setIndexInTable( int index ) { m_index = index; }

    bool isNotNull() const {return m_notNull; }
    void setNotNull( bool notNull ) { m_notNull = notNull; }

    bool isUnique() const { return m_unique; }
    void setUnique( bool unique ) { m_unique = unique; }

    long size() const { return m_size; }
    void setSize( long size ) { m_size = size; }

    bool isSizeFixed() const { return m_sizeFixed; }
    void setSizeFixed( bool setSizeFixed ) { m_sizeFixed = setSizeFixed; }

    std::string tableSpaceName() const { return m_tableSpaceName; }
    void setTableSpaceName( const std::string& tableSpaceName ) { m_tableSpaceName = tableSpaceName; }

  private:
    std::string m_name;
    std::string m_type;
    int m_index;
    bool m_notNull;
    bool m_unique;
    long m_size;
    bool m_sizeFixed;
    std::string m_tableSpaceName;
  };

}

#endif
