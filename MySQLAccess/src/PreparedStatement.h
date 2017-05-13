// $Id: PreparedStatement.h,v 1.4 2011/03/22 10:29:54 avalassi Exp $
#ifndef CORAL_MYSQLACCESS_PREPAREDSTATEMENT_H
#define CORAL_MYSQLACCESS_PREPAREDSTATEMENT_H

#include "MySQL_headers.h"

#include <string>
#include <utility>
#include <vector>
#include "CoralBase/AttributeList.h"

#include "NamedInputParametersParser.h"

namespace seal
{
  class Service;
}

namespace coral
{
  class AttributeList;
  class ITypeConverter;

  namespace MySQLAccess
  {

    class ISessionProperties;

    /**
     * @class MySQLAccess::PreparedStatement PreparedStatement.h
     *
     * Class responsible for the execution of SQL statements and for the retrieval of its results using the new binary MySQL protocol as of MySQL version 4.1.x
     *
     */

    class PreparedStatement
    {
    public:

      struct Field
      {
        std::string name;
        const std::type_info* type;
      };

      typedef std::vector< Field > ResultSetMetaData;

      /**
       * Constructor
       *
       * @param environmentHandle Pointer to the master environment handle
       *
       * @param serviceContexHandle Pointer to the service context handle
       *
       * @param errorHandle Pointer to the error handle
       *
       * @param serverVersion The major version number of the database server
       */
      PreparedStatement( const coral::MySQLAccess::ISessionProperties&, const std::string&  );

      /// Destructor
      virtual ~PreparedStatement();

      /**
       * Prepares the SQL statement
       * Returns false in case of any error, true otherwise.
       *
       * @param sqlStatement The SQL statement.
       *
       */
      void prepare( const std::string& sqlStatement );

      /**
       * Binds the input data
       * Returns false in case of any error, true otherwise.
       *
       * @param inputData The input data.
       *
       */
      void bind( const coral::AttributeList& inputData );

      /**
       * Executes the statement.
       * Returns false in case of any error, true otherwise.
       *
       * @param scrollable Boolean flag which, if set, produces scrollable result set.
       *
       */
      bool execute( const coral::AttributeList& );

      /**
       * Returns the number of rows processed.
       *
       */
      unsigned int numberOfRowsProcessed() const;

      /**
       * Returns the C++ type for an output column [1,...]
       *
       * @param columnId The column identifier
       *
       * @param typeConverter The SQL to C++ type converter
       *
       */
      const std::type_info* typeForOutputColumn( int columnId ) const;

      /**
       * Defines the output. It assumes that the privided output AttributeList matches exactly
       * the actual output from the statement.
       * Returns false in case of any error, true otherwise.
       *
       * @param outputData The buffers where the output data should be delivered.
       *
       */
      void defineOutput( coral::AttributeList& outputData );

      /**
       * Sets the number of prefetched rows
       *
       * @param numberOfRows The number of rows to be prefetched.
       *
       */
      void setNumberOfPrefetchedRows( unsigned int numberOfRows );

      /**
       * Sets the size of the cache for prefetched rows
       *
       * @param sizeInMB The size of the cache in MBytes.
       *
       */
      void setCacheSize( unsigned int sizeInMB );

      /**
       * Fetches the next row. Returns false in case no more rows are there.
       *
       */
      bool fetchNext();

      /**
       * Retrieves the current row number
       *
       */
      unsigned int currentRowNumber() const;

    private:
      /// Extract result set metadata
      void getResultSetInfo();
      /// Resets the internal structures.
      void reset();
      /// Copies data into the strings
      void copyData( MYSQL_ROW& );

    private:
      /// The seal service
      const coral::MySQLAccess::ISessionProperties& m_sessionProperties;
      /// The Frontier data source
      MYSQL*                                        m_connection;
      /// Bind variables meta data
      BVars m_bvars;
      /// "Binding" parameters' cache
      const coral::AttributeList*                   m_boundInputData;
      /// "Binding" result cache
      coral::AttributeList*                         m_boundOutputData;
      /// Currently selected result set row
      int m_currentRow;
      /// Result set metadata
      ResultSetMetaData m_metaData;
      /// The major version number of the database server
      std::string m_serverVersion;
#if (  MYSQL_VERSION_ID > 40100 )
      /// The executed sql statement
      MYSQL_STMT*                                   m_preparedSqlStatement;
      /// The binding structures
      MYSQL_BIND*                                   m_binders; // FIXME - Make it std::vector
      /// The length idicators for bound data
      unsigned long*                                m_bindlens; // FIXME - Make it std::vector
      /// The nullnes indicators for bound data
      my_bool*                                      m_bindnulls; // FIXME - Make it std::vector
#endif
      /// The original sql statement
      std::string m_sqlStatement;
      /// The sql statement with '?' markers used for prepared statements
      std::string m_markedSqlStatement;
      /// The result set
      MYSQL_RES*                                    m_result;
      /// The flag saying whether this was a SELECT ... or not
      bool m_isSelect;
    };
  }
}

#endif // CORAL_MYSQLACCESS_PREPAREDSTATEMENT_H
