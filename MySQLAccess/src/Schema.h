// $Id: Schema.h,v 1.25 2011/03/22 10:29:54 avalassi Exp $
#ifndef MYSQLACCESS_SCHEMA_H
#define MYSQLACCESS_SCHEMA_H 1

#include "CoralBase/VersionInfo.h" // Relevant for #ifdef CORAL240xx
#include "CoralBase/boost_thread_headers.h"
#include "RelationalAccess/ISchema.h"
#include "ITransactionObserver.h"

namespace coral
{
  namespace MySQLAccess
  {

    class ISessionProperties;
    class ISchemaProperties;

    /**
     * Class Schema
     *
     * Implementation of the ISchema interface for the MySQLAccess package
     */
    class Schema : virtual public coral::ISchema,
                   virtual public ITransactionObserver
    {
    public:

      /// Constructor
      explicit Schema( const ISessionProperties& sessionProperties,
                       const std::string& );

      /// Destructor
      virtual ~Schema();

      /**
       * Returns the name of this schema.
       */
      std::string schemaName() const;

      /**
       * Returns the names of all tables in the schema.
       */
      std::set<std::string> listTables() const;

      /**
       * Flag to invalidate cached schema metadata
       */
      void setTableListStale();

      /**
       * Checks the existence of a table with the specified name.
       */
      bool existsTable( const std::string& tableName ) const;

      /**
       * Drops the table with the specified name.
       * If the table does not exist a TableNotExistingException is thrown.
       */
      void dropTable( const std::string& tableName );

      /**
       * Drops the table with the specified name in case it exists.
       */
      void dropIfExistsTable( const std::string& tableName );

      /**
       * Creates a new table with the specified description and returns the corresponding table handle.
       * If a table with the same name already exists TableAlreadyExistingException is thrown.
       */
      coral::ITable& createTable( const coral::ITableDescription& description );

      /**
       * Returns a reference to an ITable object corresponding to the table with the specified name.
       * In case no table with such a name exists, a TableNotExistingException is thrown.
       */
      coral::ITable& tableHandle( const std::string& tableName );

      /**
       * Truncates the data of the the table with the specified name.
       * In case no table with such a name exists, a TableNotExistingException is thrown.
       */
      void truncateTable( const std::string& tableName );

      /**
       * Calls a stored procedure with input parameters.
       * In case of an error a SchemaException is thrown.
       */
      void callProcedure( const std::string& procedureName,
                          const coral::AttributeList& inputArguments );

      /**
       * Returns a new query object.
       */
      coral::IQuery* newQuery() const;

      /**
       * Returns a new view factory object in order to define and create a view.
       */
      coral::IViewFactory* viewFactory();

      /**
       * Checks the existence of a view with the specified name.
       */
      bool existsView( const std::string& viewName ) const;

      /**
       * Drops the view with the specified name.
       * If the view does not exist a ViewNotExistingException is thrown.
       */
      void dropView( const std::string& viewName );

      /**
       * Drops the view with the specified name in case it exists
       */
      void dropIfExistsView( const std::string& viewName );

      /**
       * Returns the names of all views in the schema.
       */
      std::set<std::string> listViews() const;

      /**
       * Returns a reference to an IView object corresponding to the view with the specified name.
       * In case no view with such a name exists, a ViewNotExistingException is thrown.
       */
      coral::IView& viewHandle( const std::string& viewName );

#ifdef CORAL240CN
      /**
       * Register a change notification callback for a table in this schema.
       * WARNING: this API extension is disabled by default (use -DCORAL240CN).
       */
      void registerNotification( const std::string& /*tableName*/,
                                 IChangeNotification& /*callback*/ ) {}
#endif

#ifdef CORAL240SQ

      /**
       * Creates a new sequence with the specified description and returns the corresponding sequence handle.
       * If a sequence with the same name already exists an exception is thrown.
       */
      ISequence& createSequence( const ISequenceDescription& );

      /**
       * Checks the existence of a sequence with the specified name.
       */
      bool existsSequence( const std::string& sequenceName ) const;

      /**
       * Returns a reference to an ISequence object corresponding to the sequence with the specified name.
       * In case no sequence with such a name exists, an exception is thrown.
       */
      ISequence& sequenceHandle( const std::string& sequenceName );

      /**
       * Drops the sequence with the specified name.
       * If the sequence does not exist an exception is thrown.
       */
      void dropSequence( const std::string& sequenceName );

      /**
       * Drops the table with the specified name in case it exists.
       */
      void dropIfExistsSequence( const std::string& sequenceName );

      /**
       * Returns the names of all sequences.
       */
      std::set<std::string> listSequences() const;

#endif

      /// Reacts on an End-Of-Transaction signal
      void reactOnEndOfTransaction();

    private:
      /// A reference to the session properties
      const ISessionProperties&  m_sessionProperties;
      /// The schema properties & objects registry
      ISchemaProperties*         m_schemaProperties;
      /// Flag inidicating that list of tables has been retrieved
      mutable bool m_tableListFresh;
      /// Flag inidicating that description of tables has been retrieved from DB server
      mutable bool m_tableDescriptionFresh;
      /// Schema lock
      mutable boost::mutex m_lock;
    };
  }
}
#endif // MYSQLACCESS_SCHEMA_H
