// -*- C++ -*-
// $Id: Statement.h,v 1.8 2011/03/22 10:36:50 avalassi Exp $
#ifndef CORAL_FRONTIERACCESS_STATEMENT_H
#define CORAL_FRONTIERACCESS_STATEMENT_H 1

#include <utility>
#include <vector>
#include <string>
#include <typeinfo>

namespace frontier
{
  class Session;
  class Request;
  class AnyData;
}

namespace coral
{
  class Service;
  class MessageStream;
}

namespace coral
{
  class AttributeList;
  class ITypeConverter;

  namespace FrontierAccess
  {
    class ISessionProperties;

    /**
     * @class FrontierAccess::Statement Statement.h
     *
     * Class responsible for the execution of SQL statements
     * and for the retrieval of its results.
     *
     */
    class Statement
    {
    public:

      struct Field
      {
        std::string name;
        std::string type;
      };

      typedef std::vector< Field > ResultSetMetaData;

    public:
      /**
       * Constructor
       */
      Statement( const coral::FrontierAccess::ISessionProperties& properties, const std::string& sqlStatement );

      /// Destructor
      ~Statement();

      /**
       * Prepares the SQL statement
       * Returns false in case of any error, true otherwise.
       *
       * @param sqlStatement The SQL statement.
       *
       */
      bool prepare( const std::string& sqlStatement );

      /**
       * Binds the input data
       * Returns false in case of any error, true otherwise.
       *
       * @param inputData The input data.
       *
       */
      bool bind( const coral::AttributeList& inputData );

      /**
       * Executes the statement.
       * Returns false in case of any error, true otherwise.
       *
       */
      bool execute( const coral::AttributeList& rowBuffer, bool forceReload );

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
      bool defineOutput( coral::AttributeList& outputData );

      /**
       * Sets the number of prefetched rows
       *
       * @param numberOfRows The number of rows to be prefetched.
       *
       */
      bool setNumberOfPrefetchedRows( unsigned int numberOfRows );

      /**
       * Sets the size of the cache for prefetched rows
       *
       * @param sizeInMB The size of the cache in MBytes.
       *
       */
      bool setCacheSize( unsigned int sizeInMB );

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
      /// Resets the internal structures.
      void reset();
      /// Copies data into the strings
      bool copyData();
      /// Log access
      coral::MessageStream& mslog();

    private:
      // Hidden
      Statement( const Statement& );
      Statement& operator=( const Statement& );

    private:
      /// Session properties
      coral::FrontierAccess::ISessionProperties& m_properties;
      /// The Frontier data source
      frontier::Session*                    m_session;
      /// Frontier request list
      std::vector<const frontier::Request*> m_listOfRequests;
      /// "Binding" parameters' cache
      const coral::AttributeList*            m_boundInputData;
      /// "Binding" result cache
      coral::AttributeList*                  m_boundOutputData;
      /// Currently selected result set row
      int m_currentRow;
      /// NULL indicators
      std::vector<unsigned char>            m_nulls;
      /// Result set metadata
      ResultSetMetaData m_metaData;
      /// The major version number of the database server
      int m_serverVersion;
      /// The original sql statement
      std::string m_preparedSqlStatement;
      /// The executed sql statement
      std::string m_sqlStatement;
      /// The data field
      frontier::AnyData*                    m_field;
      /// The message stream logger
      coral::MessageStream*                  m_mslog;
    };
  }
}

#endif
