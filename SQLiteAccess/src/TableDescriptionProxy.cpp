#include <iostream>
#include "TableDescriptionProxy.h"
#include "SessionProperties.h"
#include "DomainProperties.h"
#include "SQLiteStatement.h"
#include "SQLiteTableBuilder.h"
#include "ColumnProxy.h"

#include "RelationalAccess/TableDescription.h"
#include "RelationalAccess/SchemaException.h"
#include "RelationalAccess/ISchema.h"
#include "RelationalAccess/ITypeConverter.h"
#include "RelationalAccess/IPrimaryKey.h"
#include "RelationalAccess/IIndex.h"
#include "RelationalAccess/IForeignKey.h"

#include "CoralBase/AttributeList.h"
#include "CoralBase/Attribute.h"
#include "CoralBase/MessageStream.h"
#include "CoralBase/VersionInfo.h" // Relevant for #ifdef CORAL240xx

#include "CoralKernel/Service.h"

#include <sstream>
//#include <iostream>

// Using empty namespace to avoid name conflicts
namespace {

  static void
  initRandomGenerator()
  {
    // Static variable to ensure only one initialisation
    static bool noinit = true;
    if( noinit )
    {
#ifndef _WIN32
      srand( time(0) );
#else
      srand( (unsigned int)(time(0)) );
#endif
      noinit = false;
    }
  }

  void
  getRandomName( std::string& buffer, int size )
  {
    initRandomGenerator();
    // Add random characters in upper case
    for( int i = 0; i < size; i++ )
    {
      buffer += (char)((rand() % 26) + 65);
    }
  }

}

coral::SQLiteAccess::TableDescriptionProxy::TableDescriptionProxy( boost::shared_ptr<const SessionProperties> properties,const std::string& tableName ) :
  m_properties( properties ),
  m_description( new coral::TableDescription( properties->domainProperties().service()->name() ) ),
  m_columnDescriptionRead( false ),
  m_columns(),
  m_uniqueConstraintsRead( false ),
  m_primaryKeyRead( false ),
  m_indicesRead( false ),
  m_foreignKeysRead( false )
{

  m_description->setName( tableName );
  createFKtableIfNotexists();
}


coral::SQLiteAccess::TableDescriptionProxy::~TableDescriptionProxy()
{

  for ( std::vector< coral::SQLiteAccess::ColumnProxy* >::iterator iColumn = m_columns.begin(); iColumn != m_columns.end(); ++iColumn ) delete *iColumn;
  delete m_description;
}


std::string
coral::SQLiteAccess::TableDescriptionProxy::name() const
{

  return m_description->name();
}


std::string
coral::SQLiteAccess::TableDescriptionProxy::type() const
{

  return m_description->type();
}


void
coral::SQLiteAccess::TableDescriptionProxy::readColumnDescription()
{

  if ( m_columnDescriptionRead ) return;
#ifdef _DEBUG
  //  coral::MessageStream log( m_properties->domainProperties().service(),
  //                           m_properties->domainProperties().service()->name(),
  //                           coral::Nil );
  coral::MessageStream log( m_properties->domainProperties().service()->name() );
  log<<coral::Debug<<"SQLiteAccess::TableDescriptionProxy::readColumnDescription"<<coral::MessageStream::endmsg;
#endif

  ////////////////////////////////////////////////////////////////////
  //result structure
  // 0   1    2    3          4       5
  //cid name type notnull dflt_value  pk
  ////////////////////////////////////////////////////////////////////
  coral::SQLiteAccess::SQLiteStatement stmt(m_properties);
  coral::AttributeList result;
  result.extend("cid",typeid(int));
  result.extend("name",typeid(std::string));
  result.extend("type",typeid(std::string));
  result.extend("notnull",typeid(int));
  result.extend("dflt_value",typeid(int));
  result.extend("pk",typeid(int));
  std::string tabinfo=std::string("PRAGMA table_info(\"")+m_description->name()+std::string("\")");
  if( !stmt.prepare(tabinfo) ) {
    throw coral::SchemaException("sqlite","Could not retrieve the name of a column in a table","TableDescriptionProxy::readColumnDescription" );
  }
  int numberOfColumns=0;
  //std::string columnName;
  //std::string columntype;
  //bool hasIndex=false;
  std::pair<std::string, std::vector<std::string> > pk = std::make_pair( std::string(""),std::vector<std::string>() );
  pk.first = m_description->name(); //tablename
  while( stmt.fetchNext() ) {
    if(!stmt.defineOutput( result ) ) {
      throw coral::SchemaException("sqlite","Could not retrieve the name of a column in a table","TableDescriptionProxy::readColumnDescription" );
    }
    bool isNullable=true;
    const std::string& columnName=result[1].data<std::string>();
    const std::string& columnType=result[2].data<std::string>(); //sql type
    // GG The code 1 is sure, can be retrieved when a notnull constraint is explicitly set. Not sure about 99, from Zhen.
    if( result[3].data<int>()==1 || result[3].data<int>()==99 ) {
      isNullable=false;
    }
    if( result[5].data<int>()==1 ) {
      pk.second.push_back( columnName );
      ///PK is also not null
      //std::cout<<"is pk and is not null"<<std::endl;
      isNullable=false;
    }
    m_description->insertColumn( columnName, m_properties->typeConverter().cppTypeForSqlType(columnType) ); //use default

    if ( !isNullable ) {
      m_description->setNotNullConstraint( columnName );
    }
    ++numberOfColumns;
  }
  //std::string pkidxname=pk.first+"_pkidx";
  if ( ! pk.second.empty() ) {
    m_description->setPrimaryKey( pk.second, pk.first );
    //m_description->createIndex( pkidxname,pk.second,false);
    //m_description->setUniqueConstraint(pk.second,"",false,"");
  }
  m_primaryKeyRead = true;
  for ( int i = 0; i < numberOfColumns; ++i ) {
    coral::SQLiteAccess::ColumnProxy* col= new coral::SQLiteAccess::ColumnProxy( m_description->columnDescription( i ),*this );
    m_columns.push_back( col );
  }
  m_columnDescriptionRead = true;
}

void
coral::SQLiteAccess::TableDescriptionProxy::refreshConstraints() const
{

  if ( m_uniqueConstraintsRead ) return;
#ifdef _DEBUG
  //  coral::MessageStream log( m_properties->domainProperties().service(),
  //                           m_properties->domainProperties().service()->name(),
  //                           coral::Nil );
  coral::MessageStream log( m_properties->domainProperties().service()->name() );
  log<<coral::Debug<<"SQLiteAccess::TableDescriptionProxy::refreshConstraints"<<coral::MessageStream::endmsg;
#endif
  /////////////////////////////////////////////////////////////////////////////
  //"SELECT name FROM sqlite_master WHERE type='index' AND tbl_name=? AND sql IS NULL"
  // Note: refresh only unique constraints here, notnull constraints are refreshed in refreshColumns;
  // Note : PK are excluded
  //////////////////////////////////////////////////////////////////////
  const_cast< coral::SQLiteAccess::TableDescriptionProxy *>( this )->readColumnDescription();
  coral::SQLiteAccess::SQLiteStatement stmt(m_properties);
  std::string idxlist("SELECT name FROM sqlite_master WHERE type='index' AND sql IS NULL AND tbl_name=\"");
  idxlist+=m_description->name()+std::string("\"");
  if(!stmt.prepare(idxlist)) {
    throw coral::SchemaException("sqlite","Error in preparing list constraints","TableDescriptionProxy::refreshConstraints" );
  }
  coral::AttributeList idxlistresult;
  idxlistresult.extend("name",typeid(std::string));
  std::vector<std::string> idxnames;
  while( stmt.fetchNext() ) {
    if(!stmt.defineOutput( idxlistresult )) {
      throw coral::SchemaException("sqlite","Error in defining index info","TableDescriptionProxy::refreshConstraints" );
    }
    idxnames.push_back(idxlistresult["name"].data<std::string>());
  }
  ////////////////////////////////////////////////////////////////////
  //pragma index_info result structure
  // 0      1     2
  //seq    cid   colname
  ////////////////////////////////////////////////////////////////////
  bool hasPK=false;
  if( m_description->hasPrimaryKey() ) {
    hasPK=true;
  }
  std::map< std::string, std::vector<std::string> > uniqueconstraints;
  for(std::vector<std::string>::iterator it=idxnames.begin(); it!=idxnames.end(); ++it) {
    coral::SQLiteAccess::SQLiteStatement stmt2(m_properties);
    std::string idxinfo("PRAGMA index_info(\"");
    idxinfo += *it + std::string("\")");
    if( !stmt2.prepare(idxinfo) ) {
      throw coral::SchemaException("sqlite","Error in preparing index info","TableDescriptionProxy::refreshConstraints" );
    }
    coral::AttributeList idxinforesult;
    idxinforesult.extend("seq",typeid(int));
    idxinforesult.extend("cid",typeid(int));
    idxinforesult.extend("colname",typeid(std::string));
    while( stmt2.fetchNext() ) {
      if( !stmt2.defineOutput( idxinforesult ) ) {
        throw coral::SchemaException("sqlite","Error in defining index info","TableDescriptionProxy::refreshConstraints" );
      }
      std::string colname=idxinforesult["colname"].data<std::string>();
      if(hasPK) {
        //ignore PK index
        const std::vector<std::string>& pks=m_description->primaryKey().columnNames();
        if(std::find(pks.begin(),pks.end(),colname) != pks.end()) continue;
      }
      if( uniqueconstraints.find(*it)!=uniqueconstraints.end() ) {
        uniqueconstraints[*it].push_back(colname);
      }else{
        uniqueconstraints.insert(make_pair(*it,std::vector<std::string>(1,colname)));
      }
    }
  }
  for( std::map< std::string, std::vector< std::string > >::const_iterator it = uniqueconstraints.begin(); it != uniqueconstraints.end(); ++it ) {
    m_description->setUniqueConstraint(it->second, it->first,true,"");
  }
  m_uniqueConstraintsRead = true;
}

void
coral::SQLiteAccess::TableDescriptionProxy::refreshPrimaryKeyInfo() const
{

  if ( m_primaryKeyRead ) return;
#ifdef _DEBUG
  //  coral::MessageStream log( m_properties->domainProperties().service(),
  //                           m_properties->domainProperties().service()->name(),
  //                           coral::Nil );
  coral::MessageStream log( m_properties->domainProperties().service()->name() );
  log<<coral::Debug<<"SQLiteAccess::TableDescriptionProxy::refreshPrimaryKeyInfo"<<coral::MessageStream::endmsg;
#endif
  const_cast< coral::SQLiteAccess::TableDescriptionProxy *>( this )->readColumnDescription();
  m_primaryKeyRead = true;
}

// A simple structure to hold the index information
namespace coral {
  namespace SQLiteAccess {
    class IndexInfo {
    public:
      IndexInfo( bool _unique = false,
                 std::string name = "" ) : unique(_unique),
                                           tableName( name ),
                                           columns()
      {}
      bool unique;
      std::string tableName;
      std::vector<std::string> columns;
    };
  }
}


void
coral::SQLiteAccess::TableDescriptionProxy::refreshIndexInfo() const
{

  if ( m_indicesRead ) return;
#ifdef _DEBUG
  //  coral::MessageStream log( m_properties->domainProperties().service(),
  //                           m_properties->domainProperties().service()->name(),
  //                           coral::Nil );
  coral::MessageStream log( m_properties->domainProperties().service()->name() );
  log<<coral::Debug<<"SQLiteAccess::TableDescriptionProxy::refreshIndexInfo"<<coral::MessageStream::endmsg;
#endif
  /////////////////////////////////////////////////////////////////////////////
  //"SELECT name, sql FROM sqlite_master WHERE type='index' AND tbl_name=? AND sql IS NOT NULL"
  //Note: Index here intend to be explictly defined indices, not including implicit indices create by PK, and unique constraints.
  /////////////////////////////////////////////////////////////////////////////
  const_cast< coral::SQLiteAccess::TableDescriptionProxy *>( this )->readColumnDescription();
  coral::SQLiteAccess::SQLiteStatement stmt(m_properties);
  std::string idxlist("SELECT name,sql FROM sqlite_master WHERE type='index' AND sql IS NOT NULL AND tbl_name=\"");
  idxlist+=m_description->name()+std::string("\"");
  if(!stmt.prepare(idxlist)) {
    throw coral::SchemaException("sqlite","Error in preparing index list","TableDescriptionProxy::refreshIndexInfo" );
  }
  coral::AttributeList idxlistresult;
  idxlistresult.extend("name",typeid(std::string));
  idxlistresult.extend("sql",typeid(std::string));
  bool isUnique=false;
  std::map< std::string, coral::SQLiteAccess::IndexInfo > indices;
  std::vector<std::string> idxnames;
  while( stmt.fetchNext() ) {
    if(!stmt.defineOutput( idxlistresult )) {
      throw coral::SchemaException("sqlite","Error in retrieving index list","TableDescriptionProxy::refreshIndexInfo" );
    }
    std::string idxname=idxlistresult["name"].data<std::string>();
    std::string sql=idxlistresult["sql"].data<std::string>();
    std::map< std::string, IndexInfo >::iterator index = indices.find( idxname );
    if( sql.find(" UNIQUE ") != std::string::npos ) {
      isUnique=true;
    }
    else isUnique=false;  // FIX for bug #54806
    if( index == indices.end() ) {
      indices.insert( std::make_pair(idxname, coral::SQLiteAccess::IndexInfo(isUnique,m_description->name())));
      idxnames.push_back(idxname);
    }
  }
  ////////////////////////////////////////////////////////////////////
  //pragma index_info result structure
  // 0      1     2
  //seq    cid   colname
  ////////////////////////////////////////////////////////////////////
  coral::SQLiteAccess::SQLiteStatement stmt2(m_properties);
  for(std::vector<std::string>::iterator it=idxnames.begin(); it!=idxnames.end(); ++it) {
    std::string idxinfo("PRAGMA index_info(\"");
    idxinfo += *it + std::string("\")");
    if( !stmt2.prepare(idxinfo) ) {
      throw coral::SchemaException("sqlite","Error in preparing index info","TableDescriptionProxy::refreshIndexInfo" );
    }
    coral::AttributeList idxinforesult;
    idxinforesult.extend("seq",typeid(int));
    idxinforesult.extend("cid",typeid(int));
    idxinforesult.extend("colname",typeid(std::string));
    while( stmt2.fetchNext() ) {
      if( !stmt2.defineOutput( idxinforesult ) ) {
        throw coral::SchemaException("sqlite","Error in defining index info","TableDescriptionProxy::refreshIndexInfo" );
      }
      if( indices.find(*it)!=indices.end() ) {
        indices[*it].columns.push_back( idxinforesult["colname"].data<std::string>() );
      }
    }
  }
  for ( std::map< std::string, coral::SQLiteAccess::IndexInfo >::const_iterator index = indices.begin(); index != indices.end(); ++index ) {
    try{
      m_description->createIndex( index->first,
                                  index->second.columns,
                                  index->second.unique);
    }catch(const coral::InvalidIndexIdentifierException&) {
      continue;
    }
  }
  m_indicesRead = true;
}

// A simple structure to hold the foreign key information
namespace coral {
  namespace SQLiteAccess {
    class ForeignKeyInfo {
    public:
      ForeignKeyInfo( std::string _tableName = "" ) : tableName(_tableName),
                                                      columns(),
                                                      refColumns()
      {}
      std::string tableName;
      std::vector<std::string> columns;
      std::vector<std::string> refColumns;
    };
  }
}

void
coral::SQLiteAccess::TableDescriptionProxy::refreshForeignKeyInfo() const {

  if(m_foreignKeysRead) return;
#ifdef _DEBUG
  //  coral::MessageStream log( m_properties->domainProperties().service(),
  //                           m_properties->domainProperties().service()->name(),
  //                           coral::Nil );
  coral::MessageStream log( m_properties->domainProperties().service()->name() );
  log<<coral::Debug<<"SQLiteAccess::TableDescriptionProxy::refreshForeignKeyInfo()"<<coral::MessageStream::endmsg;
#endif
  const_cast< coral::SQLiteAccess::TableDescriptionProxy *>( this )->readColumnDescription();
  ////////////////////////////////////////////////////////////////////
  //PRAGMA foreign_key_list
  //result structure
  // 0  1    2                   3                 4
  //id seq referenced-table columnname(from) referenced-column(to)
  ////////////////////////////////////////////////////////////////////

  coral::SQLiteAccess::SQLiteStatement stmt(m_properties);
  std::string fkinfo=std::string("PRAGMA foreign_key_list(\"")+m_description->name()+std::string("\")");
  coral::AttributeList result;
  result.extend("id",typeid(std::string));
  result.extend("seq",typeid(int));
  result.extend("reftable",typeid(std::string));
  result.extend("fromcol",typeid(std::string));
  result.extend("tocol",typeid(std::string));
  // Read the foreign key information
  std::map< std::string, coral::SQLiteAccess::ForeignKeyInfo > fkeys;
  if(!stmt.prepare(fkinfo) ) {
    throw coral::SchemaException("sqlite","Error in retrieving foreign key info (prepare PRAGMA failed)","TableDescriptionProxy::refreshForeignKeyInfo");
  }
  int counter=0;
  while( stmt.fetchNext() ) {
    if(!stmt.defineOutput(result)) {
      throw coral::SchemaException("sqlite","Error in retrieving foreign key info (defineOutput PRAGMA failed)","TableDescriptionProxy::refreshForeignKeyInfo");
    }
    ++counter;
    const std::string& foreignKeyID = result[0].data<std::string>();
    const std::string& referencedTableName = result[2].data<std::string>();
    const std::string& foreignKeyColumnName = result[3].data<std::string>();
    const std::string& referencedColumnName= result[4].data<std::string>();
    std::map< std::string, ForeignKeyInfo >::iterator fkey = fkeys.find( foreignKeyID );
    if ( fkey == fkeys.end() ) {
      fkey = fkeys.insert( std::make_pair( foreignKeyID, coral::SQLiteAccess::ForeignKeyInfo( referencedTableName ) ) ).first;
    }
    fkey->second.columns.push_back( foreignKeyColumnName );
    fkey->second.refColumns.push_back( referencedColumnName );
  }
  stmt.reset();
  if(counter==0) {
    m_foreignKeysRead = true;
    return;
  }
  std::map< std::string, coral::SQLiteAccess::ForeignKeyInfo > fkeysWithName;
  fkinfo="SELECT id,name FROM coral_sqlite_fk where tablename=\"";
  fkinfo+=m_description->name()+"\"";
  coral::AttributeList result2;
  result2.extend("id",typeid(std::string));
  result2.extend("name",typeid(std::string));
  if(!stmt.prepare(fkinfo) ) {
    throw coral::SchemaException("sqlite","Error in retrieving foreign key info (prepare SELECT failed) from coral_sqlite_fk for table "+m_description->name(),"TableDescriptionProxy::refreshForeignKeyInfo");
  }
  counter=0;
  while( stmt.fetchNext() ) {
    if(!stmt.defineOutput(result2)) {
      throw coral::SchemaException("sqlite","Error in retrieving foreign key info (defineOutput SELECT failed) from coral_sqlite_fk for table "+m_description->name(),"TableDescriptionProxy::refreshForeignKeyInfo");
    }
    ++counter;
    const std::string& foreignKeyID = result2["id"].data<std::string>();
    const std::string& foreignKeyName = result2["name"].data<std::string>();
    std::map< std::string, ForeignKeyInfo >::iterator fkey = fkeys.find( foreignKeyID );
    if( fkey == fkeys.end() ) {
      throw coral::SchemaException("sqlite","Error in retrieving foreign key info (key " + foreignKeyID + "not found) from coral_sqlite_fk for table "+m_description->name(),"TableDescriptionProxy::refreshForeignKeyInfo");
    }
    fkeysWithName.insert(std::make_pair(foreignKeyName,fkeys[foreignKeyID]) );
  }

  if(fkeysWithName.size()!=0) {
    for ( std::map< std::string, coral::SQLiteAccess::ForeignKeyInfo >::const_iterator iKey = fkeysWithName.begin(); iKey != fkeysWithName.end(); ++iKey ) {
      m_description->createForeignKey( iKey->first,
                                       iKey->second.columns,
                                       iKey->second.tableName,
                                       iKey->second.refColumns );
    }
  }else if(fkeys.size()!=0 ) {
    for ( std::map< std::string, coral::SQLiteAccess::ForeignKeyInfo >::const_iterator iKey = fkeys.begin(); iKey != fkeys.end(); ++iKey ) {
      m_description->createForeignKey( iKey->first,
                                       iKey->second.columns,
                                       iKey->second.tableName,
                                       iKey->second.refColumns );
    }
  }
  stmt.reset();
  m_foreignKeysRead = true;
}

std::string
coral::SQLiteAccess::TableDescriptionProxy::tableSpaceName() const
{

  return "";
}


int
coral::SQLiteAccess::TableDescriptionProxy::numberOfColumns() const
{

  if ( ! m_columnDescriptionRead )
    const_cast<coral::SQLiteAccess::TableDescriptionProxy*>( this )->readColumnDescription();
  return m_description->numberOfColumns();
}

const coral::IColumn&
coral::SQLiteAccess::TableDescriptionProxy::columnDescription( int columnIndex ) const
{

  //if ( ! m_columnDescriptionRead )
  const_cast<coral::SQLiteAccess::TableDescriptionProxy*>( this )->readColumnDescription();
  if ( columnIndex < 0 || columnIndex >= static_cast<int>( m_columns.size() ) )
    throw coral::InvalidColumnIndexException( m_properties->domainProperties().service()->name() );
  return *( m_columns[ columnIndex ] );
}


const coral::IColumn&
coral::SQLiteAccess::TableDescriptionProxy::columnDescription( const std::string& columnName ) const
{

  //if ( ! m_columnDescriptionRead )
  const_cast<coral::SQLiteAccess::TableDescriptionProxy*>( this )->readColumnDescription();

  const coral::IColumn* pColumn = 0;
  for ( std::vector< coral::SQLiteAccess::ColumnProxy* >::const_iterator iColumn = m_columns.begin(); iColumn != m_columns.end(); ++iColumn )
    if ( ( *iColumn )->name() == columnName ) {
      pColumn = *iColumn;
      break;
    }

  if ( pColumn == 0 )
    throw coral::InvalidColumnNameException( m_properties->domainProperties().service()->name() );
  return *pColumn;
}


bool
coral::SQLiteAccess::TableDescriptionProxy::hasPrimaryKey() const
{

  //if ( ! m_primaryKeyRead )
  const_cast<coral::SQLiteAccess::TableDescriptionProxy*>( this )->refreshPrimaryKeyInfo();
  return m_description->hasPrimaryKey();
}


const coral::IPrimaryKey&
coral::SQLiteAccess::TableDescriptionProxy::primaryKey() const
{

  //if ( ! m_primaryKeyRead )
  const_cast<coral::SQLiteAccess::TableDescriptionProxy*>( this )->refreshPrimaryKeyInfo();
  return m_description->primaryKey();
}


int
coral::SQLiteAccess::TableDescriptionProxy::numberOfIndices() const
{

  //if ( ! m_indicesRead )
  const_cast<coral::SQLiteAccess::TableDescriptionProxy*>( this )->refreshIndexInfo();
  return m_description->numberOfIndices();
}

const coral::IIndex&
coral::SQLiteAccess::TableDescriptionProxy::index( int indexId ) const
{

  //if ( ! m_indicesRead )
  const_cast<coral::SQLiteAccess::TableDescriptionProxy*>( this )->refreshIndexInfo();
  return m_description->index( indexId );
}

int
coral::SQLiteAccess::TableDescriptionProxy::numberOfForeignKeys() const
{

  //if ( ! m_foreignKeysRead )
  const_cast<coral::SQLiteAccess::TableDescriptionProxy*>( this )->refreshForeignKeyInfo();
  return m_description->numberOfForeignKeys();
}

const coral::IForeignKey&
coral::SQLiteAccess::TableDescriptionProxy::foreignKey( int foreignKeyIdentifier ) const
{
  //if ( ! m_foreignKeysRead )
  const_cast<coral::SQLiteAccess::TableDescriptionProxy*>( this )->refreshForeignKeyInfo();
  return m_description->foreignKey( foreignKeyIdentifier );
}

int
coral::SQLiteAccess::TableDescriptionProxy::numberOfUniqueConstraints() const
{

  //if ( ! m_columnDescriptionRead )
  //const_cast<coral::SQLiteAccess::TableDescriptionProxy*>( this )->readColumnDescription();
  const_cast<coral::SQLiteAccess::TableDescriptionProxy*>( this )->refreshConstraints();
  return m_description->numberOfUniqueConstraints();
}

const coral::IUniqueConstraint&
coral::SQLiteAccess::TableDescriptionProxy::uniqueConstraint( int uniqueConstraintIdentifier ) const
{

  // if ( ! m_columnDescriptionRead )
  //  const_cast<coral::SQLiteAccess::TableDescriptionProxy*>( this )->readColumnDescription();
  const_cast<coral::SQLiteAccess::TableDescriptionProxy*>( this )->refreshConstraints();
  return m_description->uniqueConstraint( uniqueConstraintIdentifier );
}

void
coral::SQLiteAccess::TableDescriptionProxy::insertColumn( const std::string& name,
                                                          const std::string& type,
                                                          int size,
                                                          bool fixedSize,
                                                          std::string tableSpaceName )
{

#ifdef _DEBUG
  //  coral::MessageStream log( m_properties->domainProperties().service(),
  //                           m_properties->domainProperties().service()->name(),
  //                           coral::Nil );
  coral::MessageStream log( m_properties->domainProperties().service()->name() );
  log<<coral::Debug<<"SQLiteAccess::TableDescriptionProxy::insertColumn "<<name<<coral::MessageStream::endmsg;
#endif
  //if ( ! m_columnDescriptionRead )
  const_cast<coral::SQLiteAccess::TableDescriptionProxy*>( this )->readColumnDescription();
  m_description->insertColumn( name,type,size,fixedSize,tableSpaceName );
  std::ostringstream os;
  os << "ALTER TABLE \"" << m_description->name() << "\" ADD \"" << name << "\" " << m_properties->typeConverter().sqlTypeForCppType(type);
  //std::cout<<os.str()<<std::endl;
  coral::SQLiteAccess::SQLiteStatement stmt( m_properties );
  if ( !stmt.prepare( os.str() ) || ! stmt.execute() ) {
    m_description->dropColumn( name );
    throw coral::SchemaException( m_properties->domainProperties().service()->name(),"Could not add a new column to a table","ITableSchemaEditor::insertColumn" );
  }
  m_columns.push_back( new coral::SQLiteAccess::ColumnProxy( m_description->columnDescription( name  ),*this ) );
}

void
coral::SQLiteAccess::TableDescriptionProxy::dropColumn( const std::string& name ){

#ifdef _DEBUG
  //  coral::MessageStream log( m_properties->domainProperties().service(),
  //                           m_properties->domainProperties().service()->name(),
  //                           coral::Nil );
  coral::MessageStream log( m_properties->domainProperties().service()->name() );
  log<<coral::Debug<<"SQLiteAccess::TableDescriptionProxy::dropColumn "<<name<<coral::MessageStream::endmsg;
#endif
  //if ( ! m_uniqueConstraintsRead )
  const_cast<coral::SQLiteAccess::TableDescriptionProxy*>( this )->refreshConstraints();
  //if ( ! m_foreignKeysRead )
  const_cast<coral::SQLiteAccess::TableDescriptionProxy*>( this )->refreshForeignKeyInfo();
  //if ( ! m_indicesRead )
  const_cast<coral::SQLiteAccess::TableDescriptionProxy*>( this )->refreshIndexInfo();
  m_description->dropColumn( name ); //build new transient description
  for(int i=0; i<m_description->numberOfIndices(); ++i) {
    const coral::IIndex& myidx=m_description->index(i);
    const std::vector<std::string>& colnames=myidx.columnNames();
    if(std::find(colnames.begin(),colnames.end(),name)!=colnames.end()) {
      m_description->dropIndex(myidx.name());
    }
  }
  try{
    this->alterschema();
  }catch( const coral::SchemaException& er ) {
    throw er; //rethrow exception
  }
  // Reconstruct the column proxies
  for ( std::vector< coral::SQLiteAccess::ColumnProxy* >::iterator iColumn = m_columns.begin(); iColumn != m_columns.end(); ++iColumn ) delete *iColumn;
  m_columns.clear();
  int numberOfColumns=m_description->numberOfColumns();
  for ( int i = 0; i < numberOfColumns; ++i ) {
    m_columns.push_back( new coral::SQLiteAccess::ColumnProxy( m_description->columnDescription( i ), *this ) );
  }
}

void
coral::SQLiteAccess::TableDescriptionProxy::renameColumn( const std::string& originalName,const std::string& newName )
{

#ifdef _DEBUG
  //  coral::MessageStream log( m_properties->domainProperties().service(),
  //                           m_properties->domainProperties().service()->name(),
  //                           coral::Nil );
  coral::MessageStream log( m_properties->domainProperties().service()->name() );
  log<<coral::Debug<<"SQLiteAccess::TableDescriptionProxy::renameColumn "<<originalName<<" TO "<<newName<<coral::MessageStream::endmsg;
#endif
  //if ( ! m_uniqueConstraintsRead )
  const_cast<coral::SQLiteAccess::TableDescriptionProxy*>( this )->refreshConstraints();
  //if ( ! m_foreignKeysRead )
  const_cast<coral::SQLiteAccess::TableDescriptionProxy*>( this )->refreshForeignKeyInfo();
  //if ( ! m_indicesRead )
  const_cast<coral::SQLiteAccess::TableDescriptionProxy*>( this )->refreshIndexInfo();
  std::vector< std::string > oldnames;
  for(std::vector< coral::SQLiteAccess::ColumnProxy* >::iterator i=m_columns.begin(); i!=m_columns.end(); ++i) {
    oldnames.push_back( (*i)->name() );
  }
  m_description->renameColumn( originalName,newName );
  std::ostringstream os;
  coral::SQLiteAccess::SQLiteStatement stmt(m_properties);
  coral::SQLiteAccess::SQLiteTableBuilder tempbuilder( *m_description, m_properties, true );
  os<<tempbuilder.statement();
  if ( !stmt.prepare(os.str()) || ! stmt.execute() ) {
    m_description->renameColumn(newName, originalName);
    throw coral::SchemaException( m_properties->domainProperties().service()->name(),"Could not create the temporary table","TableDescriptionProxy::renameColumn" );
  }
  stmt.reset();
  std::ostringstream os2;
  os2<<"INSERT INTO \""<<tempbuilder.tableName()<<"\" SELECT ";
  for ( std::vector<std::string>::iterator it=oldnames.begin(); it!=oldnames.end(); ++it ) {
    if( it!=oldnames.begin() ) os2<<" , ";
    os2<<"\""<<*it<<"\"" ;
  }
  os2<<" FROM \""<<m_description->name()<<"\"" ;
  if ( !stmt.prepare(os2.str()) || ! stmt.execute() ) {
    m_description->renameColumn( originalName,newName );
    throw coral::SchemaException( m_properties->domainProperties().service()->name(),"Could not copy data to the temporary table","TableDescriptionProxy::renameColumn" );
  }
  stmt.reset();
  if ( !stmt.prepare( "DROP TABLE \""+m_description->name()+"\"" ) || ! stmt.execute() ) {
    m_description->renameColumn( originalName,newName );
    throw coral::SchemaException( m_properties->domainProperties().service()->name(),"Could not drop the original table","TableDescriptionProxy::renameColumn" );
  }
  stmt.reset();
  std::ostringstream os3;
  coral::SQLiteAccess::SQLiteTableBuilder rebuilder( *m_description, m_properties, false );
  os3<<rebuilder.statement();
  if ( !stmt.prepare(os3.str()) || ! stmt.execute() ) {
    m_description->renameColumn( originalName,newName );
    throw coral::SchemaException( m_properties->domainProperties().service()->name(),"Could not recreate the original table","TableDescriptionProxy::renameColumn" );
  }
  stmt.reset();
  //insert the old data from temp table to orig table
  std::ostringstream os4;
  os4<<"INSERT INTO \""<<rebuilder.tableName()<<"\" SELECT ";
  int numberOfColumns=m_description->numberOfColumns();
  for ( int iColumn = 0; iColumn < numberOfColumns; ++iColumn ) {
    if(iColumn!=0) os4<<" , ";
    const coral::IColumn& column = m_description->columnDescription( iColumn );
    os4 << "\""<<column.name()<<"\"" ;
  }
  os4<<" FROM \""<< tempbuilder.tableName()<<"\"" ;
  if ( !stmt.prepare(os4.str()) || ! stmt.execute() ) {
    m_description->renameColumn( originalName,newName );
    throw coral::SchemaException( m_properties->domainProperties().service()->name(),"Could not copy data from temporary table to original table","TableDescriptionProxy::renameColumn" );
  }
  stmt.reset();

  //drop the temp table
  if ( !stmt.prepare( "DROP TABLE \""+tempbuilder.tableName()+"\"" ) || ! stmt.execute() ) {
    m_description->renameColumn( originalName,newName );
    throw coral::SchemaException( m_properties->domainProperties().service()->name(),"Could not drop a column from the temporary table","TableDescriptionProxy::renameColumn" );
  }
  stmt.reset();
  //
  //recreate indices
  //
  for(int i=0; i<m_description->numberOfIndices(); ++i) {
    const coral::IIndex& myidx=m_description->index(i);
    std::string myidxName=myidx.name();
    bool isUnique=myidx.isUnique();
    const std::vector<std::string>& colnames=myidx.columnNames();
    if( std::find(colnames.begin(),colnames.end(),originalName)!=colnames.end() ) {
      std::vector<std::string> result;
      std::copy(colnames.begin(), colnames.end(), std::back_inserter(result) );
      std::replace(result.begin(),result.end(),originalName,newName);
      m_description->dropIndex(myidxName);
      this->createIndex(myidxName,result,isUnique,"");
    }else{
      this->createIndex(myidxName,colnames,isUnique,"");
    }
  }
}
void
coral::SQLiteAccess::TableDescriptionProxy::changeColumnType( const std::string& columnName,
                                                              const std::string& typeName,
                                                              int /* size */,
                                                              bool /* fixedSize */ )
{

#ifdef _DEBUG
  //  coral::MessageStream log( m_properties->domainProperties().service(),
  //                           m_properties->domainProperties().service()->name(),
  //                           coral::Nil );
  coral::MessageStream log( m_properties->domainProperties().service()->name() );
  log<<coral::Debug<<"SQLiteAccess::TableDescriptionProxy::changeColumnType of "<<columnName<<"TO "<<typeName<<coral::MessageStream::endmsg;
#endif
  //if ( ! m_uniqueConstraintsRead )
  const_cast<coral::SQLiteAccess::TableDescriptionProxy*>( this )->refreshConstraints();
  //if ( ! m_foreignKeysRead )
  const_cast<coral::SQLiteAccess::TableDescriptionProxy*>( this )->refreshForeignKeyInfo();
  //if ( ! m_indicesRead )
  const_cast<coral::SQLiteAccess::TableDescriptionProxy*>( this )->refreshIndexInfo();
  std::string oldtype=m_description->columnDescription(columnName).type();
  m_description->changeColumnType( columnName,typeName);
  try{
    this->alterschema();
  }catch( const coral::SchemaException& er ) {
    m_description->changeColumnType( columnName,oldtype);
    coral::MessageStream log( m_properties->domainProperties().service()->name() );
    log<<coral::Error << er.what() <<coral::MessageStream::endmsg;
    throw coral::SchemaException( m_properties->domainProperties().service()->name(),
                                  "Could not change the type of a column in a table ",
                                  "ITableSchemaEditor::changeColumnType" ); //rethrow exception
  }
}


void
coral::SQLiteAccess::TableDescriptionProxy::setNotNullConstraint( const std::string& columnName,
                                                                  bool isNotNull )
{

#ifdef _DEBUG
  //  coral::MessageStream log( m_properties->domainProperties().service(),
  //                           m_properties->domainProperties().service()->name(),
  //                           coral::Nil );
  coral::MessageStream log( m_properties->domainProperties().service()->name() );
  log<<coral::Debug<<"SQLiteAccess::TableDescriptionProxy::setNotNullConstraint "<<columnName<<"TO "<<isNotNull<<coral::MessageStream::endmsg;
#endif
  //if ( ! m_uniqueConstraintsRead )
  const_cast<coral::SQLiteAccess::TableDescriptionProxy*>( this )->refreshConstraints();
  //if ( ! m_foreignKeysRead )
  const_cast<coral::SQLiteAccess::TableDescriptionProxy*>( this )->refreshForeignKeyInfo();
  //if ( ! m_indicesRead )
  const_cast<coral::SQLiteAccess::TableDescriptionProxy*>( this )->refreshIndexInfo();
  m_description->setNotNullConstraint( columnName, isNotNull );
  try{
    this->alterschema();
  }catch( const coral::SchemaException& er ) {
    m_description->setNotNullConstraint( columnName, !isNotNull );
    coral::MessageStream log( m_properties->domainProperties().service()->name() );
    log<<coral::Error << er.what() <<coral::MessageStream::endmsg;

    throw coral::SchemaException( m_properties->domainProperties().service()->name(),
                                  "Could not change the NOT NULL constraint of a column in a table",
                                  "ITableSchemaEditor::setNotNullConstraint" );
  }
}

void
coral::SQLiteAccess::TableDescriptionProxy::setUniqueConstraint( const std::string& columnName,
                                                                 std::string name,
                                                                 bool isUnique,
                                                                 std::string tableSpaceName )
{

  this->setUniqueConstraint( std::vector<std::string>( 1, columnName ),name,isUnique,tableSpaceName );
}


void
coral::SQLiteAccess::TableDescriptionProxy::setUniqueConstraint( const std::vector<std::string>& columnNames,
                                                                 std::string name, bool isUnique,
                                                                 std::string /* tableSpaceName */)
{

#ifdef _DEBUG
  //  coral::MessageStream log( m_properties->domainProperties().service(),
  //                           m_properties->domainProperties().service()->name(),
  //                           coral::Nil );
  coral::MessageStream log( m_properties->domainProperties().service()->name() );
  log<<coral::Debug<<"SQLiteAccess::TableDescriptionProxy::setUniqueConstraint "<<coral::MessageStream::endmsg;
#endif
  //if ( ! m_foreignKeysRead )
  const_cast<coral::SQLiteAccess::TableDescriptionProxy*>( this )->refreshForeignKeyInfo();
  //if ( ! m_indicesRead )
  const_cast<coral::SQLiteAccess::TableDescriptionProxy*>( this )->refreshIndexInfo();
  this->refreshConstraints();
  m_description->setUniqueConstraint( columnNames,name,isUnique,"" );
  try{
    this->alterschema();
  }catch( const coral::SchemaException& er ) {
    m_description->setUniqueConstraint( columnNames,name,!isUnique,"" );
    coral::MessageStream log( m_properties->domainProperties().service()->name() );
    log<<coral::Error<< er.what() <<coral::MessageStream::endmsg;

    throw coral::SchemaException( m_properties->domainProperties().service()->name(),
                                  "Could not set a unique constraint from the table",
                                  "ITableSchemaEditor::setUniqueConstraint" );
  }
}

void
coral::SQLiteAccess::TableDescriptionProxy::setPrimaryKey( const std::string& columnName,
                                                           std::string tableName)
{

  this->setPrimaryKey( std::vector< std::string >( 1, columnName ), tableName );
}


void
coral::SQLiteAccess::TableDescriptionProxy::setPrimaryKey( const std::vector<std::string>& columnNames,
                                                           std::string /* tableSpaceName */ )
{

#ifdef _DEBUG
  //  coral::MessageStream log( m_properties->domainProperties().service(),
  //                           m_properties->domainProperties().service()->name(),
  //                           coral::Nil );
  coral::MessageStream log( m_properties->domainProperties().service()->name() );
  log<<coral::Debug<<"SQLiteAccess::TableDescriptionProxy::setPrimaryKey "<<coral::MessageStream::endmsg;
#endif
  //  if ( ! m_uniqueConstraintsRead )
  const_cast<coral::SQLiteAccess::TableDescriptionProxy*>( this )->refreshConstraints();
  //if ( ! m_foreignKeysRead )
  const_cast<coral::SQLiteAccess::TableDescriptionProxy*>( this )->refreshForeignKeyInfo();
  //if ( ! m_indicesRead )
  const_cast<coral::SQLiteAccess::TableDescriptionProxy*>( this )->refreshIndexInfo();
  m_description->setPrimaryKey( columnNames,"" );
  try{
    this->alterschema();
  }catch( const coral::SchemaException& er ) {
    m_description->dropPrimaryKey();
    coral::MessageStream log( m_properties->domainProperties().service()->name() );
    log<<coral::Error << er.what() <<coral::MessageStream::endmsg;
    throw coral::SchemaException( m_properties->domainProperties().service()->name(),
                                  "Could not add a primary key constraint on a table",
                                  "ITableSchemaEditor::setPrimaryKey" );
  }
}

void
coral::SQLiteAccess::TableDescriptionProxy::dropPrimaryKey()
{

#ifdef _DEBUG
  //  coral::MessageStream log( m_properties->domainProperties().service(),
  //                           m_properties->domainProperties().service()->name(),
  //                           coral::Nil );
  coral::MessageStream log( m_properties->domainProperties().service()->name() );
  log<<coral::Debug<<"SQLiteAccess::TableDescriptionProxy::dropPrimaryKey "<<coral::MessageStream::endmsg;
#endif
  //if ( ! m_uniqueConstraintsRead )
  const_cast<coral::SQLiteAccess::TableDescriptionProxy*>( this )->refreshConstraints();
  //if ( ! m_foreignKeysRead )
  const_cast<coral::SQLiteAccess::TableDescriptionProxy*>( this )->refreshForeignKeyInfo();
  //if ( ! m_indicesRead )
  const_cast<coral::SQLiteAccess::TableDescriptionProxy*>( this )->refreshIndexInfo();
  m_description->dropPrimaryKey();
  this->alterschema();
}

void
coral::SQLiteAccess::TableDescriptionProxy::createIndex( const std::string& indexName,
                                                         const std::string& columnName,
                                                         bool isUnique,
                                                         std::string /* tableSpaceName */ )
{

  this->createIndex( indexName, std::vector<std::string>( 1, columnName ), isUnique, "");
}


void
coral::SQLiteAccess::TableDescriptionProxy::createIndex( const std::string& name,
                                                         const std::vector<std::string>& columnNames,
                                                         bool isUnique,
                                                         std::string /* tableSpaceName */ )
{

  /**create index in db AND update transient registry. Ignore coral::InvalidIndexIdentifierException
   */
#ifdef _DEBUG
  //  coral::MessageStream log( m_properties->domainProperties().service(),
  //                           m_properties->domainProperties().service()->name(),
  //                           coral::Nil );
  coral::MessageStream log( m_properties->domainProperties().service()->name() );
  log<<coral::Debug<<"SQLiteAccess::TableDescriptionProxy::createIndex "<<name<<coral::MessageStream::endmsg;
#endif
  //if ( ! m_columnDescriptionRead )
  const_cast<coral::SQLiteAccess::TableDescriptionProxy*>( this )->readColumnDescription();
  //if ( ! m_indicesRead )
  const_cast<coral::SQLiteAccess::TableDescriptionProxy*>( this )->refreshIndexInfo();

  // Create the transient one
  try{
    m_description->createIndex( name,
                                columnNames,
                                isUnique,
                                "" );
  }catch( const coral::InvalidIndexIdentifierException& ) {
    //std::cout<<"caught in create Index, ignore in create Index "<<std::endl;
  }
  std::ostringstream os;
  os << "CREATE ";
  if ( isUnique ) os << "UNIQUE ";
  os << "INDEX \"" << name << "\" ON \""
     << m_description->name()<<"\"( ";
  for ( std::vector<std::string>::const_iterator iColumn = columnNames.begin();
        iColumn != columnNames.end(); ++iColumn ) {
    if ( iColumn != columnNames.begin() ) os << ", ";
    os << "\""<<*iColumn <<"\"";
  }
  os << " )";
  coral::SQLiteAccess::SQLiteStatement stmt( m_properties);
  if ( ! stmt.prepare( os.str() ) || !stmt.execute() ) {
    m_description->dropIndex( name );
    throw coral::SchemaException( m_properties->domainProperties().service()->name(), "Could not create an index on the table","ITableSchemaEditor::createIndex" );
  }
}

void
coral::SQLiteAccess::TableDescriptionProxy::dropIndex( const std::string& indexName )
{

#ifdef _DEBUG
  //  coral::MessageStream log( m_properties->domainProperties().service(),
  //                           m_properties->domainProperties().service()->name(),
  //                           coral::Nil );
  coral::MessageStream log( m_properties->domainProperties().service()->name() );
  log<<coral::Debug<<"SQLiteAccess::TableDescriptionProxy::dropIndex "<<indexName<<coral::MessageStream::endmsg;
#endif
  //const_cast<coral::SQLiteAccess::TableDescriptionProxy*>( this )->readColumnDescription();
  const_cast<coral::SQLiteAccess::TableDescriptionProxy*>( this )->refreshIndexInfo();
  try{
    m_description->dropIndex(indexName);
  }catch(const coral::InvalidIndexIdentifierException&) {
    throw coral::SchemaException( m_properties->domainProperties().service()->name(), "Index "+indexName+" doesnot exist in table description","ITableSchemaEditor::dropIndex" );
  }
  coral::SQLiteAccess::SQLiteStatement stmt( m_properties );
  if( !stmt.prepare("DROP INDEX \"" +indexName+"\"") || !stmt.execute() ) {
    throw coral::SchemaException( m_properties->domainProperties().service()->name(), "Could not drop an index from the table","ITableSchemaEditor::dropIndex" );
  }
}

#ifdef CORAL240DC
void
coral::SQLiteAccess::TableDescriptionProxy::createForeignKey( const std::string& name,
                                                              const std::string& columnName,
                                                              const std::string& referencedTableName,
                                                              const std::string& referencedColumnName,
                                                              bool onDeleteCascade )
{

  this->createForeignKey( name,
                          std::vector< std::string >( 1, columnName ),
                          referencedTableName,
                          std::vector< std::string >( 1, referencedColumnName ),
                          onDeleteCascade );
}
#else
void
coral::SQLiteAccess::TableDescriptionProxy::createForeignKey( const std::string& name,
                                                              const std::string& columnName,
                                                              const std::string& referencedTableName,
                                                              const std::string& referencedColumnName )
{

  this->createForeignKey( name,
                          std::vector< std::string >( 1, columnName ),
                          referencedTableName,
                          std::vector< std::string >( 1, referencedColumnName ) );
}
#endif

#ifdef CORAL240DC
void
coral::SQLiteAccess::TableDescriptionProxy::createForeignKey( const std::string& name,
                                                              const std::vector<std::string>& columnNames,
                                                              const std::string& referencedTableName,
                                                              const std::vector<std::string>& referencedColumnNames,
                                                              bool onDeleteCascade )
{

#ifdef _DEBUG
  //  coral::MessageStream log( m_properties->domainProperties().service(),
  //                           m_properties->domainProperties().service()->name(),
  //                           coral::Nil );
  coral::MessageStream log( m_properties->domainProperties().service()->name() );
  log<<coral::Debug<<"SQLiteAccess::TableDescriptionProxy::createForeignKey ON "<<name<<" TO "<<referencedTableName<<coral::MessageStream::endmsg;
#endif
  //if ( ! m_uniqueConstraintsRead )
  const_cast<coral::SQLiteAccess::TableDescriptionProxy*>( this )->refreshConstraints();
  //if ( ! m_foreignKeysRead )
  const_cast<coral::SQLiteAccess::TableDescriptionProxy*>( this )->refreshForeignKeyInfo();
  //if ( ! m_indicesRead )
  const_cast<coral::SQLiteAccess::TableDescriptionProxy*>( this )->refreshIndexInfo();
  m_description->createForeignKey( name,
                                   columnNames,
                                   referencedTableName,
                                   referencedColumnNames,
                                   onDeleteCascade );
  try
  {
    this->alterschema();
  }
  catch( const coral::SchemaException& er )
  {
    m_description->dropForeignKey( name );
    //    coral::MessageStream log( m_properties->domainProperties().service(),
    //                             m_properties->domainProperties().service()->name(),
    //                             coral::Nil );
    coral::MessageStream log( m_properties->domainProperties().service()->name() );
    log<< coral::Error << er.what() <<coral::MessageStream::endmsg;
    throw coral::SchemaException( m_properties->domainProperties().service()->name(),
                                  "Could not create a foreign key constraint on the table",
                                  "ITableSchemaEditor::createForeignKey" );
  }
}
#else
void
coral::SQLiteAccess::TableDescriptionProxy::createForeignKey( const std::string& name,
                                                              const std::vector<std::string>& columnNames,
                                                              const std::string& referencedTableName,
                                                              const std::vector<std::string>& referencedColumnNames )
{

#ifdef _DEBUG
  //  coral::MessageStream log( m_properties->domainProperties().service(),
  //                           m_properties->domainProperties().service()->name(),
  //                           coral::Nil );
  coral::MessageStream log( m_properties->domainProperties().service()->name() );
  log<<coral::Debug<<"SQLiteAccess::TableDescriptionProxy::createForeignKey ON "<<name<<" TO "<<referencedTableName<<coral::MessageStream::endmsg;
#endif
  //if ( ! m_uniqueConstraintsRead )
  const_cast<coral::SQLiteAccess::TableDescriptionProxy*>( this )->refreshConstraints();
  //if ( ! m_foreignKeysRead )
  const_cast<coral::SQLiteAccess::TableDescriptionProxy*>( this )->refreshForeignKeyInfo();
  //if ( ! m_indicesRead )
  const_cast<coral::SQLiteAccess::TableDescriptionProxy*>( this )->refreshIndexInfo();
  m_description->createForeignKey( name,
                                   columnNames,
                                   referencedTableName,
                                   referencedColumnNames );
  try{
    this->alterschema();
  }catch( const coral::SchemaException& er ) {
    m_description->dropForeignKey( name );
    //    coral::MessageStream log( m_properties->domainProperties().service(),
    //                             m_properties->domainProperties().service()->name(),
    //                             coral::Nil );
    coral::MessageStream log( m_properties->domainProperties().service()->name() );
    log<< coral::Error << er.what() <<coral::MessageStream::endmsg;
    throw coral::SchemaException( m_properties->domainProperties().service()->name(),
                                  "Could not create a foreign key constraint on the table",
                                  "ITableSchemaEditor::createForeignKey" );
  }
}
#endif

void
coral::SQLiteAccess::TableDescriptionProxy::dropForeignKey( const std::string& name )
{

#ifdef _DEBUG
  //  coral::MessageStream log( m_properties->domainProperties().service(),
  //                           m_properties->domainProperties().service()->name(),
  //                           coral::Nil );
  coral::MessageStream log( m_properties->domainProperties().service()->name() );
  log<<coral::Debug<<"SQLiteAccess::TableDescriptionProxy::dropForeignKey ON "<<name<<coral::MessageStream::endmsg;
#endif
  //if ( ! m_uniqueConstraintsRead )
  const_cast<coral::SQLiteAccess::TableDescriptionProxy*>( this )->refreshConstraints();
  //if ( ! m_foreignKeysRead )
  this->refreshForeignKeyInfo();
  //if ( ! m_indicesRead )
  const_cast<coral::SQLiteAccess::TableDescriptionProxy*>( this )->refreshIndexInfo();
  m_description->dropForeignKey( name );
  this->alterschema();
}

//-----------------------------------------------------------------------------

void
coral::SQLiteAccess::TableDescriptionProxy::alterschema()
{

  /*
  bool debug = // printouts for bug #54806, aka bug #54767
    ( m_description->name() == "COOLSRC_F0007_IOVS" ||
      m_description->name() == "COOLTGT_F0007_IOVS" );
  if ( debug )
  {
    std::cout << "SQLiteAccess alterSchema - List of indexes:" << std::endl;
    for(int i=0; i<m_description->numberOfIndices();++i){
      bool isUnique=m_description->index(i).isUnique();
      std::string idxname=m_description->index(i).name();
      std::cout<<idxname<<", "<<(isUnique?"unique":"notunique")<<std::endl;
    }
  }
  */
#ifdef _DEBUG
  //  coral::MessageStream log( m_properties->domainProperties().service(),
  //                           m_properties->domainProperties().service()->name(),
  //                           coral::Nil );
  coral::MessageStream log(m_properties->domainProperties().service()->name() );
  log <<coral::Debug<<"SQLiteAccess::TableDescriptionProxy::alterschema "<<coral::MessageStream::endmsg;
#endif
  coral::SQLiteAccess::SQLiteStatement stmt(m_properties);
  //create fk metadata table
  //if(!stmt.prepare("DROP TABLE IF EXISTS coral_sqlite_fk" )|| !stmt.execute()){
  //  throw coral::SchemaException( m_properties->domainProperties().service()->name(),"Could not drop table coral_sqlite_fk","TableDescriptionProxy::alterschema" );
  //}
  std::string qq("DELETE FROM coral_sqlite_fk WHERE tablename=\'");
  qq += m_description->name();
  qq += "\'";
  if( !stmt.prepare(qq) || !stmt.execute())
  {
    throw coral::SchemaException( m_properties->domainProperties().service()->name()
                                  , "Could not clean table coral_sqlite_fk"
                                  , "TableDescriptionProxy::alterschema" );
  }
  stmt.reset();
  // Set the strings for the two tables
  std::string origname( m_description->name() );
  // Create a tables random name
  std::string tempname( "TMP_" );
  // Append random chars
  getRandomName(tempname, 20);
  // Experimental: try to create a normal table with a random name
  m_description->setName( tempname );
  coral::SQLiteAccess::SQLiteTableBuilder rebuilder01( *m_description, m_properties, false );

  if ( !stmt.prepare( rebuilder01.statement() ) || ! stmt.execute() )
  {
    throw coral::SchemaException( m_properties->domainProperties().service()->name()
                                  , "Could not recreate the original table"
                                  , "TableDescriptionProxy::alterschema" );
  }
  stmt.reset();

  //os.str("");
  //if(m_description->numberOfForeignKeys()!=0){
  //  os<<"CREATE TABLE coral_sqlite_fk(id TEXT NOT NULL, name TEXT, tablename TEXT) ; ";
  //  if ( !stmt.prepare(os.str()) || ! stmt.execute() ) {
  //    throw coral::SchemaException( m_properties->domainProperties().service()->name(),"Could not create table coral_sqlite_fk","TableDescriptionProxy::alterschema" );
  //  }
  //  stmt.reset();
  // os.str("")
  //create temporary table as new schema

  // Insert in the new table the data from orig table
  std::ostringstream os2;
  os2 << "INSERT INTO \"" << rebuilder01.tableName() << "\" SELECT ";
  int numberOfColumns = m_description->numberOfColumns();
  for ( int iColumn = 0; iColumn < numberOfColumns; ++iColumn )
  {
    if( iColumn != 0 )
    {
      os2 << " , ";
    }
    const coral::IColumn& column = m_description->columnDescription( iColumn );
    os2 << "\"" << column.name() << "\"";
  }
  os2 << " FROM \"" << origname << "\"";

  if ( !stmt.prepare(os2.str()) || ! stmt.execute() )
  {
    throw coral::SchemaException( m_properties->domainProperties().service()->name()
                                  , "Could not copy data to the new table"
                                  , "TableDescriptionProxy::alterschema" );
  }
  stmt.reset();
  // Drop the orig table
  if ( !stmt.prepare( "DROP TABLE \"" + origname + "\"" ) || ! stmt.execute() )
  {
    throw coral::SchemaException( m_properties->domainProperties().service()->name()
                                  , "Could not drop the original table"
                                  , "TableDescriptionProxy::alterschema" );
  }
  // Experiment: Alter the name of the table
  if ( !stmt.prepare( "ALTER TABLE \"" + tempname + "\" RENAME TO \"" + origname + "\"" ) || ! stmt.execute() )
  {
    throw coral::SchemaException( m_properties->domainProperties().service()->name()
                                  , "Could not change name of the table"
                                  , "TableDescriptionProxy::alterschema" );
  }
  m_description->setName( origname );

  //
  //recreate indices
  //

  for(int i=0; i<m_description->numberOfIndices(); ++i) {
    bool isUnique=m_description->index(i).isUnique();
    std::string idxname=m_description->index(i).name();
    this->createIndex(idxname,m_description->index(i).columnNames(),isUnique,""); // this was sometimes failing due to bug #54806 (unique constraint applied to non-unique columns)
  }
  //m_indicesRead=false;
  //if found fk constraint, create sqlite_fk table, add fk map

  //recreate fk
  //add fk id(i) to fk name(get from description) mapping
  //
  //stmt.reset();
  std::ostringstream os5;
  for ( int i = 0; i < m_description->numberOfForeignKeys(); ++i ) {
    os5<<"INSERT INTO coral_sqlite_fk(id,name,tablename)  VALUES(";
    std::string fkname=m_description->foreignKey(i).name();
    os5<<"\""<<(int)i<<"\"";
    os5<<",\""<<fkname<<"\"";
    os5<<",\""<<m_description->name()<<"\" )";
    if ( !stmt.prepare(os5.str()) || ! stmt.execute() ) {
      throw coral::SchemaException( m_properties->domainProperties().service()->name(),"Could not register FK metadata","TableDescriptionProxy::alterschema" );
    }
    stmt.reset();
    os5.str("");
  }
}

void
coral::SQLiteAccess::TableDescriptionProxy::createFKtableIfNotexists(){

  if (m_properties->isReadOnly()) return;
  std::string cfk("CREATE TABLE IF NOT EXISTS coral_sqlite_fk(id TEXT NOT NULL, name TEXT, tablename TEXT NOT NULL) ; ");
  coral::SQLiteAccess::SQLiteStatement stmt(m_properties);
  if ( ! stmt.prepare(cfk) || ! stmt.execute() ) {
    throw coral::SchemaException( m_properties->domainProperties().service()->name(),"Could not create table coral_sqlite_fk","TableDescriptionProxy::createFKtableIfNonexists" );
  }
  stmt.reset();
}
