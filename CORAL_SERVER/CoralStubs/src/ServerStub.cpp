#include "CoralStubs/ServerStub.h"

#include "CoralServerBase/CALPacketHeader.h"
#include "CALProtocol.h"

#include "CoralServerBase/ByteBuffer.h"
#include "CoralServerBase/IRowIterator.h"
#include "CoralMonitor/StopTimer.h"

#include "RelationalAccess/AccessMode.h"
#include "Exceptions.h"
#include "SegmentWriterIterator.h"
#include "SegmentReaderIterator.h"
#include "SimpleByteBufferIterator.h"
#include "ByteBufferIteratorAll.h"

// Coral exceptions
#include "RelationalAccess/ConnectionServiceException.h"
#include "RelationalAccess/AuthenticationServiceException.h"
#include "RelationalAccess/RelationalServiceException.h"
#include "RelationalAccess/SchemaException.h"
#include "RelationalAccess/SessionException.h"
#include "RelationalAccess/MonitoringException.h"
#include "RelationalAccess/WebCacheControlException.h"
#include "RelationalAccess/LookupServiceException.h"
#include "CoralBase/AttributeException.h"
#include "CoralBase/AttributeListException.h"

#include <sstream>
#include <map>

#include "CppMutex.h"
#include "CppMutexFct.h"
#include "CppTypes.h"

#define LOGGER_NAME "CoralStubs::ServerStub"
#include "logger.h"

#include "StatsTypePayload.h"

namespace coral { namespace CoralStubs {

  struct rowIteratorProperties {

    IRowIterator* iterator;

    bool inmb;

    size_t cachesize;

  };

  struct rowIteratorMap {

    std::map<size_t, rowIteratorProperties*> maps;

    pmutex_t* mutex;
    //counter for concurent fetchall operations
    size_t m_fall_ops;

  };

  IByteBufferIterator*
  request_ConnectRO( ICoralFacade& facade, SegmentReaderIterator& sri, rowIteratorMap& )
  {
    SegmentWriterIterator* swi = new SegmentWriterIterator(CALOpcodes::ConnectRO, sri.cacheable(), true);
    //copy the proxy flag to the reply
    swi->setProxy( sri.proxy() );

    std::string url;
    sri.extract16( url );
    //call the facade method
    bool proxy = sri.proxy();

    Token sessionID = facade.connect( url, coral::ReadOnly, proxy );
    //write the session id to the stream

    swi->append( sessionID );
    //write all temp data to the buffers
    swi->flush();
    //return
    return swi;
  }

  IByteBufferIterator*
  request_ConnectRW( ICoralFacade& facade, SegmentReaderIterator& sri, rowIteratorMap& )
  {
    SegmentWriterIterator* swi = new SegmentWriterIterator(CALOpcodes::ConnectRW, sri.cacheable(), true);
    //copy the proxy flag to the reply
    swi->setProxy( sri.proxy() );

    std::string url;
    sri.extract16( url );
    //call the facade method
    bool proxy = sri.proxy();

    Token sessionID = facade.connect( url, coral::Update, proxy );
    //write the session id to the stream
    swi->append( sessionID );
    //write all temp data to the buffers
    swi->flush();
    //return
    return swi;
  }

  IByteBufferIterator*
  request_Disconnect( ICoralFacade& facade, SegmentReaderIterator& sri, rowIteratorMap& )
  {
    SegmentWriterIterator* swi = new SegmentWriterIterator(CALOpcodes::Disconnect, sri.cacheable(), true);
    //copy the proxy flag to the reply
    swi->setProxy( sri.proxy() );

    coral::Token sessionID;
    sri.extract( sessionID );
    //call the facade method
    facade.releaseSession( sessionID );
    //write all temp data to the buffers
    swi->flush();
    //return
    return swi;
  }

  IByteBufferIterator*
  request_TransactionStartRO( ICoralFacade& facade, SegmentReaderIterator& sri, rowIteratorMap& )
  {
    SegmentWriterIterator* swi = new SegmentWriterIterator(CALOpcodes::StartTransactionRO, sri.cacheable(), true);
    //copy the proxy flag to the reply
    swi->setProxy( sri.proxy() );

    coral::Token sessionID;
    sri.extract( sessionID );
    //call the facade method
    facade.startTransaction( sessionID, true );
    //write all temp data to the buffers
    swi->flush();
    //return
    return swi;
  }

  IByteBufferIterator*
  request_TransactionStartRW( ICoralFacade& facade, SegmentReaderIterator& sri, rowIteratorMap& )
  {
    SegmentWriterIterator* swi = new SegmentWriterIterator(CALOpcodes::StartTransactionRW, sri.cacheable(), true);
    //copy the proxy flag to the reply
    swi->setProxy( sri.proxy() );

    coral::Token sessionID;
    sri.extract( sessionID );
    //call the facade method
    facade.startTransaction( sessionID, false );
    //write all temp data to the buffers
    swi->flush();
    //return
    return swi;
  }

  IByteBufferIterator*
  request_TransactionCommit( ICoralFacade& facade, SegmentReaderIterator& sri, rowIteratorMap& )
  {
    SegmentWriterIterator* swi = new SegmentWriterIterator(CALOpcodes::CommitTransaction, sri.cacheable(), true);
    //copy the proxy flag to the reply
    swi->setProxy( sri.proxy() );

    coral::Token sessionID;
    sri.extract( sessionID );
    //call the facade method
    facade.commitTransaction( sessionID );
    //write all temp data to the buffers
    swi->flush();
    //return
    return swi;
  }

  IByteBufferIterator*
  request_TransactionRollback( ICoralFacade& facade, SegmentReaderIterator& sri, rowIteratorMap& )
  {
    SegmentWriterIterator* swi = new SegmentWriterIterator(CALOpcodes::RollbackTransaction, sri.cacheable(), true);
    //copy the proxy flag to the reply
    swi->setProxy( sri.proxy() );

    coral::Token sessionID;
    sri.extract( sessionID );
    //call the facade method
    facade.rollbackTransaction( sessionID );
    //write all temp data to the buffers
    swi->flush();
    //return
    return swi;
  }

  IByteBufferIterator*
  request_FetchRows( ICoralFacade& facade, SegmentReaderIterator& sri, rowIteratorMap& mgri )
  {
    SegmentWriterIterator* swi = new SegmentWriterIterator(CALOpcodes::FetchRows, sri.cacheable(), true);
    //copy the proxy flag to the reply
    swi->setProxy( sri.proxy() );

    coral::Token sessionID;
    sri.extract( sessionID );

    QueryDefinition qd;
    sri.extract( qd );

    bool cacheSizeInMB;
    sri.extract( cacheSizeInMB );

    uint32_t cacheSize;
    sri.extract( cacheSize );
    //check if we have an empty rowbuffer
    AttributeList* pRowBuffer = NULL;

    bool isempty;
    sri.extract( isempty );
    //row buffer is empty
    if(isempty)
    {
      //create a new attributelist
      pRowBuffer = new AttributeList;
      //get the list
      sri.extractE( *pRowBuffer );
    }

    IRowIteratorPtr rowi = facade.fetchRows( sessionID, qd, pRowBuffer, cacheSize, cacheSizeInMB );

    uint32_t ntoken = 0;

    lockMutex(mgri.mutex);

    std::map<size_t, rowIteratorProperties*>::iterator i;

    do {
      ntoken++;
      i = mgri.maps.find(ntoken);
    } while(i != mgri.maps.end());

    rowIteratorProperties* rip = new rowIteratorProperties;
    rip->iterator = rowi.release();
    rip->inmb = cacheSizeInMB;
    rip->cachesize = cacheSize;

    mgri.maps.insert(std::pair<size_t, rowIteratorProperties*>(ntoken, rip));

    unlockMutex(mgri.mutex);
    //write the session id to the stream
    swi->append( ntoken );
    //write all temp data to the buffers
    swi->flush();
    //return
    return swi;
  }

  IByteBufferIterator*
  request_FetchRowsOT( ICoralFacade& facade, SegmentReaderIterator& sri, rowIteratorMap& mgri )
  {
    SegmentWriterIterator* swi = new SegmentWriterIterator(CALOpcodes::FetchRowsOT, sri.cacheable(), true);
    //copy the proxy flag to the reply
    swi->setProxy( sri.proxy() );

    coral::Token sessionID;
    sri.extract( sessionID );

    QueryDefinition qd;
    sri.extract( qd );

    bool cacheSizeInMB;
    sri.extract( cacheSizeInMB );

    uint32_t cacheSize;
    sri.extract( cacheSize );

    std::map< std::string, std::string > outputTypes;
    sri.extract( outputTypes );

    IRowIteratorPtr rowi = facade.fetchRows( sessionID, qd, outputTypes, cacheSize, cacheSizeInMB );

    uint32_t ntoken = 0;

    lockMutex(mgri.mutex);

    std::map<size_t, rowIteratorProperties*>::iterator i;

    do {
      ntoken++;
      i = mgri.maps.find(ntoken);
    } while(i != mgri.maps.end());

    rowIteratorProperties* rip = new rowIteratorProperties;
    rip->iterator = rowi.release();
    rip->inmb = cacheSizeInMB;
    rip->cachesize = cacheSize;

    mgri.maps.insert(std::pair<size_t, rowIteratorProperties*>(ntoken, rip));

    unlockMutex(mgri.mutex);
    //write the session id to the stream
    swi->append( ntoken );
    //write all temp data to the buffers
    swi->flush();
    //return
    return swi;
  }

  IByteBufferIterator*
  request_FetchRowsNext( ICoralFacade&, SegmentReaderIterator& sri, rowIteratorMap& mgri )
  {
    SegmentWriterIterator* swi = new SegmentWriterIterator(CALOpcodes::FetchRowsNext, sri.cacheable(), true);
    //copy the proxy flag to the reply
    swi->setProxy( sri.proxy() );

    uint32_t cursorID;
    sri.extract( cursorID );

    lockMutex(mgri.mutex);

    std::map<size_t, rowIteratorProperties*>::iterator i = mgri.maps.find(cursorID);
    if(i == mgri.maps.end()) {
      unlockMutex(mgri.mutex);
      throw StubsException("Can't find the RowIterator");
    }
    //get the iterator properties
    rowIteratorProperties* rip = i->second;
    //unlock the mutex
    unlockMutex(mgri.mutex);
    //get all rows and store them into the buffer iterator
    //FIXME better solution is to connect the byte buffer iterator
    //with the row iterator
    IRowIterator* rowi = rip->iterator;

    if(rip->inmb)
      //cache size in mb
    {
      size_t realsize = rip->cachesize * 1000000;
      bool hasnext = false;
      while(swi->written() < realsize)
      {
        hasnext = rowi->next();
        if(!hasnext) break;

        swi->append( true );

        swi->appendV( rowi->currentRow() );
      }
      swi->append( false );
      swi->append( !hasnext );
    }
    else
    {
      bool hasnext = false;
      size_t counter = 0;
      while(counter < rip->cachesize)
      {
        hasnext = rowi->next();
        if(!hasnext) break;

        swi->append( true );

        swi->appendV( rowi->currentRow() );

        counter++;
      }
      swi->append( false );
      swi->append( !hasnext );
    }
    //write all temp data to the buffers
    swi->flush();
    //return
    return swi;
  }

  IByteBufferIterator*
  request_ReleaseCursor( ICoralFacade&, SegmentReaderIterator& sri, rowIteratorMap& mgri )
  {
    SegmentWriterIterator* swi = new SegmentWriterIterator(CALOpcodes::ReleaseCursor, sri.cacheable(), true);
    //copy the proxy flag to the reply
    swi->setProxy( sri.proxy() );

    uint32_t cursorID;
    sri.extract( cursorID );

    lockMutex(mgri.mutex);

    std::map<size_t, rowIteratorProperties*>::iterator i = mgri.maps.find(cursorID);
    if(i == mgri.maps.end()) {
      unlockMutex(mgri.mutex);
      throw StubsException("Can't find the RowIterator");
    }
    rowIteratorProperties* rip = i->second;

    mgri.maps.erase( i );

    unlockMutex(mgri.mutex);

    delete rip->iterator;

    delete rip;
    //write all temp data to the buffers
    swi->flush();
    //return
    return swi;
  }

  IByteBufferIterator*
  request_FetchAllRows( ICoralFacade& facade, SegmentReaderIterator& sri, rowIteratorMap& )
  {
    /*
  SegmentWriterIterator* swi = new SegmentWriterIterator(CALOpcodes::FetchAllRows, sri.cacheable(), true);
  //copy the proxy flag to the reply
  swi->setProxy( sri.proxy() );
    */
    coral::Token sessionID;
    sri.extract( sessionID );

    QueryDefinition qd;
    sri.extract( qd );

    //create AttributeList
    AttributeList* rowBuffer = new AttributeList;
    bool isempty = true;

    bool hasbuffer;
    sri.extract( hasbuffer );
    //row buffer is empty
    if( hasbuffer )
      //row buffer is not empty
    {
      //get the list
      sri.extractE( *rowBuffer );

      isempty = rowBuffer->size() == 0;
    }

    IRowIteratorPtr rowptr;

    if(isempty)
      rowptr = facade.fetchAllRows( sessionID, qd, NULL );
    else
      rowptr = facade.fetchAllRows( sessionID, qd, rowBuffer );

    return new ByteBufferIteratorAll(rowptr.release(), CALOpcodes::FetchAllRows, sri.cacheable(), sri.proxy(), isempty, rowBuffer);
    /*

  if(isempty)
  {
    IRowIteratorPtr rowptr = facade.fetchAllRows( sessionID, qd, NULL );

    IRowIterator* rowi = rowptr.get();
    //get all rows and store them into the buffer iterator
    //FIXME better solution is to connect the byte buffer iterator
    //with the row iterator

    if(rowi->next())
    {
      swi->append( true );

      const AttributeList& attr01 = rowi->currentRow();

      swi->appendE( attr01 );

      void* structure = swi->getStructure( attr01 );

      //write only the raw data
      swi->appendD( structure );

      while(rowi->next())
      {
        const AttributeList& attr02 = rowi->currentRow();

        swi->append( true );
        //write only the raw data
        swi->appendD( structure, attr02 );
      }
      free( structure );
    }
    swi->append( false );
  }
  else
  {
    IRowIteratorPtr rowptr = facade.fetchAllRows( sessionID, qd, rowBuffer );

    IRowIterator* rowi = rowptr.get();
    //get all rows and store them into the buffer iterator
    //FIXME better solution is to connect the byte buffer iterator
    //with the row iterator
    void* structure = swi->getStructure( *rowBuffer );

    while(rowi->next())
    {
      swi->append( true );
      //write only the raw data
      swi->appendD( structure );
    }
    swi->append( false );
    free( structure );
  }
  delete rowBuffer;
  //write all temp data to the buffers
  swi->flush();
  //return
  return swi;
    */
  }

  IByteBufferIterator*
  request_FetchAllRowsOT( ICoralFacade& facade, SegmentReaderIterator& sri, rowIteratorMap& )
  {
    coral::Token sessionID;
    sri.extract( sessionID );
    //check if we have an empty rowbuffer
    QueryDefinition qd;
    sri.extract( qd );

    std::map< std::string, std::string > outputTypes;
    sri.extract( outputTypes );

    IRowIteratorPtr rowptr = facade.fetchAllRows( sessionID, qd, outputTypes );

    return new ByteBufferIteratorAll(rowptr.release(), CALOpcodes::FetchAllRowsOT, sri.cacheable(), sri.proxy(), true, 0);

    /*
  IRowIterator* rowi = rowptr.get();



  SegmentWriterIterator* swi = new SegmentWriterIterator(CALOpcodes::FetchAllRowsOT, sri.cacheable(), true);
  //copy the proxy flag to the reply
  swi->setProxy( sri.proxy() );

  coral::Token sessionID;
  sri.extract( sessionID );
  //check if we have an empty rowbuffer
  QueryDefinition qd;
  sri.extract( qd );

  std::map< std::string, std::string > outputTypes;
  sri.extract( outputTypes );

  IRowIteratorPtr rowptr = facade.fetchAllRows( sessionID, qd, outputTypes );

  IRowIterator* rowi = rowptr.get();
  //get all rows and store them into the buffer iterator
  //FIXME better solution is to connect the byte buffer iterator
  //with the row iterator
  if(rowi->next())
  {
    swi->append( true );

    const AttributeList& attr01 = rowi->currentRow();

    swi->appendE( attr01 );

    void* structure = swi->getStructure( attr01 );
    //write only the raw data
    swi->appendD( structure );

    while(rowi->next())
    {
      swi->append( true );

      const AttributeList& attr02 = rowi->currentRow();
      //write only the raw data
      swi->appendD( structure, attr02 );

    }
    free( structure );
  }

  swi->append( false );
  //write all temp data to the buffers
  swi->flush();
  //return
  return swi;
    */
  }

  IByteBufferIterator*
  request_ListTables( ICoralFacade& facade, SegmentReaderIterator& sri, rowIteratorMap& )
  {
    SegmentWriterIterator* swi = new SegmentWriterIterator(CALOpcodes::ListTables, sri.cacheable(), true);
    //copy the proxy flag to the reply
    swi->setProxy( sri.proxy() );

    coral::Token sessionID;
    sri.extract( sessionID );

    std::string schemaName;
    sri.extract16( schemaName );

    const std::set<std::string> myset = facade.listTables( sessionID, schemaName );

    logger << Debug << "request_ListTables: found [" << myset.size() << "] tables" << endlog;

    swi->append( myset );
    //write all temp data to the buffers
    swi->flush();
    //return
    return swi;
  }

  IByteBufferIterator*
  request_FetchTableDescription( ICoralFacade& facade, SegmentReaderIterator& sri, rowIteratorMap& )
  {
    SegmentWriterIterator* swi = new SegmentWriterIterator(CALOpcodes::FetchTableDescription, sri.cacheable(), true);
    //copy the proxy flag to the reply
    swi->setProxy( sri.proxy() );

    coral::Token sessionID;
    sri.extract( sessionID );

    std::string schemaName;
    sri.extract16( schemaName );

    std::string tableName;
    sri.extract16( tableName );

    const TableDescription& desc = facade.fetchTableDescription( sessionID, schemaName, tableName );

    swi->append( desc );
    //write all temp data to the buffers
    swi->flush();
    //return
    return swi;
  }

  IByteBufferIterator*
  request_TableExists( ICoralFacade& facade, SegmentReaderIterator& sri, rowIteratorMap& )
  {
    SegmentWriterIterator* swi = new SegmentWriterIterator(CALOpcodes::TableExists, sri.cacheable(), true);
    //copy the proxy flag to the reply
    swi->setProxy( sri.proxy() );

    coral::Token sessionID;
    sri.extract( sessionID );

    std::string schemaName;
    sri.extract16( schemaName );

    std::string tableName;
    sri.extract16( tableName );

    swi->append( facade.existsTable( sessionID, schemaName, tableName ) );
    //write all temp data to the buffers
    swi->flush();
    //return
    return swi;
  }

  IByteBufferIterator*
  request_ListViews( ICoralFacade& facade, SegmentReaderIterator& sri, rowIteratorMap& )
  {
    SegmentWriterIterator* swi = new SegmentWriterIterator(CALOpcodes::ListViews, sri.cacheable(), true);
    //copy the proxy flag to the reply
    swi->setProxy( sri.proxy() );

    coral::Token sessionID;
    sri.extract( sessionID );

    std::string schemaName;
    sri.extract16( schemaName );

    const std::set<std::string> myset = facade.listViews( sessionID, schemaName );

    logger << Debug << "request_ListViews: found [" << myset.size() << "] tables" << endlog;

    swi->append( myset );
    //write all temp data to the buffers
    swi->flush();
    //return
    return swi;
  }

  IByteBufferIterator*
  request_FetchViewDescription( ICoralFacade& facade, SegmentReaderIterator& sri, rowIteratorMap& )
  {
    SegmentWriterIterator* swi = new SegmentWriterIterator(CALOpcodes::FetchViewDescription, sri.cacheable(), true);
    //copy the proxy flag to the reply
    swi->setProxy( sri.proxy() );

    coral::Token sessionID;
    sri.extract( sessionID );

    std::string schemaName;
    sri.extract16( schemaName );

    std::string viewName;
    sri.extract16( viewName );

    const std::pair<TableDescription,std::string>& desc = facade.fetchViewDescription( sessionID, schemaName, viewName );

    swi->append( desc.first );
    swi->append16( desc.second );
    //write all temp data to the buffers
    swi->flush();
    //return
    return swi;
  }

  IByteBufferIterator*
  request_ViewExists( ICoralFacade& facade, SegmentReaderIterator& sri, rowIteratorMap& )
  {
    SegmentWriterIterator* swi = new SegmentWriterIterator(CALOpcodes::ViewExists, sri.cacheable(), true);
    //copy the proxy flag to the reply
    swi->setProxy( sri.proxy() );

    coral::Token sessionID;
    sri.extract( sessionID );

    std::string schemaName;
    sri.extract16( schemaName );

    std::string viewName;
    sri.extract16( viewName );

    swi->append( facade.existsView( sessionID, schemaName, viewName ) );
    //write all temp data to the buffers
    swi->flush();
    //return
    return swi;
  }

  IByteBufferIterator*
  request_FetchSessionProperties( ICoralFacade& facade, SegmentReaderIterator& sri, rowIteratorMap& )
  {
    SegmentWriterIterator* swi = new SegmentWriterIterator(CALOpcodes::FetchSessionProperties, sri.cacheable(), true);
    //copy the proxy flag to the reply
    swi->setProxy( sri.proxy() );

    coral::Token sessionID;
    sri.extract( sessionID );

    const std::vector<std::string>& sessionp = facade.fetchSessionProperties( sessionID );

    swi->append( sessionp );
    //write all temp data to the buffers
    swi->flush();
    //return
    return swi;
  }

  typedef IByteBufferIterator* (*request_t)( ICoralFacade& facade, SegmentReaderIterator&, rowIteratorMap& );

  static StatsTypePayload myStatsTypePayload_0x01("0x01","ConnectRO");
  static StatsTypePayload myStatsTypePayload_0x02("0x02","ConnectRW");
  static StatsTypePayload myStatsTypePayload_0x03("0x03","Disconnect");
  static StatsTypePayload myStatsTypePayload_0x04("0x04","TransactionStartRo");
  static StatsTypePayload myStatsTypePayload_0x05("0x05","TransactionStartRW");
  static StatsTypePayload myStatsTypePayload_0x06("0x06","TransactionCommit");
  static StatsTypePayload myStatsTypePayload_0x07("0x07","TransactionRollback");
  static StatsTypePayload myStatsTypePayload_0x10("0x10","FetchRows");
  static StatsTypePayload myStatsTypePayload_0x11("0x11","FetchRowsNext");
  static StatsTypePayload myStatsTypePayload_0x12("0x12","ReleaseCursor");
  static StatsTypePayload myStatsTypePayload_0x13("0x13","FetchAllRows");
  static StatsTypePayload myStatsTypePayload_0x14("0x14","ListTables");
  static StatsTypePayload myStatsTypePayload_0x15("0x15","FetchTableDescription");
  static StatsTypePayload myStatsTypePayload_0x16("0x16","TableExists");
  static StatsTypePayload myStatsTypePayload_0x17("0x17","FetchSessionProperties");
  static StatsTypePayload myStatsTypePayload_0x18("0x18","FetchRowsOT");
  static StatsTypePayload myStatsTypePayload_0x19("0x19","FetchAllRowsOT");
  static StatsTypePayload myStatsTypePayload_0x1a("0x1a","ListViews");
  static StatsTypePayload myStatsTypePayload_0x1b("0x1b","FetchViewDescription");
  static StatsTypePayload myStatsTypePayload_0x1c("0x1c","ViewExists");

  static StatsTypePayload* monitor_matrix[29] = {
    NULL,
    &myStatsTypePayload_0x01,
    &myStatsTypePayload_0x02,
    &myStatsTypePayload_0x03,
    &myStatsTypePayload_0x04,
    &myStatsTypePayload_0x05,
    &myStatsTypePayload_0x06,
    &myStatsTypePayload_0x07,
    NULL, //0x08
    NULL, //0x09
    NULL, //0x0a
    NULL, //0x0b
    NULL, //0x0c
    NULL, //0x0d
    NULL, //0x0e
    NULL, //0x0f
    &myStatsTypePayload_0x10,
    &myStatsTypePayload_0x11,
    &myStatsTypePayload_0x12,
    &myStatsTypePayload_0x13,
    &myStatsTypePayload_0x14,
    &myStatsTypePayload_0x15,
    &myStatsTypePayload_0x16,
    &myStatsTypePayload_0x17,
    &myStatsTypePayload_0x18,
    &myStatsTypePayload_0x19,
    &myStatsTypePayload_0x1a,
    &myStatsTypePayload_0x1b,
    &myStatsTypePayload_0x1c
  };
  //the array for calling the function by the opcode
  static request_t request_matrix[29] = {
    NULL, //0x00
    &request_ConnectRO, //0x01
    &request_ConnectRW, //0x02
    &request_Disconnect, //0x03
    &request_TransactionStartRO, //0x04
    &request_TransactionStartRW, //0x05
    &request_TransactionCommit, //0x06
    &request_TransactionRollback, //0x07
    NULL, //0x08
    NULL, //0x09
    NULL, //0x0a
    NULL, //0x0b
    NULL, //0x0c
    NULL, //0x0d
    NULL, //0x0e
    NULL, //0x0f
    &request_FetchRows, //0x10
    &request_FetchRowsNext, //0x11
    &request_ReleaseCursor, //0x12
    &request_FetchAllRows, //0x13
    &request_ListTables, //0x14
    &request_FetchTableDescription, //0x15
    &request_TableExists, //0x16
    &request_FetchSessionProperties, //0x17
    &request_FetchRowsOT, //0x18
    &request_FetchAllRowsOT, //0x19
    &request_ListViews, //0x1a
    &request_FetchViewDescription, //0x1b
    &request_ViewExists //0x1c

  };

  //the array to determine which call is available, go from 0 to 255
  static bool request_table[255] = {
    false, //0x00
    true, //0x01
    true, //0x02
    true, //0x03
    true, //0x04
    true, //0x05
    true, //0x06
    true, //0x07
    false, //0x08
    false, //0x09
    false, //0x0a
    false, //0x0b
    false, //0x0c
    false, //0x0d
    false, //0x0e
    false, //0x0f
    true , //0x10
    true , //0x11
    true , //0x12
    true , //0x13
    true , //0x14
    true , //0x15
    true , //0x16
    true , //0x17
    true , //0x18
    true , //0x19
    true , //0x1a
    true , //0x1b
    true , //0x1c
    false, //0x1d
    false, //0x1e
    false //0x1f
  };

  void
  prepareCoralException( SegmentWriterIterator* swi, coral::Exception& e )
  {
    //copy the exception string to a local string
    std::string ex(e.what());
    //find the first position
    size_t pos01 = ex.find("( CORAL");
    //create a substring until the end of the exception string
    std::string ez(ex.substr(pos01));
    //find the second position
    size_t pos02 = ez.find(" from ");
    //create the original three parts
    std::string part01(ex.substr(0, pos01 - 1));
    std::string part02(ez.substr(11, pos02 - 12));
    std::string part03(ez.substr(pos02 + 7, ez.size() - pos02 - 10));
    //write the encoding for exceptions to the buffer
    swi->exception(0x00, 0x02, part01, part02, part03);
  }

  ServerStub::ServerStub(ICoralFacade& coralFacade)
    : m_facade( coralFacade )
    , m_rowimap()
  {
    //logger << "Create ServerStub" << endlog;
    m_rowimap = new rowIteratorMap;

    m_rowimap->mutex = new pmutex_t;

    m_rowimap->m_fall_ops = 0;

    initMutex(m_rowimap->mutex);
  }

  ServerStub::~ServerStub()
  {
    //logger << "Delete ServerStub..." << endlog;
    //logger << "Delete ServerStub... 1" << endlog;
    std::map<size_t, rowIteratorProperties*>::iterator i;
    for(i = m_rowimap->maps.begin(); i != m_rowimap->maps.end(); i++ )
    {
      rowIteratorProperties* rip = i->second;

      delete rip->iterator;

      delete rip;
    }
    //logger << "Delete ServerStub... 2" << endlog;

    delete m_rowimap->mutex;
    //logger << "Delete ServerStub... 3" << endlog;
    delete m_rowimap;
    //logger << "Delete ServerStub... DONE" << endlog;
  }

  void ServerStub::setCertificateData( const coral::ICertificateData* cert )
  {
    m_facade.setCertificateData( cert );
  }

  IByteBufferIteratorPtr
  ServerStub::replyToRequest( const ByteBuffer& requestBuffer )
  {
    //create a simple bytebuffer iterator for holding the request buffer
    //we need that kind of transformation from byte buffer
    //to byte buffer iterator
    SimpleByteBufferIterator sbi( requestBuffer );
    //create the reader
    SegmentReaderIterator sri( 0, sbi );
    // Workaround for catching the CAL protocol exception
    // BUG #64373
    try
    {
      //extract header by calling empty method
      sri.noextract();
    }
    catch( StreamBufferException& e )
    {
      // Log local what happened
      logger << Error << "Caught coral::Exception: '" << e.what() << "'" << endlog;

      SegmentWriterIterator* swi = new SegmentWriterIterator(sri.opcode(), sri.cacheable(), true);

      // Copy the proxy flag to the reply
      swi->setProxy( sri.proxy() );

      prepareCoralException( swi, e );

      return std::auto_ptr<IByteBufferIterator>( swi );
    }
    //get the opcode
    CALOpcode opcode = sri.opcode();

    if(!request_table[opcode])
    {
      SegmentWriterIterator* swi = new SegmentWriterIterator(opcode, sri.cacheable(), true);
      //copy the proxy flag to the reply
      swi->setProxy( sri.proxy() );

      std::ostringstream s;
      s << "Message with opcode (" << hexstring(&opcode,1) << ") can't be handled";

      swi->exception(0x00, 0x10, s.str(), "ServerStub::replyToRequest", "");

      return std::auto_ptr<IByteBufferIterator>( swi );
    }
    //execute the facade implementation
    try
    {
      StatsTypePayload* mon = monitor_matrix[opcode];
      StopTimer execTimer;
      execTimer.start();
      //execute
      IByteBufferIterator* iter = request_matrix[opcode](m_facade, sri, *m_rowimap);

      execTimer.stop();
      mon->add(execTimer.getUserTime(),execTimer.getSystemTime(),execTimer.getRealTime());

      return std::auto_ptr<IByteBufferIterator>( iter );
    }
    catch ( coral::Exception& e )
    {
      //log local what happened
      logger << Error << "Caught coral::Exception: '" << e.what() << "'" << endlog;

      SegmentWriterIterator* swi = new SegmentWriterIterator(opcode, sri.cacheable(), true);

      //copy the proxy flag to the reply
      swi->setProxy( sri.proxy() );

      prepareCoralException( swi, e );

      return std::auto_ptr<IByteBufferIterator>( swi );
    }
    catch ( std::exception& e )
    {
      logger << Error << "Caught std::exception: '" << e.what() << "'" << endlog;
      //create new exception message
      SegmentWriterIterator* swi = new SegmentWriterIterator(opcode, sri.cacheable(), true);

      //copy the proxy flag to the reply
      swi->setProxy( sri.proxy() );

      swi->exception(0x00, 0x01, e.what(), "ServerStub::replyToRequest", "");

      return std::auto_ptr<IByteBufferIterator>( swi );
    }
    catch (...)
    {
      //create new exception message
      logger << Error << "Caught unknown exception" << endlog;
      //create new exception message
      SegmentWriterIterator* swi = new SegmentWriterIterator(opcode, sri.cacheable(), true);

      //copy the proxy flag to the reply
      swi->setProxy( sri.proxy() );

      swi->exception(0x00, 0x00, "Unkown exception caught", "ServerStub::replyToRequest", "");

      return std::auto_ptr<IByteBufferIterator>( swi );
    }
  }


} }
