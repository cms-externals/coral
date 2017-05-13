// $Id: QueryDefinition.h,v 1.6.2.1 2010/12/20 09:10:10 avalassi Exp $
#ifndef CORALSERVERBASE_QUERYDEFINITION_H
#define CORALSERVERBASE_QUERYDEFINITION_H 1

// Include files
#include <map>
#include <memory>
#include <string>
#include <vector>
#include "CoralBase/AttributeList.h"
#include "RelationalAccess/IQueryDefinition.h"

namespace coral
{

  // Forward declaration
  class AttributeList;

  /** @class QueryDefinition QueryDefinition.h
   *
   *  Definition of a CORAL relational query.
   *
   *  @author Alexander Kalkhof and Andrea Valassi
   *  @date   2009-01-29
   */

  class QueryDefinition : virtual public IQueryDefinition
  {

  public:

    /// Standard constructor from optional schema and table names.
    explicit QueryDefinition( const std::string& schemaName = "",
                              const std::string& tableName = "" );

    /// Copy constructor.
    QueryDefinition( const QueryDefinition& rhs );

    /// Assignment operator.
    QueryDefinition& operator=( const QueryDefinition& rhs );

    /// Destructor.
    virtual ~QueryDefinition();

    /// Get the schema name ("" means the nominal schema).
    const std::string& getSchemaName() const
    {
      return m_schemaName;
    }

    /// Get the distinct flag.
    bool getDistinct() const
    {
      return m_distinct;
    }

    /// Get the output list.
    const std::vector< std::pair<std::string,std::string> >& getOutputList() const
    {
      return m_outputList;
    }

    /// Get the table list.
    const std::vector< std::pair<std::string,std::string> >& getTableList() const
    {
      return m_tableList;
    }

    /// Get the subqueries.
    const std::map< std::string, QueryDefinition >& getSubQueries() const
    {
      return m_subqueries;
    }

    /// Get the condition.
    const std::string& getCondition() const
    {
      return m_condition;
    }

    /// Get the condition data.
    const AttributeList& getConditionData() const
    {
      return m_conditionData;
    }

    /// Get the GROUP BY clause.
    const std::string& getGroupBy() const
    {
      return m_groupBy;
    }

    /// Get the order list.
    const std::vector< std::string >& getOrderList() const
    {
      return m_orderList;
    }

    /// Get the row limit and offset.
    /// The returned pointer is 0 if limitReturnedRows was never called.
    const std::pair<int,int>* getRowLimitAndOffset() const
    {
      return m_rowLimitAndOffset.get();
    }

    /// Get the set operation.
    /// The returned pointer is 0 if applySetOperation was never called.
    const std::pair<SetOperation,QueryDefinition>* getSetOperation() const
    {
      return m_setOperation.get();
    }

    /// Set the schema name ("" means the nominal schema).
    void setSchemaName( const std::string& schemaName );

    /// Requires a distinct selection.
    void setDistinct();

    /// Appends an expression to the output list.
    /// WARNING: This class performs no checks for invalid/repeated items.
    void addToOutputList( const std::string& expression,
                          std::string alias = "" );

    /// Appends a table name in the table selection list (FROM clause).
    /// WARNING: This class performs no checks for invalid/repeated items.
    void addToTableList( const std::string& tableName,
                         std::string alias = "" );

    /// Defines a subquery (in the FROM clause).
    /// The alias should be used in a subsequent call to addToTableList.
    IQueryDefinition& defineSubQuery( const std::string& alias );

    /// Defines the condition to the query (WHERE clause).
    void setCondition( const std::string& condition,
                       const AttributeList& conditionData );

    /// Defines the GROUP BY clause in the query.
    /// WARNING! The new expression is not appended to the old one.
    void groupBy( const std::string& groupBy );

    /// Appends an expression to the ordering list (ORDER clause).
    void addToOrderList( const std::string& expression );

    /// Instructs the server to send only up to maxRows rows
    /// at the result of the query starting from the offset row.
    void limitReturnedRows( int rowLimit = 1,
                            int rowOffset = 0 );

    /// Applies a set operation.
    /// Returns the rhs query definition so that it can be filled in.
    IQueryDefinition& applySetOperation( SetOperation operationType );

  private:

    std::string m_schemaName;
    bool m_singleTable;
    bool m_distinct;
    std::vector< std::pair<std::string,std::string> > m_outputList;
    std::vector< std::pair<std::string,std::string> > m_tableList;
    std::map< std::string, QueryDefinition > m_subqueries;
    std::string m_condition;
    AttributeList m_conditionData;
    std::string m_groupBy;
    std::vector< std::string > m_orderList;
    std::auto_ptr< std::pair<int,int> > m_rowLimitAndOffset;
    std::auto_ptr< std::pair<SetOperation,QueryDefinition> > m_setOperation;

  };

}
#endif // CORALSERVERBASE_QUERYDEFINITION_H
