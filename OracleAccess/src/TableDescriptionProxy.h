#ifndef ORACLEACCESS_TABLEDESCRIPTIONPROXY_H
#define ORACLEACCESS_TABLEDESCRIPTIONPROXY_H 1

#include <boost/shared_ptr.hpp>
#include "CoralBase/boost_thread_headers.h"
#include "CoralBase/VersionInfo.h" // Relevant for #ifdef CORAL240xx
#include "RelationalAccess/ITableDescription.h"
#include "RelationalAccess/ITableSchemaEditor.h"

#include "IConstraintRegistry.h"

struct OCIDescribe;

namespace coral
{

  class TableDescription;

  namespace OracleAccess
  {

    class ColumnProxy;
    class SessionProperties;

    /**
     * Class TableDescription
     *
     * Implementation of the ITableDescription and ITableSchemaEditor interfaces for the OracleAccess package
     */
    class TableDescriptionProxy : virtual public coral::ITableDescription,
                                  virtual public coral::ITableSchemaEditor,
                                  virtual public IConstraintRegistry
    {
    public:
      /// Constructor
      TableDescriptionProxy( boost::shared_ptr<const SessionProperties> properties,
                             const std::string& schemaName,
                             OCIDescribe* ociDescribeHandle,
                             const std::string& tableName );

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

      /**
       * Inserts a new column in the table.
       * If the column name already exists or is invalid, an InvalidColumnNameException is thrown.
       */
      void insertColumn( const std::string& name,
                         const std::string& type,
                         int size = 0,
                         bool fixedSize = true,
                         std::string tableSpaceName = "" );

      /**
       * Drops a column from the table.
       * If the column name does not exist or is invalid, an InvalidColumnNameException is thrown.
       */
      void dropColumn( const std::string& name );

      /**
       * Renames a column in the table.
       * If the column name does not exist or is invalid, an InvalidColumnNameException is thrown.
       */
      void renameColumn( const std::string& originalName,
                         const std::string& newName );

      /**
       * Changes the C++ type of a column in the table.
       * If the column name does not exist or is invalid, an InvalidColumnNameException is thrown.
       */
      void changeColumnType( const std::string& columnName,
                             const std::string& typeName,
                             int size = 0,
                             bool fixedSize = true );

      /**
       * Sets or removes a NOT NULL constraint on a column.
       * If the column name does not exist or is invalid, an InvalidColumnNameException is thrown.
       */
      void setNotNullConstraint( const std::string& columnName,
                                 bool isNotNull = true );

      /**
       * Adds or removes a unique constraint on a column.
       * If the column name does not exist or is invalid, an InvalidColumnNameException is thrown.
       * If a unique constrain already exists for the specified column an UniqueConstraintAlreadyExistingException is thrown.
       * If not unique constraint exists for the specified column in the case it is asked to be dropped,
       * an InvalidUniqueConstraintIdentifierException is thrown
       */
      void setUniqueConstraint( const std::string& columnName,
                                std::string name = "",
                                bool isUnique = true,
                                std::string tableSpaceName = "" );

      /**
       * Adds or removes a unique constraint defined over one or more columns.
       * If any of the column names does not exist or is invalid, an InvalidColumnNameException is thrown.
       * If a unique constrain already exists for the specified set of columns an UniqueConstraintAlreadyExistingException is thrown.
       * If not unique constraint exists for the specified set of columns in the case it is asked to be dropped,
       * an InvalidUniqueConstraintIdentifierException is thrown
       */
      void setUniqueConstraint( const std::vector<std::string>& columnNames,
                                std::string name = "",
                                bool isUnique = true,
                                std::string tableSpaceName = "" );

      /**
       * Defines a primary key from a single column.
       * If the column name does not exist or is invalid, an InvalidColumnNameException is thrown.
       * If a primary key has already been defined, an ExistingPrimaryKeyException is thrown.
       */
      void setPrimaryKey( const std::string& columnName,
                          std::string tableSpaceName = "" );

      /**
       * Defines a primary key from one or more columns.
       * If any of the column names does not exist or is invalid, an InvalidColumnNameException is thrown.
       * If a primary key has already been defined, an ExistingPrimaryKeyException is thrown.
       */
      void setPrimaryKey( const std::vector<std::string>& columnNames,
                          std::string tableSpaceName = "" );

      /**
       * Drops the existing primary key.
       * If there is no primary key defined a NoPrimaryKeyException is thrown.
       */
      void dropPrimaryKey();

      /**
       * Creates an index on a column.
       * If the column name does not exist or is invalid, an InvalidColumnNameException is thrown.
       * If an index has already been defined with that name an InvalidIndexIdentifierException is thrown.
       */
      void createIndex( const std::string& indexName,
                        const std::string& columnName,
                        bool isUnique = false,
                        std::string tableSpaceName = "" );

      /**
       * Creates an index over one or more columns.
       * If any of the column names does not exist or is invalid, an InvalidColumnNameException is thrown.
       * If an index has already been defined with that name an InvalidIndexIdentifierException is thrown.
       */
      void createIndex( const std::string& name,
                        const std::vector<std::string>& columnNames,
                        bool isUnique = false,
                        std::string tableSpaceName = "" );

      /**
       * Drops an existing index.
       * If the specified index name is not valid an InvalidIndexIdentifierException is thrown.
       */
      void dropIndex( const std::string& indexName );

      /**
       * Creates a foreign key constraint.
       * If the column name does not exist or is invalid, an InvalidColumnNameException is thrown.
       * If a foreign key has already been defined with that name an InvalidForeignKeyIdentifierException is thrown.
       */
#ifdef CORAL240DC
      void createForeignKey( const std::string& name,
                             const std::string& columnName,
                             const std::string& referencedTableName,
                             const std::string& referencedColumnName,
                             bool onDeleteCascade = false );
#else
      void createForeignKey( const std::string& name,
                             const std::string& columnName,
                             const std::string& referencedTableName,
                             const std::string& referencedColumnName );
#endif
      /**
       * Creates a foreign key over one or more columns.
       * If any of the column names does not exist or is invalid, an InvalidColumnNameException is thrown.
       * If a foreign key has already been defined with that name an InvalidForeignKeyIdentifierException is thrown.
       */
#ifdef CORAL240DC
      void createForeignKey( const std::string& name,
                             const std::vector<std::string>& columnNames,
                             const std::string& referencedTableName,
                             const std::vector<std::string>& referencedColumnNames,
                             bool onDeleteCascade = false );
#else
      void createForeignKey( const std::string& name,
                             const std::vector<std::string>& columnNames,
                             const std::string& referencedTableName,
                             const std::vector<std::string>& referencedColumnNames );
#endif
      /**
       * Drops a foreign key.
       * If the specified name is not valid an InvalidForeignKeyIdentifierException is thrown.
       */
      void dropForeignKey( const std::string& name );

      /// Reads the unique constraints
      void refreshConstraints() const;

      /// Returns the name of the associated schema
      const std::string& schemaName() const
      {
        return m_schemaName;
      }

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
      boost::shared_ptr<const SessionProperties> m_sessionProperties;

      /// The schema name for this table
      const std::string m_schemaName;

      /// The OCI describe handle
      OCIDescribe* m_ociDescribeHandle;

      /// The transient table description object
      coral::TableDescription* m_description;

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

      /// The mutex protecting the data members
      mutable boost::mutex m_mutex;
    };

  }

}

#endif
