// $Id: TableDDLBuilder.cpp,v 1.12 2011/03/22 10:29:54 avalassi Exp $
#include "MySQL_headers.h"

#include <cstdio>
#include <cstdlib>
#include <iostream>
#include "CoralBase/AttributeSpecification.h"
#include "CoralBase/Blob.h"
#include "RelationalAccess/IColumn.h"
#include "RelationalAccess/IForeignKey.h"
#include "RelationalAccess/IIndex.h"
#include "RelationalAccess/IPrimaryKey.h"
#include "RelationalAccess/ITableDescription.h"
#include "RelationalAccess/ITypeConverter.h"
#include "RelationalAccess/IUniqueConstraint.h"

#include "ColumnConstraint.h"
#include "ColumnDefinition.h"
#include "TableDDLBuilder.h"

namespace coral
{
  namespace MySQLAccess
  {
    TableDDLBuilder::TableDDLBuilder( const ITypeConverter& tc, const ITableDescription& td, const std::string& db )
      : m_tc( tc )
      , m_td( td )
      , m_db( db )
      , m_definition( "CREATE TABLE \"" + m_db + "\".\"" + m_td.name() + "\" (" )
    {
    }

    /// Builds the proper DDL SQL statement according to the given table description
    std::string TableDDLBuilder::sql()
    {
      // Build the column definitions
      defineColumns();

      // Now build constraints & indexes

      if( m_td.hasPrimaryKey() )
        definePrimaryKey();

      if( m_td.numberOfUniqueConstraints() > 0 )
        defineUniqueConstraints();

      if( m_td.numberOfIndices() > 0 )
        defineIndices();

      if( m_td.numberOfForeignKeys() > 0 )
        defineForeignKeys();

      m_definition += ")";

      return m_definition;
    }

    unsigned int TableDDLBuilder::checkEnvClobPrefixLength() const
    {
      unsigned int clobPrefixLength = 32;
      const char* clobPrefixEnvSetting = getenv( "CORAL_MYSQL_CLOB_PREFIX_LEN" );
      if( clobPrefixEnvSetting != 0 )
        // FIXME - max. allowed prefix len in MySQL 4.0, for 4.1 & later it can be longer
        if( (sscanf( clobPrefixEnvSetting, "%ud", &clobPrefixLength ) != 1) || clobPrefixLength > 255 ) // FIXME
          clobPrefixLength = 32;
      return clobPrefixLength;
    }

    void TableDDLBuilder::defineColumns()
    {
      // Guess the SQL column type
      size_t numColumns = m_td.numberOfColumns();

      for( size_t iColumn = 0; iColumn < numColumns; ++iColumn )
      {
        if( iColumn > 0 )
          m_definition += ", ";

        ColumnDefinition coldef;

        const coral::IColumn& column = m_td.columnDescription( iColumn );

        coldef.name = column.name();

        // Ask type converter for the suggested SQL type
        coldef.type     = m_tc.sqlTypeForCppType( column.type() );

        // What value domain is it?
        // FIXME - how to detect date, time or timestamp?
        // FIXME - TODO in the future releases of CORAL
        if( *(coral::AttributeSpecification::typeIdForName( column.type() )) == typeid( std::string ) )
          coldef.domain = coral::MySQLAccess::String;
        else if( *(coral::AttributeSpecification::typeIdForName( column.type() )) == typeid( coral::Blob ) )
          coldef.domain = coral::MySQLAccess::Blob;
        else
        {
          coldef.domain = coral::MySQLAccess::Numeric;

          //std::cout << "BEFORE Column: " << coldef.name << " C++ type: " << column.type() << " SQL type: " << coldef.type << std::endl;

          // FIXME!!!! Hack to fix wrong DDL for unsigned numeric types
          size_t lenb = std::string::npos; size_t lene = std::string::npos;
          if( (lenb=coldef.type.find("UNSIGNED")) != std::string::npos )
          {
            coldef.type.erase(lenb,8);
            coldef.is_unsigned = true;

            //std::cout << "AFTER removing UNSIGNED Column: " << coldef.name << " C++ type: " << column.type() << " SQL type: " << coldef.type << std::endl;

            // Make sure that the type converter did not put a size in already
            lenb = coldef.type.find('(');

            if( lenb != std::string::npos )
            {
              lene = coldef.type.find(lenb, ')');
              //std::cout << "AFTER removing >>" << coldef.type.substr(lenb, lene-lenb);
              coldef.type.erase( lenb, lene-lenb );
              //std::cout << "<< Column: " << coldef.name << " type: " << coldef.type << std::endl;
            }
          }
        }

        // Check the size & variability
        coldef.length   = column.size();
        coldef.fixed    = column.isSizeFixed();

        // Nullable?
        coldef.nullable = !column.isNotNull();

        // Unique?
        coldef.unique   = column.isUnique();

        m_definition   += coldef.sql();
      }
    }

    void TableDDLBuilder::definePrimaryKey()
    {
      ColumnConstraint colcons;

      colcons.type     = coral::MySQLAccess::Primary;
      colcons.name     = m_td.name();


      // FIXME - MySQL required workaround for CLOB types in case they are used as primary key,
      // FIXME - this will be done properly in future CORAL releases
      // FIXME - as CORAL provides required interface changes to support index prefixes for all types of constraints
      const std::vector<std::string>& pkColumns = m_td.primaryKey().columnNames();
      size_t numColumns                         = pkColumns.size();
      for( size_t iColumn = 0; iColumn < numColumns; ++iColumn )
      {
        const coral::IColumn& column = m_td.columnDescription( pkColumns[iColumn] );
        if( *(coral::AttributeSpecification::typeIdForName( column.type() )) == typeid( std::string ) && column.size() > 255 )
        { /* MySQL can store it as CLOB only */
          colcons.keys.push_back( IndexedColumn( column.name(), checkEnvClobPrefixLength() ) );
        }
        else
          // FIXME - TODO index prefix in future CORAL releases
          colcons.keys.push_back( IndexedColumn( column.name(), 0 /* Means don't use index prefix */ ) );  // FIXME
      }

      if( ! colcons.sql().empty() )
        m_definition      += ", " + colcons.sql();
    }

    void TableDDLBuilder::defineUniqueConstraints()
    {
      for( int unicons = 0; unicons < m_td.numberOfUniqueConstraints(); ++unicons )
      {
        m_definition += ", ";

        ColumnConstraint colcons;

        colcons.type     = coral::MySQLAccess::Unique;
        colcons.name     = m_td.uniqueConstraint(unicons).name();

        // FIXME - MySQL required workaround for CLOB types in case they are used as unique key, will be done properly in future CORAL releases
        // FIXME - as CORAL provides required interface changes to support index prefixes for all types of constraints
        const std::vector<std::string>& ukColumns = m_td.uniqueConstraint(unicons).columnNames();
        size_t numColumns                         = ukColumns.size();
        for( size_t iColumn = 0; iColumn < numColumns; ++iColumn )
        {
          const coral::IColumn& column = m_td.columnDescription( ukColumns[iColumn] );
          if( *(coral::AttributeSpecification::typeIdForName( column.type() )) == typeid( std::string ) && column.size() > 255 )
          { /* MySQL can store it as CLOB only */
            colcons.keys.push_back( IndexedColumn( column.name(), checkEnvClobPrefixLength() ) );
          }
          else
            // FIXME - TODO index prefix in future CORAL releases
            colcons.keys.push_back( IndexedColumn( column.name(), 0 /* Means don't use index prefix */ ) );  // FIXME
        }

        m_definition      += colcons.sql();
      }
    }

    void TableDDLBuilder::defineForeignKeys()
    {
      for( int fkcons = 0; fkcons < m_td.numberOfForeignKeys(); ++fkcons )
      {
        ColumnConstraint colcons;
        ColumnConstraint idxcons; // MySQL requires that for each FK there is index in referencing table, in the referenced one as well

        // FK setup
        colcons.type     = coral::MySQLAccess::Foreign;
        colcons.name     = m_td.foreignKey(fkcons).name();

        // FK index setup
        idxcons.type     = coral::MySQLAccess::None;
        idxcons.name     = m_td.foreignKey(fkcons).name();

        const std::vector<std::string>& fkColumns = m_td.foreignKey(fkcons).columnNames();
        size_t numColumns                         = fkColumns.size();

        // Setup for foreign key columns, we don't consider index prefixes for foreign keys
        for( size_t iColumn = 0; iColumn < numColumns; ++iColumn )
        {
          const coral::IColumn& column = m_td.columnDescription( fkColumns[iColumn] );
          colcons.keys.push_back( IndexedColumn( column.name(), 0 /* Means don't use index prefix */ ) );
          idxcons.keys.push_back( IndexedColumn( column.name(), 0 /* Means don't use index prefix */ ) );
        }

        // Setup for referenced table & columns
        colcons.refTable   = m_td.foreignKey(fkcons).referencedTableName();
        colcons.refColumns = m_td.foreignKey(fkcons).referencedColumnNames();


        if( m_builtIdx.find( idxcons.name ) == m_builtIdx.end() )
        {
          m_builtIdx.insert( idxcons.name );

          if( ! idxcons.sql().empty() )
            m_definition += ", " + idxcons.sql();
        }

        if( ! colcons.sql().empty() )
          m_definition += ", " + colcons.sql();
      }
    }

    void TableDDLBuilder::defineIndices()
    {
      for( int idxcons = 0; idxcons < m_td.numberOfIndices(); ++idxcons )
      {
        ColumnConstraint colcons;

        colcons.type     = coral::MySQLAccess::None; // FIXME - we might have defined the dedicated enum for this
        colcons.name     = m_td.index(idxcons).name();

        // This gets automatically created for unique not null column
        if( colcons.name == "PRIMARY" )
          continue;

        m_builtIdx.insert( colcons.name );

        if( m_td.index(idxcons).isUnique() )
          colcons.uniqueIndex = true;

        // FIXME - MySQL required workaround for CLOB types in case they are used as unique key,
        // FIXME - this will be done properly in future CORAL releases
        // FIXME - as CORAL provides required interface changes to support index prefixes for all types of constraints
        const std::vector<std::string>& idxColumns = m_td.index(idxcons).columnNames();
        size_t numColumns                          = idxColumns.size();
        for( size_t iColumn = 0; iColumn < numColumns; ++iColumn )
        {
          const coral::IColumn& column = m_td.columnDescription( idxColumns[iColumn] );
          if( *(coral::AttributeSpecification::typeIdForName( column.type() )) == typeid( std::string ) && column.size() > 255 )
          { /* MySQL can store it as CLOB only */
            colcons.keys.push_back( IndexedColumn( column.name(), checkEnvClobPrefixLength() ) );
          }
          else
            // FIXME - TODO index prefix in future CORAL releases
            colcons.keys.push_back( IndexedColumn( column.name(), 0 /* Means don't use index prefix */ ) );  // FIXME
        }

        if( ! colcons.sql().empty() )
          m_definition += ", " + colcons.sql();
      }
    }
  }
}
