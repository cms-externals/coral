// Include files
#include <cstdio>
#include <cstring>
#include <stdexcept>
#include <iostream>

// Coral Exceptions
#include "CoralBase/Attribute.h"
#include "CoralBase/AttributeException.h"
#include "CoralBase/AttributeListException.h"
#include "CoralBase/Blob.h"
#include "CoralBase/Date.h"
#include "CoralBase/TimeStamp.h"
#include "RelationalAccess/AuthenticationServiceException.h"
#include "RelationalAccess/ConnectionServiceException.h"
#include "RelationalAccess/LookupServiceException.h"
#include "RelationalAccess/MonitoringException.h"
#include "RelationalAccess/RelationalServiceException.h"
#include "RelationalAccess/SchemaException.h"
#include "RelationalAccess/SessionException.h"
#include "RelationalAccess/WebCacheControlException.h"

// Local include files
#include "DummyFacade.h"
#include "DummyRowIterator.h"
#include "Exceptions.h"

// Namespace
using namespace coral::CoralStubs;

//-----------------------------------------------------------------------------

DummyFacade::DummyFacade()
{
}

//-----------------------------------------------------------------------------

DummyFacade::~DummyFacade()
{
}

//-----------------------------------------------------------------------------

void
DummyFacade::setCertificateData( const coral::ICertificateData* /*cert*/ )
{
  // DUMMY
}

//-----------------------------------------------------------------------------

coral::Token
DummyFacade::connect( const std::string& dbUrl,
                      const coral::AccessMode mode,
                      bool& /*fromProxy*/ ) const
{
  if(mode == coral::ReadOnly) return 42;
  throw ConnectionNotAvailableException(dbUrl, "DummyFacade::connect");
}

//-----------------------------------------------------------------------------

void
DummyFacade::releaseSession( Token /*sessionID*/ ) const
{
}

//-----------------------------------------------------------------------------

void
DummyFacade::startTransaction( Token /*sessionID*/,
                               bool readOnly ) const
{
  if(!readOnly) throw TransactionException("coral::CoralStubs","Can't open read-write transaction","DummyFacade::startTransaction");
}

//-----------------------------------------------------------------------------

void
DummyFacade::commitTransaction( Token /*sessionID*/ ) const
{
  // throw std::runtime_error("May the force be with you");
}

//-----------------------------------------------------------------------------

void
DummyFacade::rollbackTransaction( Token /*sessionID*/ ) const
{
  // throw std::runtime_error("May the force be with you");
}

//-----------------------------------------------------------------------------

const std::vector<std::string>
DummyFacade::fetchSessionProperties( Token /*sessionID*/ ) const
{
  std::vector<std::string> h;
  h.push_back("Test01");
  return h;
}

//-----------------------------------------------------------------------------

coral::IRowIteratorPtr
DummyFacade::fetchRows( Token /*sessionID*/,
                        const QueryDefinition& /*qd*/,
                        AttributeList* /*pRowBuffer*/,
                        size_t /*cacheSize*/,
                        bool /*cacheSizeInMB*/ ) const
{
  DummyRowIterator* tall = new DummyRowIterator;
  for(size_t i = 0; i < 5; i++ )
  {
    AttributeList& attr = tall->addRow();
    attr.extend("A", typeid(coral::TimeStamp));
    attr.extend("B", typeid(double));
    attr.extend("C1", typeid(std::string));
    attr.extend("C2", typeid(std::string));
    attr.extend("D", typeid(int));
    attr.extend("E", typeid(coral::Blob));
    coral::TimeStamp time(2006, 4, 23, 12, i, 6, 23);
    attr[0].data<coral::TimeStamp>() = time;
    attr[1].data<double>() = 23.54 * i;
    attr[2].data<std::string>() = "2009-02-05_09:44:08.269264000 GMT";
    attr[3].data<std::string>() = "2009-02-05_09:44:08.269264000 GMT";
    attr[4].data<int>() = i;
    coral::Blob& myblob = attr[5].data<coral::Blob>();
    std::cout << "Initialise the blob ... " << std::endl;
    size_t maxcby256 = 4000;
    size_t maxc = 256*maxcby256; // 1 MB
    myblob.resize( maxc + 1 );
    unsigned char* p0 = static_cast<unsigned char*>(myblob.startingAddress());
    unsigned char* p = p0;
    for ( size_t j = 0; j < 256; ++j, ++p ) *p = (unsigned char)j;
    for ( size_t j = 1; j < maxcby256; ++j ) std::memcpy( p0+j*256, p0, 256 );
    std::cout << "Done" << std::endl;
  }
  return std::auto_ptr<IRowIterator>(tall);
}

//-----------------------------------------------------------------------------

coral::IRowIteratorPtr
DummyFacade::fetchRows( Token /*sessionID*/,
                        const QueryDefinition& /*qd*/,
                        const std::map< std::string,
                        std::string > /*outputTypes*/,
                        size_t /*cacheSize*/,
                        bool /*cacheSizeInMB*/ ) const
{
  throw StubsException("DummyFacade::fetchRows (with output types), This function is not available");
}

//-----------------------------------------------------------------------------

coral::IRowIteratorPtr
DummyFacade::fetchAllRows( Token /*sessionID*/,
                           const QueryDefinition& /*qd*/,
                           AttributeList* rowBuffer ) const
{
  if( rowBuffer != NULL )
    rowBuffer->toOutputStream(std::cout) << std::endl;
  DummyRowIterator* tall = new DummyRowIterator;
  for(size_t i = 0; i < 30; i++ )
  {
    AttributeList& attr = tall->addRow();
    attr.extend("A", typeid(coral::TimeStamp));
    attr.extend("B", typeid(double));
    attr.extend("C1", typeid(std::string));
    attr.extend("C2", typeid(std::string));
    attr.extend("D", typeid(int));
    attr.extend("E", typeid(coral::Blob));
    coral::TimeStamp time(2006, 4, 23, 12, i, 6, 23);
    attr[0].data<coral::TimeStamp>() = time;
    attr[1].data<double>() = 23.54 * i;
    attr[2].data<std::string>() = "2009-02-05_09:44:08.269264000 GMT";
    attr[3].data<std::string>() = "2009-02-05_09:44:08.269264000 GMT";
    attr[4].data<int>() = i;
    coral::Blob& myblob = attr[5].data<coral::Blob>();
    std::cout << "Initialise the blob ... " << std::endl;
    size_t maxcby256 = 4000;
    size_t maxc = 256*maxcby256; // 1 MB
    myblob.resize( maxc + 1 );
    unsigned char* p0 = static_cast<unsigned char*>(myblob.startingAddress());
    unsigned char* p = p0;
    for ( size_t j = 0; j < 256; ++j, ++p ) *p = (unsigned char)j;
    for ( size_t j = 1; j < maxcby256; ++j ) std::memcpy( p0+j*256, p0, 256 );
    std::cout << "Done" << std::endl;
  }
  return std::auto_ptr<IRowIterator>(tall);
}

//-----------------------------------------------------------------------------

coral::IRowIteratorPtr
DummyFacade::fetchAllRows( Token /*sessionID*/,
                           const QueryDefinition& /*qd*/,
                           const std::map< std::string,
                           std::string > /*outputTypes*/ ) const
{
  throw StubsException("DummyFacade::fetchAllRows (with output types), This function is not available");
}

//-----------------------------------------------------------------------------

const std::set<std::string>
DummyFacade::listTables( Token /*sessionID*/,
                         const std::string& /*schemaName*/ ) const
{
  std::set<std::string> myset;

  myset.insert("the_big_table");
  myset.insert("the_small_table");

  return myset;
}

//-----------------------------------------------------------------------------

bool DummyFacade::existsTable( Token /*sessionID*/,
                               const std::string& /*schemaName*/,
                               const std::string& /*tableName*/ ) const
{
  return true;
}

//-----------------------------------------------------------------------------

const coral::TableDescription
DummyFacade::fetchTableDescription( Token /*sessionID*/,
                                    const std::string& /*schemaName*/,
                                    const std::string& /*tableName*/ ) const
{
  TableDescription td;

  td.setName("MyGreatTable");
  td.insertColumn("Column01", "string", 0, true, "Column01_SPName");

  return td;
}

//-----------------------------------------------------------------------------

const std::set<std::string>
DummyFacade::listViews( Token /*sessionID*/,
                        const std::string& /*schemaName*/ ) const
{
  throw StubsException("DummyFacade::listViews, This function is not available");
}

//-----------------------------------------------------------------------------

bool DummyFacade::existsView( Token /*sessionID*/,
                              const std::string& /*schemaName*/,
                              const std::string& /*viewName*/ ) const
{
  throw StubsException("DummyFacade::existsView, This function is not available");
}

//-----------------------------------------------------------------------------

const std::pair<coral::TableDescription,std::string>
DummyFacade::fetchViewDescription( Token /*sessionID*/,
                                   const std::string& /*schemaName*/,
                                   const std::string& /*viewName*/ ) const
{
  throw StubsException("DummyFacade::fetchViewDescription, This function is not available");
}

//-----------------------------------------------------------------------------

int
DummyFacade::deleteTableRows( Token /*sessionID*/,
                              const std::string& /*schemaName*/,
                              const std::string& /*tableName*/,
                              const std::string& /*whereClause*/,
                              const std::string& /*whereData*/ ) const
{
  return 0;
}

//-----------------------------------------------------------------------------

const std::string
DummyFacade::formatRowBufferAsString( Token /*sessionID*/,
                                      const std::string& /*schemaName*/,
                                      const std::string& /*tableName*/ ) const
{
  return std::string("");
}

//-----------------------------------------------------------------------------

void
DummyFacade::insertRowAsString( Token /*sessionID*/,
                                const std::string& /*schemaName*/,
                                const std::string& /*tableName*/,
                                const std::string& /*rowBufferAS*/ ) const
{
  throw StubsException("This function is not available");

}

//-----------------------------------------------------------------------------

coral::Token
DummyFacade::bulkInsertAsString( Token /*sessionID*/,
                                 const std::string& /*schemaName*/,
                                 const std::string& /*tableName*/,
                                 const std::string& /*rowBufferAS*/,
                                 int /*rowCacheSizeDb*/ ) const
{
  throw StubsException("This function is not available");
}

//-----------------------------------------------------------------------------

void
DummyFacade::releaseBulkOp( Token /*bulkOpID*/ ) const
{
  throw StubsException("This function is not available");

}

//-----------------------------------------------------------------------------

void
DummyFacade::processRows( Token /*bulkOpID*/,
                          const std::vector<coral::AttributeList>& /*rowsAS*/ ) const
{
  throw StubsException("This function is not available");

}

//-----------------------------------------------------------------------------

void
DummyFacade::flush( Token /*bulkOpID*/ ) const
{
  throw StubsException("This function is not available");

}

//-----------------------------------------------------------------------------

void
DummyFacade::callProcedure( Token /*sessionID*/,
                            const std::string& /*schemaName*/,
                            const std::string& /*procedureName*/,
                            const coral::AttributeList& /*inputArguments*/ ) const
{
  throw StubsException("DummyFacade::callProcedure, This function is not available");
}

//-----------------------------------------------------------------------------
