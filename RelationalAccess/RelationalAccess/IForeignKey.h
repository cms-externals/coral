#ifndef RELATIONALACCESS_IFOREIGNKEY_H
#define RELATIONALACCESS_IFOREIGNKEY_H

#include <string>
#include <vector>
#include "CoralBase/VersionInfo.h" // Relevant for #ifdef CORAL240xx

namespace coral {
  /**
   * Class IForeignKey
   * Interface for the description of a foreign key constraint defined on a table.
   */
  class IForeignKey {
  public:
    /**
     * Returns the system name of the foreign key constraint.
     */
    virtual std::string name() const = 0;
  
    /**
     * Returns the names of the columns defining the constraint.
     */
    virtual const std::vector<std::string>& columnNames() const = 0;
    
    /**
     * Returns the name of the referenced table.
     */
    virtual std::string referencedTableName() const = 0;
    
    /**
     * Returns the names of the columns in the referenced table.
     */
    virtual const std::vector<std::string>& referencedColumnNames() const = 0;

#ifdef CORAL240DC
    /**
     * Returns the onDeleteCascade flag.
     */
    virtual bool onDeleteCascade() const = 0;
#endif

  protected:
    /// Protected empty destructor
    virtual ~IForeignKey() {}
  };

}
#endif
