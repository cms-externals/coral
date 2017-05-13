// $Id: QueryDefinition.h,v 1.6 2011/03/22 10:29:54 avalassi Exp $
#ifndef MYSQLACCESS_QUERYDEFINITION_H
#define MYSQLACCESS_QUERYDEFINITION_H 1

#include <map>
#include <vector>
#include "RelationalAccess/IQueryDefinition.h"

namespace coral
{

  class ITableDescription;
  class ISchema;

  namespace MySQLAccess
  {

    class ISessionProperties;

    /**
     * Class implementing the IQueryDefinition interface
     */
    class QueryDefinition : virtual public coral::IQueryDefinition
    {
    public:
      /// Constructor for many tables
      explicit QueryDefinition( const coral::MySQLAccess::ISessionProperties& properties );

      /// Constructor for single table
      QueryDefinition( const coral::MySQLAccess::ISessionProperties& properties,
                       const std::string& tableName );

      /// Destructor
      virtual ~QueryDefinition();

      /// Returns the sql fragment corresponding to the definition
      std::string sqlFragment() const;

      /// Returns the bind data relevant for this fragment
      const coral::AttributeList& bindData() const;

      /// Returns the output variables
      const std::vector<std::string> outputVariables() const;

      /// Returns a reference to the session properties
      const ISessionProperties& sessionProperties() const;

      // Methods inherited from the base interface

      /**
       * Requires a distinct selection.
       */
      void setDistinct();

      /**
       * Appends an expression to the output list
       */
      void addToOutputList( const std::string& expression,
                            std::string alias = "" );

      /**
       * Appends a table name in the table selection list (the FROM part of the query)
       */
      void addToTableList( const std::string& tableName,
                           std::string alias = "" );

      /**
       * Defines a subquery.
       * The specified name should be used in a subsequent call to addToTableList.
       */
      coral::IQueryDefinition& defineSubQuery( const std::string& allias );

      /**
       * Defines the condition to the query (the WHERE clause)
       */
      void setCondition( const std::string& condition,
                         const coral::AttributeList& inputData );

      /**
       * Appends a GROUP BY clause in the query
       */
      void groupBy( const std::string& expression );

      /**
       * Appends an expression to the ordering list (the ORDER clause)
       */
      void addToOrderList( const std::string& expression );

      /**
       * Instructs the server to send only up to maxRows rows at the result
       * of the query starting from the offset row.
       */
      void limitReturnedRows( int maxRows = 1,
                              int offset = 0 );

      /**
       * Applies a set operation. Returns the rhs query definition so that it can be filled in.
       */
      coral::IQueryDefinition& applySetOperation( coral::IQueryDefinition::SetOperation opetationType );

    private:
      /// Calculates the sql fragment from the underlying data
      void process();

    protected:
      /// A reference to the session data
      const ISessionProperties&                            m_properties;

    private:
      /// A flag indicating whether a single table is involved in the query
      bool m_singleTable;

      /// The sql fragment corresponding to the query definition
      std::string m_sqlFragmnent;

      /// The assosiated input data
      coral::AttributeList*                                m_inputData;

      /// The distinct flag
      bool m_distinct;

      /// The output list
      std::vector< std::pair< std::string, std::string > > m_outputList;

      /// The table list
      std::vector< std::pair< std::string, std::string > > m_tableList;

      /// Sub-query definitions
      std::map< std::string, QueryDefinition >             m_subQueries;

      /// Condition string
      std::string m_condition;

      /// The GROUP BY clause
      std::string m_grouping;

      /// The order list
      std::vector< std::string >                           m_orderList;

      /// The limit to the number of returned rows
      int m_rowLimit;

      /// The offset to the number of returned rows
      int m_rowOffset;

      /// Rhs query in a set operation
      std::pair< QueryDefinition*, coral::IQueryDefinition::SetOperation > m_setQuery;

      /// The final output list
      std::vector< std::string >                           m_output;
    };

  }

}

#endif
