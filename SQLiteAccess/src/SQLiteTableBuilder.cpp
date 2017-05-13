#include <iostream>
#include "SQLiteTableBuilder.h"
#include "SQLiteStatement.h"
#include "SessionProperties.h"
#include "DomainProperties.h"

#include "RelationalAccess/ITableDescription.h"
#include "RelationalAccess/IColumn.h"
#include "RelationalAccess/IPrimaryKey.h"
#include "RelationalAccess/IForeignKey.h"
#include "RelationalAccess/IUniqueConstraint.h"
#include "RelationalAccess/ITypeConverter.h"

#include "CoralBase/AttributeSpecification.h"
#include "CoralBase/MessageStream.h"
#include "CoralBase/VersionInfo.h" // Relevant for #ifdef CORAL240xx

#include "CoralKernel/Service.h"

#include <sstream>
#include <map>
#include <vector>

coral::SQLiteAccess::SQLiteTableBuilder::SQLiteTableBuilder( const coral::ITableDescription& description, boost::shared_ptr<const SessionProperties> properties, bool temporary) : m_description(description), m_statement( "" ), m_temporary(temporary)
{
  std::string name=description.name();
  if(m_temporary) {
    name+="temp";
  }
  std::ostringstream os;
  if( !m_temporary ) {
    os<< "CREATE TABLE \""+name+"\" ( " ;
  }else{
    os<< "CREATE TEMPORARY TABLE \""+name+"\" ( " ;
  }
  int numberOfColumns = description.numberOfColumns();
  for ( int iColumn = 0; iColumn < numberOfColumns; ++iColumn ) {
    if ( iColumn > 0 ) os << ",";
    // The column name
    const coral::IColumn& column = description.columnDescription( iColumn );
    os << "\""<<column.name()<<"\" ";
    // The column sql type
    std::string sqltype=properties->typeConverter().sqlTypeForCppType( column.type() );
    os<<sqltype<<" ";
    // The NULL-ness
    if ( column.isNotNull() ) {
      if ( ! ( description.hasPrimaryKey() &&
               description.primaryKey().columnNames().size() == 1 &&
               description.primaryKey().columnNames()[0] == column.name() ) ) {
        os << " NOT NULL";
      }
    }
  } //endofcolumnloop

  // Define the primary keys
  if ( description.hasPrimaryKey() ) {
    const IPrimaryKey& primaryKey = description.primaryKey();
    os<<", PRIMARY KEY(";
    const std::vector< std::string >& columnsForPrimaryKey=primaryKey.columnNames();
    for ( size_t i = 0; i < columnsForPrimaryKey.size(); ++i ) {
      if ( i > 0 ) os << ",";
      os << "\""<<columnsForPrimaryKey[i]<<"\"";
    }
    os << ")";
  }

  // Add the unique constraints
  for ( int i = 0; i < description.numberOfUniqueConstraints(); ++i ) {
    const coral::IUniqueConstraint& constraint = description.uniqueConstraint( i );
    os << ", UNIQUE ( ";
    const std::vector< std::string >& columns = constraint.columnNames();
    for ( size_t ii = 0; ii < columns.size(); ++ii ) {
      if ( ii > 0 ) os << ",";
      os << "\""<<columns[ii]<<"\"";
    }
    os << " )";
  }

  // Define the foreign keys
  for ( int i = 0; i < description.numberOfForeignKeys(); ++i )
  {
    os << ", FOREIGN KEY(";
    const coral::IForeignKey& foreignKey = description.foreignKey(i);
    const std::vector< std::string >& fromcolumns = foreignKey.columnNames();
    for(std::vector<std::string>::const_iterator ifcol=fromcolumns.begin();
        ifcol!=fromcolumns.end(); ++ifcol)
    {
      if( ifcol!=fromcolumns.begin() )
      {
        os<<",";
      }
      os<< "\""<<*ifcol<<"\"";
    }
    //os<< ") REFERENCES \""<< foreignKey.referencedTableName()<<"\"(";
    os<< ") REFERENCES "<< foreignKey.referencedTableName()<<"(";
    const std::vector<std::string>& tocolumns=foreignKey.referencedColumnNames();
    for(std::vector<std::string>::const_iterator itcol=tocolumns.begin();
        itcol!=tocolumns.end(); ++itcol)
    {
      if( itcol!=tocolumns.begin() )
      {
        os<< ",";
      }
      os<< "\""<<*itcol<<"\"";
    }
    os<< ")";
#ifdef CORAL240DC
    // Add feature for delete cascade
    if( foreignKey.onDeleteCascade() )
    {
      os << " ON DELETE CASCADE";
    }
#endif
  }
  os << " )";
  // Define the indices
  m_statement = os.str();
#ifdef _DEBUG
  //  coral::MessageStream log( properties->domainProperties().service(),
  //                           properties->domainProperties().service()->name(),
  //                           coral::Nil );
  coral::MessageStream log( properties->domainProperties().service()->name() );
  log<<coral::Debug<<"SQLiteTableBuilder::SQLiteTableBuilder: "<<m_statement<<coral::MessageStream::endmsg;
#endif
}

std::string
coral::SQLiteAccess::SQLiteTableBuilder::tableName() const {

  std::string name=m_description.name();
  if(m_temporary) {
    name+="temp";
  }
  return name;
}

std::string
coral::SQLiteAccess::SQLiteTableBuilder::statement() const
{

  return m_statement;
}

coral::SQLiteAccess::SQLiteTableBuilder::~SQLiteTableBuilder()
{

}
