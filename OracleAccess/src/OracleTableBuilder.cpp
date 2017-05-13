#include <map>
#include <sstream>
#include <vector>

#include "CoralBase/AttributeSpecification.h"
#include "CoralBase/Blob.h"
#include "CoralBase/TimeStamp.h"
#include "CoralBase/MessageStream.h"
#include "CoralBase/VersionInfo.h" // Relevant for #ifdef CORAL240xx

#include "CoralKernel/Service.h"

#include "RelationalAccess/IColumn.h"
#include "RelationalAccess/IForeignKey.h"
#include "RelationalAccess/IPrimaryKey.h"
#include "RelationalAccess/ITableDescription.h"
#include "RelationalAccess/ITypeConverter.h"
#include "RelationalAccess/IUniqueConstraint.h"

#include "DomainProperties.h"
#include "SessionProperties.h"
#include "OracleTableBuilder.h"

std::string
coral::OracleAccess::OracleTableBuilder::sqlType( boost::shared_ptr<const SessionProperties> properties,
                                                  const std::string& cppTypeName,
                                                  int variableSize,
                                                  bool fixedSize )
{
  std::string suggestedSqlType = properties->sqlTypeForCppType( cppTypeName );
  if ( *( coral::AttributeSpecification::typeIdForName( cppTypeName ) ) == typeid( std::string ) &&  variableSize > 0 ) {
    if ( variableSize <= 4000 ) {
      if ( fixedSize ) {
        if ( variableSize <= 2000 ) {
          std::ostringstream os;
          os << "CHAR(" << variableSize << ")";
          suggestedSqlType = os.str();
        }
        else {
          // Size is too big for char
          coral::MessageStream log( properties->domainServiceName() );
          log << coral::Warning << "A variable has been declared to have a fixed size of "
              << variableSize << ", which is bigger than the allowed 2000. The variable-size SQL type "
              << "VARCHAR(" << variableSize << ") will be used." << coral::MessageStream::endmsg;
          std::ostringstream os;
          os << "VARCHAR(" << variableSize << ")";
          suggestedSqlType = os.str();
        }
      }
      else {
        std::ostringstream os;
        os << "VARCHAR(" << variableSize << ")";
        suggestedSqlType = os.str();
      }
    }
    else {
      coral::MessageStream log( properties->domainServiceName() );
      log << coral::Warning << "A variable has a size larger than 4000. A CLOB type will be used." << coral::MessageStream::endmsg;
      suggestedSqlType = "CLOB";
    }
  }

  if ( *( coral::AttributeSpecification::typeIdForName( cppTypeName ) ) == typeid( coral::TimeStamp ) ) {
    std::ostringstream os;
    os << "TIMESTAMP(" << variableSize << ")";
    suggestedSqlType = os.str();
  }

  return suggestedSqlType;
}



coral::OracleAccess::OracleTableBuilder::OracleTableBuilder( const coral::ITableDescription& description,
                                                             boost::shared_ptr<const SessionProperties> properties,
                                                             const std::string& schemaName ) :
  m_statement( "" )
{
  std::ostringstream os;
  os << "CREATE TABLE " << schemaName << ".\"" << description.name() << "\" (";

  std::map< std::string, std::vector< std::string > > lobsAndTableSpaces;

  int numberOfColumns = description.numberOfColumns();
  for ( int iColumn = 0; iColumn < numberOfColumns; ++iColumn ) {

    if ( iColumn > 0 ) os << ",";

    // The column name
    const coral::IColumn& column = description.columnDescription( iColumn );
    os << " \"" << column.name() << "\" ";

    // The column type
    std::string suggestedSqlType = coral::OracleAccess::OracleTableBuilder::sqlType( properties,
                                                                                     column.type(),
                                                                                     column.size(),
                                                                                     column.isSizeFixed() );
    os << suggestedSqlType;

    // In case of BLOB types define the table space
    if ( *( coral::AttributeSpecification::typeIdForName( column.type() ) ) == typeid( coral::Blob ) ||
         suggestedSqlType == "CLOB" ) {
      std::string tableSpaceName = column.tableSpaceName();
      if ( tableSpaceName.empty() )
        tableSpaceName = properties->domainProperties().tableSpaceForLobs();
      if ( ! tableSpaceName.empty() )
        lobsAndTableSpaces[tableSpaceName].push_back( column.name() );
    }

    // The NULL-ness
    if ( column.isNotNull() ) {
      if ( ! ( description.hasPrimaryKey() &&
               description.primaryKey().columnNames().size() == 1 &&
               description.primaryKey().columnNames()[0] == column.name() ) )
        os << " NOT NULL";
    }

    // The uniqueness of the column
    if ( column.isUnique() ) {
      if ( ! ( description.hasPrimaryKey() &&
               description.primaryKey().columnNames().size() == 1 &&
               description.primaryKey().columnNames()[0] == column.name() ) ) {
        // The table space for this constraint
        std::string tableSpaceName = "";
        std::string constraintName = "";
        bool foundConstraint = false;
        for ( int i = 0; i < description.numberOfUniqueConstraints(); ++i ) {
          const coral::IUniqueConstraint& constraint = description.uniqueConstraint( i );
          const std::vector<std::string>& columnNames = constraint.columnNames();
          if ( columnNames.size() == 1 && columnNames[0] == column.name() ) {
            tableSpaceName = constraint.tableSpaceName();
            constraintName = constraint.name();
            foundConstraint = true;
            break;
          }
        }
        if ( foundConstraint && constraintName.empty() ) {
          os << " UNIQUE";
          if ( tableSpaceName.empty() ) // If not defined, use the default
            tableSpaceName = properties->domainProperties().tableSpaceForIndices();
          if ( ! tableSpaceName.empty() )
            os << " USING INDEX TABLESPACE \"" << tableSpaceName << "\"";
        }
      }
    }
  }

  // Add the primary key constraint
  if ( description.hasPrimaryKey() ) {
    os << ", CONSTRAINT \"" << description.name() << "_PK\" PRIMARY KEY ( ";
    const std::vector< std::string >& columnsForPrimaryKey = description.primaryKey().columnNames();
    for ( size_t i = 0; i < columnsForPrimaryKey.size(); ++i ) {
      if ( i > 0 ) os << ",";
      os << "\"" << columnsForPrimaryKey[i] << "\"";
    }
    os << " )";
    // The table space for this index
    std::string tableSpaceName = description.primaryKey().tableSpaceName();
    if ( tableSpaceName.empty() ) // If not defined, use the default
      tableSpaceName = properties->domainProperties().tableSpaceForIndices();
    if ( ! tableSpaceName.empty() )
      os << " USING INDEX TABLESPACE \"" << tableSpaceName << "\"";
  }

  // Add the unique constraints
  for ( int i = 0; i < description.numberOfUniqueConstraints(); ++i ) {
    const coral::IUniqueConstraint& constraint = description.uniqueConstraint( i );
    std::string constraintName = constraint.name();
    if ( constraintName.empty() ) {
      if ( constraint.columnNames().size() == 1 ) continue;  // Already treated...
      std::ostringstream osConstraintName;
      osConstraintName << description.name() << "_UC_" << i;
      constraintName = osConstraintName.str();
    }
    os << ", CONSTRAINT \"" << constraintName << "\" UNIQUE ( ";
    const std::vector< std::string >& columns = constraint.columnNames();
    for ( size_t j = 0; j < columns.size(); ++j ) {
      if ( j > 0 ) os << ",";
      os << "\"" << columns[j] << "\"";
    }
    os << " )";
    std::string tableSpaceName = constraint.tableSpaceName();
    if ( tableSpaceName.empty() ) // If not defined, use the default
      tableSpaceName = properties->domainProperties().tableSpaceForIndices();
    if ( ! tableSpaceName.empty() )
      os << " USING INDEX TABLESPACE \"" << tableSpaceName << "\"";
  }

  // Add the foreign key constraints
  for ( int i = 0; i < description.numberOfForeignKeys(); ++i ) {
    const coral::IForeignKey& foreignKey = description.foreignKey( i );
    os << ", CONSTRAINT \"" << foreignKey.name() << "\" FOREIGN KEY ( ";
    const std::vector< std::string >& columns = foreignKey.columnNames();
    for ( size_t j = 0; j < columns.size(); ++j ) {
      if ( j > 0 ) os << ",";
      os << "\"" << columns[j] << "\"";
    }
    os << " ) REFERENCES " << schemaName << ".\"" << foreignKey.referencedTableName() << "\" ( ";
    const std::vector< std::string >& columnsR = foreignKey.referencedColumnNames();
    for ( size_t j = 0; j < columnsR.size(); ++j ) {
      if ( j > 0 ) os << ",";
      os << "\"" << columnsR[j] << "\"";
    }
    os << " )";
#ifdef CORAL240DC
    if( foreignKey.onDeleteCascade() )
    {
      os << " ON DELETE CASCADE";
    }
#endif
  }

  os << " )";

  // Append the tablespace for the LOBS
  for ( std::map< std::string, std::vector< std::string > >::const_iterator iTs = lobsAndTableSpaces.begin();
        iTs != lobsAndTableSpaces.end(); ++iTs ) {
    os << " LOB (";
    const std::vector< std::string >& lobNames = iTs->second;
    for ( size_t i = 0; i < lobNames.size(); ++i ) {
      if ( i > 0 ) os << ",";
      os << " \"" << lobNames[i] << "\"";
    }

    os << " ) STORE AS ( TABLESPACE \"" << iTs->first << "\" )";
  }

  // Define the table space
  std::string tableSpaceName = description.tableSpaceName();

  if ( tableSpaceName.empty() ) { // If not define, use the default
    tableSpaceName = properties->domainProperties().tableSpaceForTables();
  }

  if ( ! tableSpaceName.empty() ) os << " TABLESPACE \"" << tableSpaceName << "\"";

  m_statement = os.str();
}


coral::OracleAccess::OracleTableBuilder::~OracleTableBuilder()
{
}

std::string
coral::OracleAccess::OracleTableBuilder::statement() const
{
  return m_statement;
}
