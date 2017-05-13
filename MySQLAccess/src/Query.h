// $Id: Query.h,v 1.5 2011/03/22 10:29:54 avalassi Exp $
#ifndef MYSQLACCESS_QUERY_H
#define MYSQLACCESS_QUERY_H 1

#ifdef WIN32
#pragma warning ( disable : 4250 ) // dominance in multiple inheritance
#endif

#include "RelationalAccess/IQuery.h"
#include "QueryDefinition.h"

namespace coral
{
  namespace MySQLAccess
  {

    class ISessionProperties;
    class Cursor;

    /**
     * Class Query
     *
     * Implementation of the IQuery interface for the MySQLAccess module
     */

    class Query : virtual public coral::IQuery, public coral::MySQLAccess::QueryDefinition
    {
    public:
      /// Constructor for queries in a single table
      Query( const ISessionProperties& properties, const std::string& tableName );

      /// Constructor for general queries
      explicit Query( const ISessionProperties& properties );

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
