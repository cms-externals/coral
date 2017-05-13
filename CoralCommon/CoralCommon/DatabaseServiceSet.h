#ifndef RELATIONALACCESS_DATABASESERVICESET_H
#define RELATIONALACCESS_DATABASESERVICESET_H

#include "RelationalAccess/IDatabaseServiceSet.h"
#include "RelationalAccess/AccessMode.h"
#include <vector>
#include <string>

namespace coral {

  class DatabaseServiceDescription;

  /**
   * Class DatabaseServiceSet
   *
   * Simple implementation of the IDatabaseServiceSet interface
   */
  class DatabaseServiceSet : virtual public IDatabaseServiceSet
  {
  public:
    /// Constructor
    explicit DatabaseServiceSet( const std::string& context );

    /// Destructor
    virtual ~DatabaseServiceSet();

    /// Copy constructor
    DatabaseServiceSet( const DatabaseServiceSet& rhs );

    /// Assignment operator
    DatabaseServiceSet& operator=( const DatabaseServiceSet& rhs );

  public:
    /// Appends a new replica.
    DatabaseServiceDescription& appendReplica( const std::string& connectionName,
                                               const std::string& authenticationMechanism,
                                               AccessMode accessMode );

  public:
    /**
     * Returns the number of actual database services corresponding to
     * the logical name.
     */
    int numberOfReplicas() const;

    /**
     * Returns a reference to the service description object corresponding
     * to the specified index.
     * If the index is out of range an InvalidReplicaIdentifierException is thrown.
     */
    const IDatabaseServiceDescription& replica( int index ) const;

  private:
    std::string m_context;
    std::vector< DatabaseServiceDescription *> m_data;
  };

}

#endif
