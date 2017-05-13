#ifndef FRONTIER_ACCESS_TABLE_DESCRIPTION_PROXY_H
#define FRONTIER_ACCESS_TABLE_DESCRIPTION_PROXY_H

#include "RelationalAccess/ITableDescription.h"
#include "RelationalAccess/ITableSchemaEditor.h"
#include "IConstraintRegistry.h"

namespace coral {

  class TableDescription;

  namespace FrontierAccess {

    class ISessionProperties;
    class ColumnProxy;

    /**
     * Class TableDescription
     *
     * Implementation of the ITableDescription interface for the FrontierAccess package
     */
    class TableDescriptionProxy : virtual public coral::ITableDescription, virtual public IConstraintRegistry
    {
    public:
      /// Constructor
      TableDescriptionProxy( const ISessionProperties& properties, const std::string& tableName );

      /// Destructor
      virtual ~TableDescriptionProxy();

      /**
       * Returns the name of the table.
       */
      std::string name() const;

      /**
       * Returns the table type (RDBMS SPECIFIC)
       */
      std::string type() const;

      /**
       * Returns the name of the table space for this table.
       */
      std::string tableSpaceName() const;

      /**
       * Returns the number of columns in the table.
       */
      int numberOfColumns() const;

      /**
       * Returns the description of the column corresponding to the specified index.
       * If the index is out of range an InvalidColumnIndexException is thrown.
       */
      const coral::IColumn& columnDescription( int columnIndex ) const;

      /**
       * Returns the description of the column corresponding to the specified name.
       * If the specified column name is invalid an InvalidColumnNameException is thrown.
       */
      const coral::IColumn& columnDescription( const std::string& columnName ) const;

      /**
       * Returns the existence of a primary key in the table.
       */
      bool hasPrimaryKey() const;

      /**
       * Returns the primary key for the table. If there is no primary key a NoPrimaryKeyException is thrown.
       */
      const coral::IPrimaryKey& primaryKey() const;

      /**
       * Returns the number of indices defined in the table.
       */
      int numberOfIndices() const;

      /**
       * Returns the index corresponding to the specified identitier.
       * If the identifier is out of range an InvalidIndexIdentifierException is thrown.
       */
      const coral::IIndex& index( int indexId ) const;


      /**
       * Returns the number of foreign key constraints defined in the table.
       */
      int numberOfForeignKeys() const;

      /**
       * Returns the foreign key corresponding to the specified identifier.
       * In case the identifier is out of range an InvalidForeignKeyIdentifierException is thrown.
       */
      const coral::IForeignKey& foreignKey( int foreignKeyIdentifier ) const;

      /**
       * Returns the number of unique constraints defined in the table.
       */
      int numberOfUniqueConstraints() const;

      /**
       * Returns the unique constraint for the specified identifier.
       * If the identifier is out of range an InvalidUniqueConstraintIdentifierException is thrown.
       */
      const coral::IUniqueConstraint& uniqueConstraint( int uniqueConstraintIdentifier ) const;

      /// Reads the unique constraints
      void refreshConstraints() const;

    private:
      /// Reads the column description from the database
      void readColumnDescription();

      /// Reads the primary key from the database
      void refreshPrimaryKeyInfo() const;

      /// Reads the indices from the database
      void refreshIndexInfo() const;

      /// Reads the foreign keys from the database
      void refreshForeignKeyInfo() const;

    private:
      /// The session properties
      const ISessionProperties& m_sessionProperties;

      /*       /// The OCI describe handle */
      /*       OCIDescribe*              m_describeHandle; */

      /// The transient table description object
      coral::TableDescription*  m_description;

      /// Flag indicating whether the column description has been read from the database
      bool m_columnDescriptionRead;

      /// The column proxies
      std::vector< ColumnProxy* > m_columns;

      /// Flag indicating whether the unique constraints have been read from the database
      mutable bool m_uniqueConstraintsRead;

      /// Flag indicating whether the primary key information has been updated
      mutable bool m_primaryKeyRead;

      /// Flag indicating whether the index information has been updated
      mutable bool m_indicesRead;

      /// Flag indicating whether the foreign key information has been updated
      mutable bool m_foreignKeysRead;

    };

  }

}

#endif
