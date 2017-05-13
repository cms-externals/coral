#ifndef SQLITEACCESS_SQLITESTATEMENT_H
#define SQLITEACCESS_SQLITESTATEMENT_H 1

#include <string>
#include <vector>
#include <boost/any.hpp>
#include <boost/shared_ptr.hpp>

//class sqlite3;
//class sqlite3_stmt;
struct sqlite3_stmt;

namespace coral
{
  class AttributeList;
  //class ITypeConverter;

  namespace SQLiteAccess
  {
    class SessionProperties;
    class StatementStatistics;

    /**@class SQLiteStatement SQLiteStatement.h
       @author Zhen Xie
    */
    class SQLiteStatement
    {
    public:

      /**Constructor
       *
       * @param  [IN] properties
       * @exception
       */
      explicit SQLiteStatement( boost::shared_ptr<const SessionProperties> properties );

      ///Destructor
      ~SQLiteStatement();

      //return the shared pointer for SessionProperties
      boost::shared_ptr<const SessionProperties> sessionProperties() const;

      /**prepare the statement
       *
       * @param  [IN] sql statement string
       * @exception
       */
      bool prepare( const std::string& statement );

      /**Executes the statements which no results are expected
       * Returns false in case of any error, true otherwise.
       *
       * @param  [IN]
       * @param  [OUT]
       * @exception
       */
      bool execute();

      /**
       * Returns the C++ type for an output column [0,..]
       *
       * @param columnId The column identifier
       *
       * @param typeConverter The SQL to C++ type converter
       *
       */
      //const std::type_info* typeForOutputColumn( int columnId ) const;

      /**
       * Returns the C++ type for an output column [0,..]
       *
       * @param columnId The column identifier
       *
       * @param typeConverter The SQL to C++ type converter
       *
       */
      const std::type_info* decltypeForColumn( int columnId ) const;

      /**
       * Returns the number of rows processed.
       *
       */
      unsigned int numberOfRowsProcessed() const;

      /**
       * Returns the C++ type for an output column [1,...]
       *
       * @param columnId The column identifier, starting from 1
       *
       * @param typeConverter The SQL to C++ type converter
       *
       */
      //std::string typeForOutputColumn( int columnId, const coral::ITypeConverter& typeConverter ) const;

      /**bind the statement with the defined input data
       *
       * @param  [IN]
       * @exception
       */
      bool bind( const coral::AttributeList& inputData );

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

      /**bind the statement with the defined input data
       *
       * @param  [IN]
       * @exception
       */
      //bool bind( const std::vector<boost::any>& inputData );

      /**
       * Retrieves the current row number
       *
       */
      unsigned int currentRowNumber() const;

      /**
       * Fetches the next row. Returns false in case no more rows are there.
       *
       */
      bool fetchNext();

      /**
       * Defines the output. It assumes that the privided output AttributeList matches exactly
       * the actual output from the statement.
       * Returns false in case of any error, true otherwise.
       *
       * @param [OUT] outputData The buffers where the output data should be delivered.
       * @exception
       */
      bool defineOutput( coral::AttributeList& outputData );

      /**reset the prepared statement
       *
       * @exception
       */
      bool reset();

      //bool isVariableNull( int variableIndex ) const;

    private:

      /**execute the statement, returns SQLITE status
       * @exception
       */
      int step();

      /**finalize the prepared statement
       *
       * @exception
       */
      void finalize();

    private:

      /// The session property
      boost::shared_ptr<const SessionProperties> m_properties;

      /// The sqlite3 statement handle
      sqlite3_stmt* m_stmt;

      /// The sql statement
      //std::string        m_sqlstmt;

      /// The null columns
      //std::vector<int> m_nullcols;

      /// The row counter
      int m_rowcounter;

      /// The statistics for the statement
      StatementStatistics* m_statementStatistics;

      /// A rollback statement
      bool m_rollBack;
    };

  }

}
#endif /* SQLITEACCESS_SQLITESTATEMENT_H */
