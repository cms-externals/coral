#ifndef ORACLEACCESS_QUERY_H
#define ORACLEACCESS_QUERY_H 1

#ifdef WIN32
#pragma warning ( disable : 4250 )
#endif

#include <boost/shared_ptr.hpp>
#include "RelationalAccess/IQuery.h"
#include "QueryDefinition.h"

namespace coral
{

  namespace OracleAccess
  {

    class Cursor;
    class SessionProperties;

    /**
     * Class Query
     *
     * Implementation of the IQuery interface for the OracleAccess module
     */

    class Query : public coral::OracleAccess::QueryDefinition,
                  virtual public coral::IQuery
    {
    public:
      /// Constructor for queries on a single table in a schema
      Query( boost::shared_ptr<const SessionProperties> properties,
             const std::string& schemaName,
             const std::string& tableName );

      /// Constructor for general queries in a given schema
      explicit Query( boost::shared_ptr<const SessionProperties> properties,
                      const std::string& schemaName );

      /// Destructor
      virtual ~Query();

      /**
       * Instructs the server to lock the rows involved in the result set.
       */
      void setForUpdate();

      /**
       * Defines the client cache size in rows
       */
      void setRowCacheSize( int numberOfCachedRows );

      /**
       * Defines the client cache size in MB.
       */
      void setMemoryCacheSize( int sizeInMB );

      /**
       * Defines the output types of the result set.
       */
      void defineOutputType( const std::string& outputIdentifier,
                             const std::string& cppTypeName );

      /**
       * Defines the output data buffer for the result set.
       */
      void defineOutput( coral::AttributeList& outputDataBuffer );

      /**
       * Executes the query and returns a reference to the undelying ICursor object
       * in order for the user to loop over the result set.
       */
      coral::ICursor& execute();

    private:
      /// The cursor object
      Cursor* m_cursor;

      /// Flag indicating whether the result set should be locked
      bool m_forUpdate;

      /// The memory cache size in MB
      size_t m_memoryCache;

      /// The row cache size
      size_t m_rowCache;

      /// The output buffer
      coral::AttributeList* m_outputBuffer;

      /// The externally defined output types
      std::map< std::string, std::string > m_outputTypes;
    };
  }

}

#endif
