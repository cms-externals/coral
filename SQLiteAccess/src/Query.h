#ifndef SQLITEACCESS_QUERY_H
#define SQLITEACCESS_QUERY_H

#ifdef WIN32
//IQuery and QueryDefinition are inherits of IQueryDefinition
//QueryDefinition dominates the others but setting them to pure virtual
#pragma warning ( disable : 4250 )
#endif

#include "RelationalAccess/IQuery.h"
#include "QueryDefinition.h"

namespace coral {

  namespace SQLiteAccess {

    class SessionProperties;
    class Cursor;

    /**
     * Class Query
     *
     * Implementation of the IQuery interface for the SQLiteAccess module
     */

    class Query : virtual public coral::IQuery,
                  public coral::SQLiteAccess::QueryDefinition
    {
    public:
      /// Constructor for queries in a single table
      Query( boost::shared_ptr<const SessionProperties> properties,
             const std::string& tableName );

      /// Constructor for general queries
      explicit Query( boost::shared_ptr<const SessionProperties> properties );

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
      /// The session property
      boost::shared_ptr<const SessionProperties> m_properties;

      /// The cursor object
      Cursor*                              m_cursor;

      /// Flag indicating whether the result set should be locked
      bool m_forUpdate;

      /// The row cache size. A positive value is the size in rows. A negative value is the size in MB
      int m_cacheSize;

      /// The output buffer
      coral::AttributeList*                m_outputBuffer;

      /// The externally defined output types
      std::map< std::string, std::string > m_outputTypes;
    };
  }

}

#endif
