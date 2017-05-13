#ifndef RELATIONALACCESS_FOREIGNKEY_H
#define RELATIONALACCESS_FOREIGNKEY_H

#include "CoralBase/VersionInfo.h" // Relevant for #ifdef CORAL240xx
#include "RelationalAccess/IForeignKey.h"

namespace coral {

  /**
   * Class ForeignKey
   *
   * Transient implementation of the IForeignKey interface
   */
  class ForeignKey : virtual public IForeignKey
  {
  public:

#ifdef CORAL240DC
    ForeignKey( const std::string name,
                const std::vector<std::string>& columnNames,
                const std::string& referencedTableName,
                const std::vector<std::string>& referencedColumnNames,
                bool onDeleteCascade )
      : m_name( name )
      , m_columnNames( columnNames )
      , m_referencedTableName( referencedTableName )
      , m_referencedColumnNames( referencedColumnNames )
      , m_onDeleteCascade( onDeleteCascade )
    {}
#else
    ForeignKey( const std::string name,
                const std::vector<std::string>& columnNames,
                const std::string& referencedTableName,
                const std::vector<std::string>& referencedColumnNames ) :
      m_name( name ),
      m_columnNames( columnNames ),
      m_referencedTableName( referencedTableName ),
      m_referencedColumnNames( referencedColumnNames )
    {}
#endif

    virtual ~ForeignKey() {}

    std::string name() const { return m_name; }

    const std::vector<std::string>& columnNames() const { return m_columnNames; }

    std::vector<std::string>& columnNames() { return m_columnNames; }

    std::string referencedTableName() const { return m_referencedTableName; }

    const std::vector<std::string>& referencedColumnNames() const { return m_referencedColumnNames; }
#ifdef CORAL240DC
    bool onDeleteCascade() const { return m_onDeleteCascade; }
#endif

  private:

    std::string m_name;

    std::vector<std::string> m_columnNames;

    std::string m_referencedTableName;

    std::vector<std::string> m_referencedColumnNames;
#ifdef CORAL240DC
    bool m_onDeleteCascade;
#endif

  };

}

#endif
