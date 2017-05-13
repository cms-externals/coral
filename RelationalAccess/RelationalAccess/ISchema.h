#ifndef RELATIONALACCESS_ISCHEMA_H
#define RELATIONALACCESS_ISCHEMA_H

#include <string>
#include <set>
#include "CoralBase/VersionInfo.h" // Relevant for #ifdef CORAL240xx

namespace coral 
{

  // Forward declarations
  class AttributeList;
#ifdef CORAL240CN
  class IChangeNotification;
#endif
  class IQuery;
#ifdef CORAL240SQ
  class ISequence;
  class ISequenceDescription;
#endif
  class ITable;
  class ITableDescription;
  class IView;
  class IViewFactory;

  /**
   * Class ISchema
   * Abstract interface to manage a schema in a relational database.
   * Any operation requires that a transaction has been started,
   * otherwise a TransactionNotActiveException is thrown.
   */
  class ISchema 
  {

  public:

    /**
     * Returns the name of this schema.
     */
    virtual std::string schemaName() const = 0;

    /**
     * Returns the names of all tables in the schema.
     */
    virtual std::set<std::string> listTables() const = 0;
  
    /**
     * Checks the existence of a table with the specified name.
     */
    virtual bool existsTable( const std::string& tableName ) const = 0;
  
    /**
     * Drops the table with the specified name.
     * If the table does not exist a TableNotExistingException is thrown.
     */
    virtual void dropTable( const std::string& tableName ) = 0;
    
    /**
     * Drops the table with the specified name in case it exists.
     */
    virtual void dropIfExistsTable( const std::string& tableName ) = 0;
    
    /**
     * Creates a new table with the specified description and returns the corresponding table handle.
     * If a table with the same name already exists TableAlreadyExistingException is thrown.
     */
    virtual ITable& createTable( const ITableDescription& description ) = 0;
  
    /**
     * Returns a reference to an ITable object corresponding to the table with the specified name.
     * In case no table with such a name exists, a TableNotExistingException is thrown.
     */
    virtual ITable& tableHandle( const std::string& tableName ) = 0;

    /**
     * Truncates the data of the the table with the specified name.
     * In case no table with such a name exists, a TableNotExistingException is thrown.
     */
    virtual void truncateTable( const std::string& tableName ) = 0;

    /**
     * Calls a stored procedure with input parameters.
     * In case of an error a SchemaException is thrown.
     */
    virtual void callProcedure( const std::string& procedureName,
                                const coral::AttributeList& inputArguments ) = 0;

    /**
     * Returns a new query object.
     */
    virtual IQuery* newQuery() const = 0;
  
    /**
     * Returns a new view factory object in order to define and create a view.
     */
    virtual IViewFactory* viewFactory() = 0;
    
    /**
     * Checks the existence of a view with the specified name.
     */
    virtual bool existsView( const std::string& viewName ) const = 0;
  
    /**
     * Drops the view with the specified name.
     * If the view does not exist a ViewNotExistingException is thrown.
     */
    virtual void dropView( const std::string& viewName ) = 0;
  
    /**
     * Drops the view with the specified name in case it exists
     */
    virtual void dropIfExistsView( const std::string& viewName ) = 0;
  
    /**
     * Returns the names of all views in the schema.
     */
    virtual std::set<std::string> listViews() const = 0;
  
    /**
     * Returns a reference to an IView object corresponding to the view with the specified name.
     * In case no view with such a name exists, a ViewNotExistingException is thrown.
     */
    virtual IView& viewHandle( const std::string& viewName ) = 0;

#ifdef CORAL240CN
    /**
     * Register a change notification callback for a table in this schema.
     * WARNING: this API extension is disabled by default (use -DCORAL240CN).
     */
    virtual void registerNotification( const std::string& tableName, 
                                       IChangeNotification& callback ) = 0;
#endif

#ifdef CORAL240SQ

    /**
     * Creates a new sequence with the specified description and returns the corresponding sequence handle.
     * If a sequence with the same name already exists an exception is thrown.
     */
    virtual ISequence& createSequence( const ISequenceDescription& ) = 0;

    /**
     * Returns the names of all sequences in the schema.
     */
    //virtual std::set<std::string> listSequences() const = 0;
  
    /**
     * Checks the existence of a sequence with the specified name.
     */
    virtual bool existsSequence( const std::string& sequenceName ) const = 0;

    /**
     * Returns a reference to an ISequence object corresponding to the sequence with the specified name.
     * In case no sequence with such a name exists, an exception is thrown.
     */
    virtual ISequence& sequenceHandle( const std::string& sequenceName ) = 0;

    /**
     * Drops the sequence with the specified name.
     * If the sequence does not exist an exception is thrown.
     */
    virtual void dropSequence( const std::string& sequenceName ) = 0;

    /**
     * Drops the table with the specified name in case it exists.
     */
    virtual void dropIfExistsSequence( const std::string& sequenceName ) = 0;

    /**
     * Returns the names of all sequences.
     */
    virtual std::set<std::string> listSequences() const = 0;

#endif

  protected:
    /// Protected empty destructor
    virtual ~ISchema() {}
  };
}

#endif


