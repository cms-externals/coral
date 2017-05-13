#include <algorithm>
#include "CoralBase/VersionInfo.h" // Relevant for #ifdef CORAL240xx
#include "RelationalAccess/TableDescription.h"
#include "RelationalAccess/SchemaException.h"
#include "Column.h"
#include "ForeignKey.h"
#include "Index.h"
#include "PrimaryKey.h"
#include "UniqueConstraint.h"

coral::TableDescription::TableDescription( std::string context ) :
  m_context( context ),
  m_name( "" ),
  m_type( "" ),
  m_tableSpaceName( "" ),
  m_columns(),
  m_uniqueConstraints(),
  m_primaryKey( 0 ),
  m_indices(),
  m_foreignKeys()
{
}


coral::TableDescription::TableDescription( const coral::TableDescription& rhs )
  : ITableSchemaEditor( rhs )
  , ITableDescription( rhs )
  , m_context( "")
  , m_name( "" )
  , m_type( "" )
  , m_tableSpaceName( "" )
  , m_columns()
  , m_uniqueConstraints()
  , m_primaryKey( 0 )
  , m_indices()
  , m_foreignKeys()
{
  this->operator=( rhs );
}


coral::TableDescription& coral::TableDescription::operator=( const coral::TableDescription& rhs )
{
  this->clear();
  this->m_context = rhs.m_context;
  this->m_name = rhs.m_name;
  this->m_type = rhs.m_type;
  this->m_tableSpaceName = rhs.m_tableSpaceName;
  for ( size_t i = 0; i < rhs.m_columns.size(); ++i )
    this->m_columns.push_back( new Column( *(rhs.m_columns[i]) ) );
  for ( size_t i = 0; i < rhs.m_uniqueConstraints.size(); ++i )
    this->m_uniqueConstraints.push_back( new UniqueConstraint( *(rhs.m_uniqueConstraints[i]) ) );
  if ( rhs.m_primaryKey )
    this->m_primaryKey = new PrimaryKey( *(rhs.m_primaryKey) );
  for ( size_t i = 0; i < rhs.m_indices.size(); ++i )
    this->m_indices.push_back( new Index( *(rhs.m_indices[i]) ) );
  for ( size_t i = 0; i < rhs.m_foreignKeys.size(); ++i )
    this->m_foreignKeys.push_back( new ForeignKey( *(rhs.m_foreignKeys[i]) ) );
  return *this;
}


coral::TableDescription::TableDescription( const coral::ITableDescription& rhs,
                                           std::string context ) :
  m_context( context ),
  m_name( rhs.name() ),
  m_type( rhs.type() ),
  m_tableSpaceName( rhs.tableSpaceName() ),
  m_columns(),
  m_uniqueConstraints(),
  m_primaryKey( 0 ),
  m_indices(),
  m_foreignKeys()
{
  // Add the columns
  int numberOfColumns = rhs.numberOfColumns();
  for ( int i = 0; i < numberOfColumns; ++i ) {
    const coral::IColumn& column = rhs.columnDescription( i );
    this->insertColumn( column.name(),
                        column.type(),
                        column.size(),
                        column.isSizeFixed() ); // Ommit the tablespace
    if ( column.isNotNull() )
      this->setNotNullConstraint( column.name() );
  }

  // Add the primary key
  if ( rhs.hasPrimaryKey() ) {
    this->setPrimaryKey( rhs.primaryKey().columnNames(),
                         rhs.primaryKey().tableSpaceName() );
  }

  // Add the unique constraints
  int numberOfUniqueConstraints = rhs.numberOfUniqueConstraints();
  for ( int i = 0; i < numberOfUniqueConstraints; ++i ) {
    this->setUniqueConstraint( rhs.uniqueConstraint( i ).columnNames() );
  }
}


coral::TableDescription::~TableDescription()
{
  this->clear();
}


void coral::TableDescription::clear()
{
  m_context = "";
  m_name = "";
  m_type = "";
  m_tableSpaceName = "";
  for ( std::vector< coral::Column* >::iterator iColumn = m_columns.begin();
        iColumn != m_columns.end(); ++iColumn ) delete *iColumn;
  m_columns.clear();
  for ( std::vector< coral::UniqueConstraint* >::iterator iUniqueConstraint = m_uniqueConstraints.begin();
        iUniqueConstraint != m_uniqueConstraints.end(); ++iUniqueConstraint ) delete *iUniqueConstraint;
  m_uniqueConstraints.clear();
  if ( m_primaryKey ) delete m_primaryKey;
  m_primaryKey = 0;
  for ( std::vector< coral::Index* >::iterator iIndex = m_indices.begin();
        iIndex != m_indices.end(); ++iIndex ) delete *iIndex;
  m_indices.clear();
  for ( std::vector< coral::ForeignKey* >::iterator iForeignKey = m_foreignKeys.begin();
        iForeignKey != m_foreignKeys.end(); ++iForeignKey ) delete *iForeignKey;
  m_foreignKeys.clear();
}


void
coral::TableDescription::setName( const std::string& tableName )
{
  m_name = tableName;
}


void
coral::TableDescription::setType( const std::string& tableType )
{
  m_type = tableType;
}

void
coral::TableDescription::setTableSpaceName( const std::string& tableSpaceName )
{
  m_tableSpaceName = tableSpaceName;
}


std::string
coral::TableDescription::name() const
{
  return m_name;
}


std::string
coral::TableDescription::type() const
{
  return m_type;
}


std::string
coral::TableDescription::tableSpaceName() const
{
  return m_tableSpaceName;
}


int
coral::TableDescription::numberOfColumns() const
{
  return static_cast<int>( m_columns.size() );
}


void
coral::TableDescription::insertColumn( const std::string& name,
                                       const std::string& type,
                                       int size,
                                       bool fixedSize,
                                       std::string tableSpaceName )
{
  for ( std::vector< coral::Column* >::const_iterator iColumn = m_columns.begin();
        iColumn != m_columns.end(); ++iColumn )
    if ( ( *iColumn )->name() == name )
      throw coral::InvalidColumnNameException( m_context,
                                               "ITableSchemaEditor::insertColumn" );
  coral::Column* column = new coral::Column( name, type, size, fixedSize, tableSpaceName );
  column->setIndexInTable( static_cast<int>( m_columns.size() ) );
  m_columns.push_back( column );
}


const coral::IColumn&
coral::TableDescription::columnDescription( int columnIndex ) const
{
  if ( columnIndex < 0 || columnIndex >= static_cast<int>( m_columns.size() ) )
    throw coral::InvalidColumnIndexException( m_context );
  return *( m_columns[columnIndex] );
}


const coral::IColumn&
coral::TableDescription::columnDescription( const std::string& columnName ) const
{
  const coral::IColumn* pColumn = 0;
  for ( std::vector< coral::Column* >::const_iterator iColumn = m_columns.begin();
        iColumn != m_columns.end(); ++iColumn )
    if ( ( *iColumn )->name() == columnName ) {
      pColumn = *iColumn;
      break;
    }

  if ( pColumn == 0 )
    throw coral::InvalidColumnNameException( m_context );

  return *pColumn;
}


void
coral::TableDescription::setNotNullConstraint( const std::string& columnName,
                                               bool isNotNull )
{
  for ( std::vector< coral::Column* >::iterator iColumn = m_columns.begin();
        iColumn != m_columns.end(); ++iColumn )
    if ( ( *iColumn )->name() == columnName ) {
      (*iColumn)->setNotNull( isNotNull );
      return;
    }
  throw coral::InvalidColumnNameException( m_context, "ITableSchemaEditor::setNotNullConstraint" );
}


void
coral::TableDescription::dropColumn( const std::string& name )
{
  // First of all check that the column is not involved in any multicolumn index or constraint
  for ( std::vector< coral::UniqueConstraint* >::const_iterator iConstraint = m_uniqueConstraints.begin();
        iConstraint != m_uniqueConstraints.end(); ++iConstraint ) {
    if ( (*iConstraint)->columnNames().size() == 1 ) continue;
    for ( std::vector<std::string>::const_iterator iColumn = (*iConstraint)->columnNames().begin();
          iColumn != (*iConstraint)->columnNames().end(); ++iColumn ) {
      if ( *iColumn == name ) {
        throw coral::SchemaException( m_context,
                                      "Column \"" + name + "\" is involved in a multicolumn unique constraint",
                                      "ITableSchemaEditor::dropColumn" );
      }
    }
  }

  for ( std::vector< coral::Index* >::const_iterator iIndex = m_indices.begin();
        iIndex != m_indices.end(); ++iIndex ) {
    if ( (*iIndex)->columnNames().size() == 1 ) continue;
    for ( std::vector<std::string>::const_iterator iColumn = (*iIndex)->columnNames().begin();
          iColumn != (*iIndex)->columnNames().end(); ++iColumn ) {
      if ( *iColumn == name ) {
        throw coral::SchemaException( m_context,
                                      "Column \"" + name + "\" is involved in a multicolumn index",
                                      "ITableSchemaEditor::dropColumn" );
      }
    }
  }

  for ( std::vector< coral::ForeignKey* >::const_iterator iForeignKey = m_foreignKeys.begin();
        iForeignKey != m_foreignKeys.end(); ++iForeignKey ) {
    if ( (*iForeignKey)->columnNames().size() == 1 ) continue;
    for ( std::vector<std::string>::const_iterator iColumn = (*iForeignKey)->columnNames().begin();
          iColumn != (*iForeignKey)->columnNames().end(); ++iColumn ) {
      if ( *iColumn == name ) {
        throw coral::SchemaException( m_context,
                                      "Column \"" + name + "\" is involved in a multicolumn foreign key",
                                      "ITableSchemaEditor::dropColumn" );
      }
    }
  }

  if ( m_primaryKey && m_primaryKey->columnNames().size() > 1 ) {
    for ( std::vector<std::string>::const_iterator iColumn = m_primaryKey->columnNames().begin();
          iColumn != m_primaryKey->columnNames().end(); ++iColumn ) {
      if ( *iColumn == name ) {
        throw coral::SchemaException( m_context,
                                      "Column \"" + name + "\" is involved in a multicolumn primary key",
                                      "ITableSchemaEditor::dropColumn" );
      }
    }
  }


  // Reconstruct the column list
  std::vector< coral::Column* > newColumns;
  int i = 0;
  bool columnFound = false;
  for ( std::vector< coral::Column* >::iterator iColumn = m_columns.begin();
        iColumn != m_columns.end(); ++iColumn ) {
    if ( ( *iColumn )->name() == name ) {
      delete *iColumn;
      columnFound = true;
    }
    else {
      (*iColumn)->setIndexInTable( i );
      ++i;
      newColumns.push_back( *iColumn );
    }
  }

  if ( ! columnFound )
    throw coral::InvalidColumnNameException( m_context, "ITableSchemaEditor::dropColumn" );
  m_columns = newColumns;


  // Drop all the relevant unique constraints, indices, and foreign keys.
  for ( std::vector< coral::UniqueConstraint* >::iterator iConstraint = m_uniqueConstraints.begin();
        iConstraint != m_uniqueConstraints.end(); ++iConstraint ) {
    if ( (*iConstraint)->columnNames().size() == 1 &&
         (*iConstraint)->columnNames()[0] == name ) {
      delete *iConstraint;
      m_uniqueConstraints.erase( iConstraint );
      break;
    }
  }

  for ( std::vector< coral::Index* >::iterator iIndex = m_indices.begin();
        iIndex != m_indices.end(); ++iIndex ) {
    if ( (*iIndex)->columnNames().size() == 1 &&
         (*iIndex)->columnNames()[0] == name ) {
      delete *iIndex;
      m_indices.erase( iIndex );
      break;
    }
  }

  for ( std::vector< coral::ForeignKey* >::iterator iForeignKey = m_foreignKeys.begin();
        iForeignKey != m_foreignKeys.end(); ++iForeignKey ) {
    if ( (*iForeignKey)->columnNames().size() == 1 &&
         (*iForeignKey)->columnNames()[0] == name ) {
      delete *iForeignKey;
      m_foreignKeys.erase( iForeignKey );
      break;
    }
  }

  if ( m_primaryKey &&
       m_primaryKey->columnNames().size() == 1
       && m_primaryKey->columnNames()[0] == name ) {
    delete m_primaryKey;
    m_primaryKey = 0;
  }
}


void
coral::TableDescription::renameColumn( const std::string& originalName,
                                       const std::string& newName )
{
  for ( std::vector< coral::Column* >::iterator iColumn = m_columns.begin();
        iColumn != m_columns.end(); ++iColumn ) {
    if ( ( *iColumn )->name() == originalName ) {
      for ( std::vector< coral::Column* >::iterator iColumnR = m_columns.begin();
            iColumnR != m_columns.end(); ++iColumnR ) {
        if ( ( *iColumnR )->name() == newName )
          throw coral::InvalidColumnNameException( m_context, "ITableSchemaEditor::renameColumn" );
      }
      (*iColumn)->setName( newName );
      return;
    }
  }
  throw coral::InvalidColumnNameException( m_context, "ITableSchemaEditor::renameColumn" );

  // Update the name in the constraints and indices
  for ( std::vector< coral::UniqueConstraint* >::iterator iConstraint = m_uniqueConstraints.begin();
        iConstraint != m_uniqueConstraints.end(); ++iConstraint ) {
    for ( std::vector<std::string>::iterator iColumn = (*iConstraint)->columnNames().begin();
          iColumn != (*iConstraint)->columnNames().end(); ++iColumn ) {
      if ( *iColumn == originalName ) {
        *iColumn = newName;
        break;
      }
    }
  }

  for ( std::vector< coral::Index* >::iterator iIndex = m_indices.begin();
        iIndex != m_indices.end(); ++iIndex ) {
    if ( (*iIndex)->columnNames().size() == 1 ) continue;
    for ( std::vector<std::string>::iterator iColumn = (*iIndex)->columnNames().begin();
          iColumn != (*iIndex)->columnNames().end(); ++iColumn ) {
      if ( *iColumn == originalName ) {
        *iColumn = newName;
        break;
      }
    }
  }

  for ( std::vector< coral::ForeignKey* >::iterator iForeignKey = m_foreignKeys.begin();
        iForeignKey != m_foreignKeys.end(); ++iForeignKey ) {
    if ( (*iForeignKey)->columnNames().size() == 1 ) continue;
    for ( std::vector<std::string>::iterator iColumn = (*iForeignKey)->columnNames().begin();
          iColumn != (*iForeignKey)->columnNames().end(); ++iColumn ) {
      if ( *iColumn == originalName ) {
        *iColumn = newName;
        break;
      }
    }
  }

  if ( m_primaryKey && m_primaryKey->columnNames().size() > 1 ) {
    for ( std::vector<std::string>::iterator iColumn = m_primaryKey->columnNames().begin();
          iColumn != m_primaryKey->columnNames().end(); ++iColumn ) {
      if ( *iColumn == originalName ) {
        *iColumn = newName;
        break;
      }
    }
  }

}


void
coral::TableDescription::changeColumnType( const std::string& columnName,
                                           const std::string& typeName,
                                           int size,
                                           bool fixedSize )
{
  for ( std::vector< coral::Column* >::iterator iColumn = m_columns.begin();
        iColumn != m_columns.end(); ++iColumn )
    if ( ( *iColumn )->name() == columnName ) {
      (*iColumn)->setType( typeName );
      (*iColumn)->setSize( size );
      (*iColumn)->setSizeFixed( fixedSize );
      return;
    }
  throw coral::InvalidColumnNameException( m_context, "ITableSchemaEditor::changeColumnType" );
}


void
coral::TableDescription::setUniqueConstraint( const std::string& columnName,
                                              std::string name,
                                              bool isUnique,
                                              std::string tableSpaceName )
{
  coral::Column* pColumn = 0;
  for ( std::vector< coral::Column* >::iterator iColumn = m_columns.begin();
        iColumn != m_columns.end(); ++iColumn )
    if ( ( *iColumn )->name() == columnName ) {
      pColumn = *iColumn;
      break;
    }

  if ( pColumn == 0 )
    throw coral::InvalidColumnNameException( m_context, "ITableSchemaEditor::setUniqueConstraint" );

  // Check whether a unique constraint already exists for this single column
  if ( isUnique ) {
    for ( std::vector< coral::Column* >::iterator iColumn = m_columns.begin();
          iColumn != m_columns.end(); ++iColumn )
      if ( ( *iColumn )->name() == columnName ) {
        if ( ( *iColumn )->isUnique() )
          throw coral::UniqueConstraintAlreadyExistingException( m_context );
        break;
      }
    if ( m_primaryKey ) {
      if ( m_primaryKey->columnNames().size() == 1 && m_primaryKey->columnNames()[0] == columnName )
        throw coral::UniqueConstraintAlreadyExistingException( m_context );
    }
  }
  else {
    for ( std::vector< coral::UniqueConstraint* >::iterator iConstraint = m_uniqueConstraints.begin();
          iConstraint != m_uniqueConstraints.end(); ++iConstraint ) {
      coral::UniqueConstraint& constraint = **iConstraint;
      if ( constraint.columnNames().size() == 1 && constraint.columnNames().front() == columnName ) {
        delete *iConstraint;
        m_uniqueConstraints.erase( iConstraint );
        pColumn->setUnique( isUnique );
        return;
      }
    }
  }

  if ( isUnique ) {
    m_uniqueConstraints.push_back( new coral::UniqueConstraint( name,
                                                                std::vector<std::string>(1,columnName),
                                                                tableSpaceName ) );
    pColumn->setUnique( isUnique );
  }
  else
    throw coral::InvalidUniqueConstraintIdentifierException( m_context, "ITableSchemaEditor::setUniqueConstraint" );
}


void
coral::TableDescription::setUniqueConstraint( const std::vector<std::string>& columnNames,
                                              std::string name,
                                              bool isUnique,
                                              std::string tableSpaceName )
{
  // Check if the constraint is on a single column.
  if ( columnNames.size() == 1 ) {
    this->setUniqueConstraint( columnNames[0],
                               name,
                               isUnique,
                               tableSpaceName );
    return;
  }

  // Check whether a unique constraint already exists for this single column
  for ( std::vector< coral::UniqueConstraint* >::iterator iConstraint = m_uniqueConstraints.begin();
        iConstraint != m_uniqueConstraints.end(); ++iConstraint ) {
    coral::UniqueConstraint& constraint = **iConstraint;

    if ( constraint.columnNames().size() == columnNames.size() && std::equal( constraint.columnNames().begin(),
                                                                              constraint.columnNames().end(),
                                                                              columnNames.begin() ) ) {
      if ( isUnique ) {
        throw coral::UniqueConstraintAlreadyExistingException( m_context );
      }
      else {
        delete *iConstraint;
        m_uniqueConstraints.erase( iConstraint );
        return;
      }
    }
  }

  if ( isUnique ) {
    // Check first if a unique index exists for the same columns
    for ( std::vector< coral::Index* >::const_iterator iIndex = m_indices.begin();
          iIndex != m_indices.end(); ++iIndex ) {
      const coral::Index& index = **iIndex;
      if ( ! ( index.isUnique() ) ) continue;
      if ( index.columnNames().size() == columnNames.size() && std::equal( index.columnNames().begin(),
                                                                           index.columnNames().end(),
                                                                           columnNames.begin() ) )
        throw coral::UniqueConstraintAlreadyExistingException( m_context );
    }

    if ( m_primaryKey ) {
      if ( m_primaryKey->columnNames().size() == columnNames.size() && std::equal( m_primaryKey->columnNames().begin(),
                                                                                   m_primaryKey->columnNames().end(),
                                                                                   columnNames.begin() ) )
        throw coral::UniqueConstraintAlreadyExistingException( m_context );
    }

    m_uniqueConstraints.push_back( new coral::UniqueConstraint( name,
                                                                columnNames,
                                                                tableSpaceName ) );
  }
  else
    throw coral::InvalidUniqueConstraintIdentifierException( m_context, "ITableSchemaEditor::setUniqueConstraint" );
}


int
coral::TableDescription::numberOfUniqueConstraints() const
{
  return static_cast<int>( m_uniqueConstraints.size() );
}


const coral::IUniqueConstraint&
coral::TableDescription::uniqueConstraint( int uniqueConstraintIdentifier ) const
{
  if ( uniqueConstraintIdentifier < 0 || uniqueConstraintIdentifier >= static_cast<int>( m_uniqueConstraints.size() ) )
    throw coral::InvalidUniqueConstraintIdentifierException( m_context );
  return *( m_uniqueConstraints[uniqueConstraintIdentifier] );
}


bool
coral::TableDescription::hasPrimaryKey() const
{
  return ( m_primaryKey != 0 );
}


const coral::IPrimaryKey&
coral::TableDescription::primaryKey() const
{
  if ( ! m_primaryKey )
    throw coral::NoPrimaryKeyException( m_context );
  return *m_primaryKey;
}


void
coral::TableDescription::dropPrimaryKey()
{
  if ( ! m_primaryKey )
    throw coral::NoPrimaryKeyException( m_context, "ITableSchemaEditor::dropPrimaryKey" );

  if ( m_primaryKey->columnNames().size() == 1 ) {
    const std::string& columnName = m_primaryKey->columnNames()[0];
    for ( std::vector< coral::Column* >::iterator iColumn = m_columns.begin();
          iColumn != m_columns.end(); ++iColumn )
      if ( ( *iColumn )->name() == columnName ) {
        ( *iColumn )->setUnique( false );
        ( *iColumn )->setNotNull( false );
        break;
      }
  }

  delete m_primaryKey;
  m_primaryKey = 0;
}


void
coral::TableDescription::setPrimaryKey( const std::string& columnName,
                                        std::string tableSpaceName )
{
  this->setPrimaryKey( std::vector<std::string>( 1, columnName ), tableSpaceName );
}


void
coral::TableDescription::setPrimaryKey( const std::vector<std::string>& columnNames,
                                        std::string tableSpaceName )
{
  if ( m_primaryKey )
    throw coral::ExistingPrimaryKeyException( m_context );

  // Check first if a unique index exists for the same columns
  for ( std::vector< coral::Index* >::const_iterator iIndex = m_indices.begin();
        iIndex != m_indices.end(); ++iIndex ) {
    const coral::Index& index = **iIndex;
    if ( ! ( index.isUnique() ) ) continue;
    if ( index.columnNames().size() == columnNames.size() && std::equal( index.columnNames().begin(),
                                                                         index.columnNames().end(),
                                                                         columnNames.begin() ) )
      throw coral::UniqueConstraintAlreadyExistingException( m_context );
  }

  for ( std::vector< coral::UniqueConstraint* >::const_iterator iConstraint = m_uniqueConstraints.begin();
        iConstraint != m_uniqueConstraints.end(); ++iConstraint ) {
    const coral::UniqueConstraint& constraint = **iConstraint;
    if ( constraint.columnNames().size() == columnNames.size() && std::equal( constraint.columnNames().begin(),
                                                                              constraint.columnNames().end(),
                                                                              columnNames.begin() ) )
      throw coral::UniqueConstraintAlreadyExistingException( m_context );
  }

  if ( columnNames.size() == 1 ) {
    const std::string& columnName = columnNames[0];
    for ( std::vector< coral::Column* >::iterator iColumn = m_columns.begin();
          iColumn != m_columns.end(); ++iColumn )
      if ( ( *iColumn )->name() == columnName ) {
        ( *iColumn )->setUnique( true );
        ( *iColumn )->setNotNull( true );
        break;
      }
  }

  m_primaryKey = new coral::PrimaryKey( columnNames, tableSpaceName );
}


void
coral::TableDescription::createIndex( const std::string& indexName,
                                      const std::string& columnName,
                                      bool isUnique,
                                      std::string tableSpaceName )
{
  this->createIndex( indexName, std::vector<std::string>( 1, columnName ),
                     isUnique, tableSpaceName );
}


int
coral::TableDescription::numberOfIndices() const
{
  return static_cast<int>( m_indices.size() );
}


const coral::IIndex&
coral::TableDescription::index( int indexId ) const
{
  if ( indexId < 0 || indexId >= static_cast<int>( m_indices.size() ) )
    throw coral::InvalidIndexIdentifierException( m_context );
  return *( m_indices[indexId] );
}


void
coral::TableDescription::createIndex( const std::string& name,
                                      const std::vector<std::string>& columnNames,
                                      bool isUnique,
                                      std::string tableSpaceName )
{
  // Check first whether an index with this name already exists
  for ( std::vector<coral::Index*>::const_iterator iIndex = m_indices.begin();
        iIndex != m_indices.end(); ++iIndex ) {
    if ( (*iIndex)->name() == name )
      throw coral::InvalidIndexIdentifierException( m_context, "ITableSchemaEditor::createIndex" );
  }
  // Check whether the relevant columns exist.
  for ( std::vector<std::string>::const_iterator iColumnName = columnNames.begin();
        iColumnName != columnNames.end(); ++iColumnName ) {
    bool columnFound = false;
    for ( std::vector< coral::Column* >::iterator iColumn = m_columns.begin();
          iColumn != m_columns.end(); ++iColumn ) {
      if ( ( *iColumn )->name() == *iColumnName ) {
        columnFound = true;
        break;
      }
    }
    if ( ! columnFound )
      throw coral::InvalidColumnNameException( m_context, "ITableSchemaEditor::createIndex" );
  }

  // In case of a unique index, check if a unique constraint already exists
  if ( isUnique ) {
    for ( std::vector< coral::UniqueConstraint* >::const_iterator iConstraint = m_uniqueConstraints.begin();
          iConstraint != m_uniqueConstraints.end(); ++iConstraint ) {
      const coral::UniqueConstraint& constraint = **iConstraint;
      if ( constraint.columnNames().size() == columnNames.size() && std::equal( constraint.columnNames().begin(),
                                                                                constraint.columnNames().end(),
                                                                                columnNames.begin() ) )
        throw coral::UniqueConstraintAlreadyExistingException( m_context );
    }

    // Check if a primary key is defined for the same columns
    if ( m_primaryKey ) {
      if ( m_primaryKey->columnNames().size() == columnNames.size() && std::equal( m_primaryKey->columnNames().begin(),
                                                                                   m_primaryKey->columnNames().end(),
                                                                                   columnNames.begin() ) )
        throw coral::UniqueConstraintAlreadyExistingException( m_context );
    }
  }

  // In case of a unique index set the column a unique
  if ( isUnique && columnNames.size() == 1 ) {
    const std::string& columnName = columnNames[0];
    for ( std::vector< coral::Column* >::iterator iColumn = m_columns.begin();
          iColumn != m_columns.end(); ++iColumn )
      if ( ( *iColumn )->name() == columnName ) {
        ( *iColumn )->setUnique( true );
        break;
      }
  }

  m_indices.push_back( new coral::Index( name, columnNames, isUnique, tableSpaceName ) );
}


void
coral::TableDescription::dropIndex( const std::string& indexName )
{
  for ( std::vector< coral::Index* >::iterator iIndex = m_indices.begin();
        iIndex != m_indices.end(); ++iIndex ) {
    if ( (*iIndex)->name() == indexName ) {
      if ( ( *iIndex )->isUnique() && ( *iIndex )->columnNames().size() == 1 ) {
        const std::string& columnName = ( *iIndex )->columnNames()[0];
        for ( std::vector< coral::Column* >::iterator iColumn = m_columns.begin();
              iColumn != m_columns.end(); ++iColumn )
          if ( ( *iColumn )->name() == columnName ) {
            ( *iColumn )->setUnique( false );
            break;
          }
      }
      delete *iIndex;
      m_indices.erase( iIndex );
      return;
    }
  }
  throw coral::InvalidIndexIdentifierException( m_context, "ITableSchemaEditor::dropIndex" );
}

void
coral::TableDescription::createForeignKey( const std::string& name,
                                           const std::string& columnName,
                                           const std::string& referencedTableName,
                                           const std::string& referencedColumnName
#ifdef CORAL240DC
                                           , bool onDeleteCascade
#endif
                                           )
{
  this->createForeignKey( name, std::vector<std::string>( 1, columnName ),
                          referencedTableName, std::vector<std::string>( 1, referencedColumnName )
#ifdef CORAL240DC
                          , onDeleteCascade
#endif
                          );
}

int
coral::TableDescription::numberOfForeignKeys() const
{
  return static_cast<int>( m_foreignKeys.size() );
}


const coral::IForeignKey&
coral::TableDescription::foreignKey( int foreignKeyIdentifier ) const
{
  if ( foreignKeyIdentifier < 0 || foreignKeyIdentifier >= static_cast<int>( m_foreignKeys.size() ) )
    throw coral::InvalidForeignKeyIdentifierException( m_context );
  return *( m_foreignKeys[foreignKeyIdentifier] );
}


void
coral::TableDescription::dropForeignKey( const std::string& name )
{
  for ( std::vector< coral::ForeignKey* >::iterator iForeignKey = m_foreignKeys.begin();
        iForeignKey != m_foreignKeys.end(); ++iForeignKey ) {
    if ( (*iForeignKey)->name() == name ) {
      delete *iForeignKey;
      m_foreignKeys.erase( iForeignKey );
      return;
    }
  }
  throw coral::InvalidForeignKeyIdentifierException( m_context, "ITableSchemaEditor::dropForeignKey" );
}


void
coral::TableDescription::createForeignKey( const std::string& name,
                                           const std::vector<std::string>& columnNames,
                                           const std::string& referencedTableName,
                                           const std::vector<std::string>& referencedColumnNames
#ifdef CORAL240DC
                                           , bool onDeleteCascade
#endif
                                           )
{
  // Check first whether an foreign key with this name already exists
  for ( std::vector<coral::ForeignKey*>::const_iterator iForeignKey = m_foreignKeys.begin();
        iForeignKey != m_foreignKeys.end(); ++iForeignKey ) {
    if ( (*iForeignKey)->name() == name )
      throw coral::InvalidForeignKeyIdentifierException( m_context, "ITableSchemaEditor::createForeignKey" );
  }

  // Check whether the relevant columns exist.
  for ( std::vector<std::string>::const_iterator iColumnName = columnNames.begin();
        iColumnName != columnNames.end(); ++iColumnName ) {
    bool columnFound = false;
    for ( std::vector< coral::Column* >::iterator iColumn = m_columns.begin();
          iColumn != m_columns.end(); ++iColumn ) {
      if ( ( *iColumn )->name() == *iColumnName ) {
        columnFound = true;
        break;
      }
    }
    if ( ! columnFound )
      throw coral::InvalidColumnNameException( m_context, "ITableSchemaEditor::createForeignKey" );
  }

  m_foreignKeys.push_back( new coral::ForeignKey( name, columnNames,
                                                  referencedTableName, referencedColumnNames
#ifdef CORAL240DC
                                                  , onDeleteCascade
#endif
                                                  ) );
}
