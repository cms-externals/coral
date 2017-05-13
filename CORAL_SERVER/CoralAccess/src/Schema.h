#ifndef CORALACCESS_SCHEMA_H
#define CORALACCESS_SCHEMA_H

// Include files
#include "CoralBase/VersionInfo.h" // Relevant for #ifdef CORAL240xx
#include "CoralServerBase/ICoralFacade.h"
#include "RelationalAccess/ISchema.h"

namespace coral
{

  namespace CoralAccess
  {

    // Forward declarations
    class SessionProperties;
    class Table;
    class View;

    /** @class Schema
     *
     *  @author Andrea Valassi
     *  @date   2007-12-05
     */

    class Schema : virtual public coral::ISchema
    {

    public:

      /// Constructor
      Schema( const SessionProperties& sessionProperties,
              const std::string& schemaName ); // NB This must be != ""

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
      ITable& createTable( const ITableDescription& description );

      /**
       * Returns a reference to an ITable object corresponding to the table with the specified name.
       * In case no table with such a name exists, a TableNotExistingException is thrown.
       */
      ITable& tableHandle( const std::string& tableName );

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
      IQuery* newQuery() const;

      /**
       * Returns a new view factory object in order to define and create a view.
       */
      IViewFactory* viewFactory();

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
      IView& viewHandle( const std::string& viewName );

#ifdef CORAL240CN
      /**
       * Register a change notification callback for a table in this schema.
       * WARNING: this API extension is disabled by default (use -DCORAL240CN).
       */
      void registerNotification( const std::string& /*tableName*/,
                                 IChangeNotification& /*callback*/ ) {}
#endif

      /// Returns the facade for the CORAL server connection.
      const coral::ICoralFacade& facade() const;

    private:

      /// The properties of this remote database session.
      const SessionProperties& m_sessionProperties;

      /// The schema name.
      /// [NB this is guaranteed to be != ""]
      const std::string m_schemaName;

      /// The map of existing tables (all owned by this Schema).
      /// A null table pointer means that the table exists,
      /// but its handle has not been instantiated yet.
      /// [NB mutable because listTables is declared as const but is not!]
      mutable std::map< std::string, Table* > m_tables;

      /// Has the list of tables been fetched from the data dictionary?
      /// This flag refers to the table list (not to all table descriptions).
      /// [NB mutable because listTables is declared as const but is not!]
      mutable bool m_tableListFresh;

      /// The map of existing views (all owned by this Schema).
      /// A null view pointer means that the view exists,
      /// but its handle has not been instantiated yet.
      /// [NB mutable because listViews is declared as const but is not!]
      mutable std::map< std::string, View* > m_views;

      /// Has the list of views been fetched from the data dictionary?
      /// This flag refers to the view list (not to all view descriptions).
      /// [NB mutable because listViews is declared as const but is not!]
      mutable bool m_viewListFresh;

    };

  }

}
#endif
