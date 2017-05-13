// $Id: TableSchemaEditor.h,v 1.8 2011/03/22 10:29:54 avalassi Exp $
#ifndef CORAL_MYSQLACCESS_TABLESCHEMAEDITOR_H
#define CORAL_MYSQLACCESS_TABLESCHEMAEDITOR_H 1

#include <string>
#include <vector>
#include "CoralBase/VersionInfo.h" // Relevant for #ifdef CORAL240xx
#include "RelationalAccess/ITableSchemaEditor.h"
#include "ColumnConstraint.h"

namespace coral
{
  class TableDescription;

  namespace MySQLAccess
  {
    class ISessionProperties;
    class ISchemaProperties;

    class TableSchemaLoader;

    /**
     * Class TableSchemaEditor
     * Provides means of altering the schema of an existing table
     */
    class TableSchemaEditor : virtual public coral::ITableSchemaEditor
    {
    public:
      /**
       * Constructor
       */
      TableSchemaEditor( const std::string& tableName, const ISessionProperties& sessionProperties, ISchemaProperties& schemaProperties, bool refresh=true );

      /**
       * Destructor
       */
      virtual ~TableSchemaEditor();

      /**
       * Inserts a new column in the table.
       * If the column name already exists or is invalid, an InvalidColumnNameException is thrown.
       */
      virtual void insertColumn( const std::string& name, const std::string& type, int size = 0, bool fixedSize = true, std::string tableSpaceName = "" );

      /**
       * Drops a column from the table.
       * If the column name does not exist or is invalid, an InvalidColumnNameException is thrown.
       */
      virtual void dropColumn( const std::string& name );

      /**
       * Renames a column in the table.
       * If the column name does not exist or is invalid, an InvalidColumnNameException is thrown.
       */
      virtual void renameColumn( const std::string& originalName, const std::string& newName );

      /**
       * Changes the C++ type of a column in the table.
       * If the column name does not exist or is invalid, an InvalidColumnNameException is thrown.
       */
      virtual void changeColumnType( const std::string& columnName, const std::string& typeName, int size = 0, bool fixedSize = true );

      /**
       * Sets or removes a NOT NULL constraint on a column.
       * If the column name does not exist or is invalid, an InvalidColumnNameException is thrown.
       */
      virtual void setNotNullConstraint( const std::string& columnName, bool isNotNull = true );

      /**
       * Adds or removes a unique constraint on a column.
       * If the column name does not exist or is invalid, an InvalidColumnNameException is thrown.
       * If a unique constrain already exists for the specified column an UniqueConstraintAlreadyExistingException is thrown.
       * If not unique constraint exists for the specified column in the case it is asked to be dropped,
       * an InvalidUniqueConstraintIdentifierException is thrown
       */
      virtual void setUniqueConstraint( const std::string& columnName, std::string name = "", bool isUnique = true, std::string tableSpaceName = "" );

      /**
       * Adds or removes a unique constraint defined over one or more columns.
       * If any of the column names does not exist or is invalid, an InvalidColumnNameException is thrown.
       * If a unique constrain already exists for the specified set of columns an UniqueConstraintAlreadyExistingException is thrown.
       * If not unique constraint exists for the specified set of columns in the case it is asked to be dropped,
       * an InvalidUniqueConstraintIdentifierException is thrown
       */
      virtual void setUniqueConstraint( const std::vector<std::string>& columnNames, std::string name = "", bool isUnique = true, std::string tableSpaceName = "" );

      /**
       * Defines a primary key from a single column.
       * If the column name does not exist or is invalid, an InvalidColumnNameException is thrown.
       * If a primary key has already been defined, an ExistingPrimaryKeyException is thrown.
       */
      virtual void setPrimaryKey( const std::string& columnName, std::string tableSpaceName = "" );

      /**
       * Defines a primary key from one or more columns.
       * If any of the column names does not exist or is invalid, an InvalidColumnNameException is thrown.
       * If a primary key has already been defined, an ExistingPrimaryKeyException is thrown.
       */
      virtual void setPrimaryKey( const std::vector<std::string>& columnNames, std::string tableSpaceName = "" );

      /**
       * Drops the existing primary key.
       * If there is no primary key defined a NoPrimaryKeyException is thrown.
       */
      virtual void dropPrimaryKey();

      /**
       * Creates an index on a column.
       * If the column name does not exist or is invalid, an InvalidColumnNameException is thrown.
       * If an index has already been defined with that name an InvalidIndexIdentifierException is thrown.
       */
      virtual void createIndex( const std::string& indexName, const std::string& columnName, bool isUnique = false, std::string tableSpaceName = "" );

      /**
       * Creates an index over one or more columns.
       * If any of the column names does not exist or is invalid, an InvalidColumnNameException is thrown.
       * If an index has already been defined with that name an InvalidIndexIdentifierException is thrown.
       */
      virtual void createIndex( const std::string& name, const std::vector<std::string>& columnNames, bool isUnique = false, std::string tableSpaceName = "" );

      /**
       * Drops an existing index.
       * If the specified index name is not valid an InvalidIndexIdentifierException is thrown.
       */
      virtual void dropIndex( const std::string& indexName );

      /**
       * Creates a foreign key constraint.
       * If the column name does not exist or is invalid, an InvalidColumnNameException is thrown.
       * If a foreign key has already been defined with that name an InvalidForeignKeyIdentifierException is thrown.
       */
#ifdef CORAL240DC
      virtual void createForeignKey( const std::string& name, const std::string& columnName, const std::string& referencedTableName, const std::string& referencedColumnName, bool onDeleteCascade = false );
#else
      virtual void createForeignKey( const std::string& name, const std::string& columnName, const std::string& referencedTableName, const std::string& referencedColumnName );
#endif
      /**
       * Creates a foreign key over one or more columns.
       * If any of the column names does not exist or is invalid, an InvalidColumnNameException is thrown.
       * If a foreign key has already been defined with that name an InvalidForeignKeyIdentifierException is thrown.
       */
#ifdef CORAL240DC
      virtual void createForeignKey( const std::string& name, const std::vector<std::string>& columnNames, const std::string& referencedTableName, const std::vector<std::string>& referencedColumnNames, bool onDeleteCascade = false );
#else
      virtual void createForeignKey( const std::string& name, const std::vector<std::string>& columnNames, const std::string& referencedTableName, const std::vector<std::string>& referencedColumnNames );
#endif
      /**
       * Drops a foreign key.
       * If the specified name is not valid an InvalidForeignKeyIdentifierException is thrown.
       */
      virtual void dropForeignKey( const std::string& name );

    private:
      /// Build colum DDL fragment
      std::string buildColumn( const std::string& name, const std::string& type, int size= 0, bool fixedSize= true );
      /// Build column constraint DDL fragment
      std::string buildConstraint( ConstraintType ct, const std::vector<std::string>& columnNames, const std::string& name, const std::vector<std::string>& refColumnNames, const std::string& refTableName, bool isUnique );
      /// FIXME - refactoring needed, copied from TableDDLBuilder class
      unsigned int checkEnvClobPrefixLength() const;

    private:
      /// Table name
      std::string m_tableName;
      /// Schema name / shortcut :->
      std::string m_schemaName;
      /// Session properties
      const ISessionProperties& m_sessionProperties;
      /// Schema properties
      ISchemaProperties&        m_schemaProperties;
      /// Editable table description
      coral::TableDescription*  m_description;
      /// schema loader
      TableSchemaLoader*        m_tableSchemaLoader;
    };
  }
}

#endif // CORAL_MYSQLACCESS_TABLESCHEMAEDITOR_H
