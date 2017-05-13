// $Id: TableDescriptionProxy.h,v 1.7.2.1 2010/12/20 09:10:09 avalassi Exp $
#ifndef CORALACCESS_TABLEDESCRIPTIONPROXY_H
#define CORALACCESS_TABLEDESCRIPTIONPROXY_H 1

// Include files
#include "RelationalAccess/TableDescription.h"

namespace coral
{

  namespace CoralAccess
  {

    // Forward declarations
    class SessionProperties;

    /** @class TableDescriptionProxy
     *
     *  Proxy for loading on demand the the description of a table
     *  (or the description and definition of a view).
     *
     *  @author Andrea Valassi
     *  @date   2009-04-23
     */

    class TableDescriptionProxy : virtual public ITableDescription
    {

    public:

      /// Constructor
      TableDescriptionProxy( const SessionProperties& sessionProperties,
                             const std::string& schemaName,
                             const std::string& tableName,
                             bool isView = false );

      /// Destructor
      virtual ~TableDescriptionProxy();

      /**
       * Returns the name of the table (or view).
       */
      std::string name() const;

      /**
       * Returns the table type (RDBMS SPECIFIC)
       */
      std::string type() const
      {
        return proxiedTableDescription().type();
      }

      /**
       * Returns the name of the table space for this table.
       */
      std::string tableSpaceName() const
      {
        return proxiedTableDescription().tableSpaceName();
      }

      /**
       * Returns the number of columns in the table.
       */
      int numberOfColumns() const
      {
        return proxiedTableDescription().numberOfColumns();
      }

      /**
       * Returns the description of the column corresponding to the specified index.
       * If the index is out of range an InvalidColumnIndexException is thrown.
       */
      const IColumn& columnDescription( int columnIndex ) const
      {
        return proxiedTableDescription().columnDescription( columnIndex );
      }

      /**
       * Returns the description of the column corresponding to the specified name.
       * If the specified column name is invalid an InvalidColumnNameException is thrown.
       */
      const IColumn& columnDescription( const std::string& columnName ) const
      {
        return proxiedTableDescription().columnDescription( columnName );
      }

      /**
       * Returns the existence of a primary key in the table.
       */
      bool hasPrimaryKey() const
      {
        return proxiedTableDescription().hasPrimaryKey();
      }

      /**
       * Returns the primary key for the table. If there is no primary key a NoPrimaryKeyException is thrown.
       */
      const IPrimaryKey& primaryKey() const
      {
        return proxiedTableDescription().primaryKey();
      }

      /**
       * Returns the number of indices defined in the table.
       */
      int numberOfIndices() const
      {
        return proxiedTableDescription().numberOfIndices();
      }

      /**
       * Returns the index corresponding to the specified identitier.
       * If the identifier is out of range an InvalidIndexIdentifierException is thrown.
       */
      const IIndex& index( int indexId ) const
      {
        return proxiedTableDescription().index( indexId );
      }

      /**
       * Returns the number of foreign key constraints defined in the table.
       */
      int numberOfForeignKeys() const
      {
        return proxiedTableDescription().numberOfForeignKeys();
      }

      /**
       * Returns the foreign key corresponding to the specified identifier.
       * In case the identifier is out of range an InvalidForeignKeyIdentifierException is thrown.
       */
      const IForeignKey& foreignKey( int foreignKeyIdentifier ) const
      {
        return proxiedTableDescription().foreignKey( foreignKeyIdentifier );
      }

      /**
       * Returns the number of unique constraints defined in the table.
       */
      int numberOfUniqueConstraints() const
      {
        return proxiedTableDescription().numberOfUniqueConstraints();
      }

      /**
       * Returns the unique constraint for the specified identifier.
       * If the identifier is out of range an InvalidUniqueConstraintIdentifierException is thrown.
       */
      const IUniqueConstraint& uniqueConstraint( int uniqueConstraintIdentifier ) const
      {
        return proxiedTableDescription().uniqueConstraint( uniqueConstraintIdentifier );
      }

      /// Return the view definition (load it on demand if needed).
      /// [This method can be const because the definition is mutable].
      /// Throw InternalErrorException if the table is not a view.
      const std::string& viewDefinition() const;

    private:

      /// Return the proxied table description (load it on demand if needed).
      /// [This method can be const because the pointer is mutable].
      const ITableDescription& proxiedTableDescription() const;

    private:

      /// The session properties.
      const SessionProperties& m_sessionProperties;

      /// The schema name.
      /// [NB this is guaranteed to be != ""]
      const std::string m_schemaName;

      /// The table (or view) name.
      const std::string m_tableName;

      /// Is the table a view?
      const bool m_isView;

      /// The proxied table description (owned by this instance).
      mutable TableDescription* m_pDesc;

      /// The view definition (only relevant if this is a view).
      mutable std::string m_viewDefinition;

    };

  }

}
#endif // CORALACCESS_TABLEDESCRIPTIONPROXY_H
