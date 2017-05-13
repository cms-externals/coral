#ifndef ORACLEACCESS_ORACLEDOMAIN_PROPERTIES_H
#define ORACLEACCESS_ORACLEDOMAIN_PROPERTIES_H

#include <string>

namespace coral
{
  class Service;
}

namespace coral {

  namespace OracleAccess {

    /**
     * A simple class holding the global domain properties
     */

    class DomainProperties {
    public:
      /// Constructor
      explicit DomainProperties( coral::Service* service );

      /// Destructor
      ~DomainProperties();

      /// Sets the table space for the tables
      void setTableSpaceForTables( const std::string& tableSpace );

      /// Sets the table space for the indices
      void setTableSpaceForIndices( const std::string& tableSpace );

      /// Sets the table space for the lobs
      void setTableSpaceForLobs( const std::string& tableSpace );

      /// Sets the lob chunk size
      void setLobChunkSize( int lobChunkSize );

      /// Returns the pointer to the service
      coral::Service* service() const;

      /// Returns the table space for the tables
      std::string tableSpaceForTables() const;

      /// Returns the table space for the indices
      std::string tableSpaceForIndices() const;

      /// Returns the table space for the lobs
      std::string tableSpaceForLobs() const;

      /// Returns the lob chunk size
      int lobChunkSize() const;

    private:
      /// The pointer to the service
      coral::Service*       m_service;

      /// The default table space for tables
      std::string m_tableSpaceForTables;

      /// The default table space for indices
      std::string m_tableSpaceForIndices;

      /// The default table space for lobs
      std::string m_tableSpaceForLobs;

      /// The chunk size in bytes for a LOB retrieval
      int m_lobChunkSize;
    };

  }

}

#endif
