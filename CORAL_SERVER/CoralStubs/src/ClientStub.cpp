#include "CoralStubs/ClientStub.h"

#include "CoralMonitor/StopTimer.h"

//CoralStubs includes
#include "RowIteratorAll.h"
#include "RowIteratorFetch.h"
#include "CALProtocol.h"
#include "Exceptions.h"
#include "CppTypes.h"
#include "SegmentWriterIterator.h"
#include "SegmentReaderIterator.h"

//CoralServerBase includes
#include "CoralServerBase/IRequestHandler.h"

//C++ includes
#include <iostream>
#include <sstream>

#define LOGGER_NAME "CoralStubs::ClientStub"
#include "logger.h"

namespace coral { namespace CoralStubs {

  ClientStub::ClientStub(IRequestHandler& requestHandler)
    : m_requestHandler( requestHandler )
  {
  }

  ClientStub::~ClientStub()
  {
  }

  void
  ClientStub::setCertificateData( const coral::ICertificateData* /*cert*/ )
  {
    // DUMMY ???
  }

  coral::Token
  ClientStub::connect( const std::string& dbUrl, const coral::AccessMode mode, bool& fromProxy ) const
  {
    //ScopedTimer timer("CoralStubs::ClientStup::connect()");
    coral::CALOpcode opcode = mode == coral::ReadOnly ? coral::CALOpcodes::ConnectRO : coral::CALOpcodes::ConnectRW;
    //create the request buffer object as cacheable and no reply
    SegmentWriterIterator swi(opcode, true, false);

    swi.append16( dbUrl );

    //call the request handler api using the single buffer
    coral::IByteBufferIteratorPtr reply = m_requestHandler.replyToRequest( swi.singleBuffer() );
    //create the response buffer object
    SegmentReaderIterator sri(opcode, *(reply.get()) );

    Token mytoken;

    sri.extract(mytoken);

    fromProxy = sri.proxy();

    return mytoken;
  }

  void
  ClientStub::releaseSession( coral::Token sessionID ) const
  {
    //ScopedTimer timer("CoralStubs::ClientStup::releaseSession()");
    //create the request buffer object as no cacheable and no reply
    SegmentWriterIterator swi(coral::CALOpcodes::Disconnect, false, false);

    swi.append( sessionID );

    //call the request handler api using the single buffer
    coral::IByteBufferIteratorPtr reply = m_requestHandler.replyToRequest( swi.singleBuffer() );
    //create the response buffer object
    SegmentReaderIterator sri(coral::CALOpcodes::Disconnect, *(reply.get()) );
    //use empty instead of noextract to prevent
    //exception in the case of empty content
    sri.empty();
  }

  void
  ClientStub::startTransaction( coral::Token sessionID, bool readOnly ) const
  {
    //ScopedTimer timer("CoralStubs::ClientStup::startTransaction()");
    coral::CALOpcode opcode = readOnly ? coral::CALOpcodes::StartTransactionRO : coral::CALOpcodes::StartTransactionRW;
    //create the request buffer object as no cacheable and no reply
    SegmentWriterIterator swi(opcode, false, false);

    swi.append( sessionID );

    //call the request handler api using the single buffer
    coral::IByteBufferIteratorPtr reply = m_requestHandler.replyToRequest( swi.singleBuffer() );
    //create the response buffer object
    SegmentReaderIterator sri(opcode, *(reply.get()) );
    //use empty instead of noextract to prevent
    //exception in the case of empty content
    sri.empty();
  }

  void
  ClientStub::commitTransaction( coral::Token sessionID ) const
  {
    //ScopedTimer timer("CoralStubs::ClientStup::commitTransaction()");
    //create the request buffer object as no cacheable and no reply
    SegmentWriterIterator swi(coral::CALOpcodes::CommitTransaction, false, false);

    swi.append( sessionID );

    //call the request handler api using the single buffer
    coral::IByteBufferIteratorPtr reply = m_requestHandler.replyToRequest( swi.singleBuffer() );
    //create the response buffer object
    SegmentReaderIterator sri(coral::CALOpcodes::CommitTransaction, *(reply.get()) );
    //use empty instead of noextract to prevent
    //exception in the case of empty content
    sri.empty();
  }

  void
  ClientStub::rollbackTransaction( coral::Token sessionID ) const
  {
    //ScopedTimer timer("CoralStubs::ClientStup::rollbackTransaction()");
    //create the request buffer object as no cacheable and no reply
    SegmentWriterIterator swi(coral::CALOpcodes::RollbackTransaction, false, false);

    swi.append( sessionID );

    //call the request handler api using the single buffer
    coral::IByteBufferIteratorPtr reply = m_requestHandler.replyToRequest( swi.singleBuffer() );
    //create the response buffer object
    SegmentReaderIterator sri(coral::CALOpcodes::RollbackTransaction, *(reply.get()) );
    //use empty instead of noextract to prevent
    //exception in the case of empty content
    sri.empty();
  }

  const std::vector<std::string>
  ClientStub::fetchSessionProperties( Token sessionID ) const
  {
    //ScopedTimer timer("CoralStubs::ClientStup::sessionProperties()");
    //create the request buffer object as cacheable and no reply
    SegmentWriterIterator swi( coral::CALOpcodes::FetchSessionProperties, true, false );

    swi.append( sessionID );

    //call the request handler api using the single buffer
    coral::IByteBufferIteratorPtr reply = m_requestHandler.replyToRequest( swi.singleBuffer() );
    //create the response buffer object
    SegmentReaderIterator sri( coral::CALOpcodes::FetchSessionProperties, *(reply.get()) );
    //create a new set
    std::vector<std::string> ret;

    sri.extract( ret );

    return ret;
  }

  IRowIteratorPtr
  ClientStub::fetchRows( Token sessionID, const QueryDefinition& qd, AttributeList* pRowBuffer, size_t cacheSize, bool cacheSizeInMB ) const
  {
    //ScopedTimer timer("CoralStubs::ClientStup::fetchRows()");
    //create the request buffer object as cacheable and no reply
    SegmentWriterIterator swi( coral::CALOpcodes::FetchRows, true, false );

    swi.append( sessionID );
    swi.append( qd );
    swi.append( cacheSizeInMB );
    swi.append( (uint32_t)cacheSize );
    //check if we have an empty AttributeList
    if(pRowBuffer)
    {
      swi.append(true);
      swi.appendE(*pRowBuffer);
    }
    else
    {
      swi.append(false);
    }
    //call the request handler api using the single buffer
    coral::IByteBufferIteratorPtr reply = m_requestHandler.replyToRequest( swi.singleBuffer() );
    //create the response buffer object
    SegmentReaderIterator sri( coral::CALOpcodes::FetchRows, *(reply.get()) );
    //create a new set
    uint32_t cursorID;

    sri.extract( cursorID );

    RowIteratorFetch* tfetch = new RowIteratorFetch(m_requestHandler, cursorID, cacheSize, pRowBuffer);
    //return the iterator packed into a smart pointer
    return std::unique_ptr<IRowIterator>(tfetch);
  }

  IRowIteratorPtr
  ClientStub::fetchRows( Token sessionID, const QueryDefinition& qd, const std::map< std::string, std::string > outputTypes, size_t cacheSize, bool cacheSizeInMB ) const
  {
    logger << Debug << "fetchRows" << endlog;
    //ScopedTimer timer("CoralStubs::ClientStup::fetchRows() outputTypes");
    //create the request buffer object as cacheable and no reply
    SegmentWriterIterator swi( coral::CALOpcodes::FetchRowsOT, true, false );

    swi.append( sessionID );
    swi.append( qd );
    swi.append( cacheSizeInMB );
    swi.append( (uint32_t)cacheSize );
    swi.append( outputTypes );
    //call the request handler api using the single buffer
    coral::IByteBufferIteratorPtr reply = m_requestHandler.replyToRequest( swi.singleBuffer() );
    //create the response buffer object
    SegmentReaderIterator sri( coral::CALOpcodes::FetchRowsOT, *(reply.get()) );
    //create a new set
    uint32_t cursorID;

    sri.extract( cursorID );

    RowIteratorFetch* tfetch = new RowIteratorFetch(m_requestHandler, cursorID, cacheSize, NULL);
    //return the iterator packed into a smart pointer
    return std::unique_ptr<IRowIterator>(tfetch);
  }

  IRowIteratorPtr
  ClientStub::fetchAllRows( Token sessionID, const QueryDefinition& qd, AttributeList* rowBuffer ) const
  {
    //ScopedTimer timer("CoralStubs::ClientStup::fetchAllRows()");
    //create the request buffer object as cacheable and no reply
    SegmentWriterIterator swi( coral::CALOpcodes::FetchAllRows, true, false );

    swi.append( sessionID );

    swi.append( qd );

    if(rowBuffer)
    {
      logger << Debug << "fetchAllRows with rowBuffer" << endlog;

      if(rowBuffer->size() > 0)
      {
        swi.append( true );
        swi.appendE( *rowBuffer );
      }
      else
        swi.append( false );
    }
    else
    {
      logger << Debug << "fetchAllRows with NONE rowBuffer" << endlog;
      swi.append( false );
    }
    //call the request handler api using the single buffer
    coral::IByteBufferIteratorPtr reply = m_requestHandler.replyToRequest( swi.singleBuffer() );

    RowIteratorAll* tall = new RowIteratorAll( reply.release(), rowBuffer, coral::CALOpcodes::FetchAllRows );

    return std::unique_ptr<IRowIterator>(tall);
  }

  IRowIteratorPtr
  ClientStub::fetchAllRows( Token sessionID, const QueryDefinition& qd, const std::map< std::string, std::string > outputTypes ) const
  {
    //ScopedTimer timer("CoralStubs::ClientStup::fetchAllRows() outputTypes");
    //create the request buffer object as cacheable and no reply
    SegmentWriterIterator swi( coral::CALOpcodes::FetchAllRowsOT, true, false );

    swi.append( sessionID );
    swi.append( qd );
    swi.append( outputTypes );
    //call the request handler api using the single buffer
    coral::IByteBufferIteratorPtr reply = m_requestHandler.replyToRequest( swi.singleBuffer() );

    RowIteratorAll* tall = new RowIteratorAll( reply.release(), NULL, coral::CALOpcodes::FetchAllRowsOT );

    return std::unique_ptr<IRowIterator>(tall);
  }

  const std::set<std::string>
  ClientStub::listTables( Token sessionID, const std::string& schemaName ) const
  {
    //ScopedTimer timer("CoralStubs::ClientStup::listTables()");
    //create the request buffer object as cacheable and no reply
    SegmentWriterIterator swi( coral::CALOpcodes::ListTables, true, false );

    swi.append( sessionID );
    swi.append16( schemaName );
    //call the request handler api using the single buffer
    coral::IByteBufferIteratorPtr reply = m_requestHandler.replyToRequest( swi.singleBuffer() );
    //create the response buffer object
    SegmentReaderIterator sri( coral::CALOpcodes::ListTables, *(reply.get()) );
    //create a new set
    std::set<std::string> myset;

    sri.extract( myset );
    //return the set
    return myset;
  }

  bool ClientStub::existsTable( Token sessionID, const std::string& schemaName, const std::string& tableName ) const
  {
    //create the request buffer object as cacheable and no reply
    SegmentWriterIterator swi( coral::CALOpcodes::TableExists, true, false );

    swi.append( sessionID );
    swi.append16( schemaName );
    swi.append16( tableName );
    //call the request handler api using the single buffer
    coral::IByteBufferIteratorPtr reply = m_requestHandler.replyToRequest( swi.singleBuffer() );
    //create the response buffer object
    SegmentReaderIterator sri( coral::CALOpcodes::TableExists, *(reply.get()) );
    //create a new set
    bool exists;

    sri.extract( exists );
    //return the set
    return exists;
  }

  const TableDescription
  ClientStub::fetchTableDescription( Token sessionID, const std::string& schemaName, const std::string& tableName ) const
  {
    //ScopedTimer timer("CoralStubs::ClientStup::fetchTableDescription()");
    //create the request buffer object as cacheable and no reply
    SegmentWriterIterator swi( coral::CALOpcodes::FetchTableDescription, true, false );

    swi.append( sessionID );
    swi.append16( schemaName );
    swi.append16( tableName );
    //call the request handler api using the single buffer
    coral::IByteBufferIteratorPtr reply = m_requestHandler.replyToRequest( swi.singleBuffer() );
    //create the response buffer object
    SegmentReaderIterator sri( coral::CALOpcodes::FetchTableDescription, *(reply.get()) );
    //create a new set
    TableDescription td;

    sri.extract( td );
    //return the set
    return td;
  }

  const std::set<std::string>
  ClientStub::listViews( Token sessionID, const std::string& schemaName ) const
  {
    //ScopedTimer timer("CoralStubs::ClientStup::listViews()");
    //create the request buffer object as cacheable and no reply
    SegmentWriterIterator swi( coral::CALOpcodes::ListViews, true, false );

    swi.append( sessionID );
    swi.append16( schemaName );
    //call the request handler api using the single buffer
    coral::IByteBufferIteratorPtr reply = m_requestHandler.replyToRequest( swi.singleBuffer() );
    //create the response buffer object
    SegmentReaderIterator sri( coral::CALOpcodes::ListViews, *(reply.get()) );
    //create a new set
    std::set<std::string> myset;

    sri.extract( myset );
    //return the set
    return myset;
  }

  bool ClientStub::existsView( Token sessionID, const std::string& schemaName, const std::string& viewName ) const
  {
    //create the request buffer object as cacheable and no reply
    SegmentWriterIterator swi( coral::CALOpcodes::ViewExists, true, false );

    swi.append( sessionID );
    swi.append16( schemaName );
    swi.append16( viewName );
    //call the request handler api using the single buffer
    coral::IByteBufferIteratorPtr reply = m_requestHandler.replyToRequest( swi.singleBuffer() );
    //create the response buffer object
    SegmentReaderIterator sri( coral::CALOpcodes::ViewExists, *(reply.get()) );
    //create a new set
    bool exists;

    sri.extract( exists );
    //return the set
    return exists;
  }

  const std::pair< TableDescription , std::string >
  ClientStub::fetchViewDescription( Token sessionID, const std::string& schemaName, const std::string& viewName ) const
  {
    //ScopedTimer timer("CoralStubs::ClientStup::fetchViewDescription()");
    //create the request buffer object as cacheable and no reply
    SegmentWriterIterator swi( coral::CALOpcodes::FetchViewDescription, true, false );

    swi.append( sessionID );
    swi.append16( schemaName );
    swi.append16( viewName );
    //call the request handler api using the single buffer
    coral::IByteBufferIteratorPtr reply = m_requestHandler.replyToRequest( swi.singleBuffer() );
    //create the response buffer object
    SegmentReaderIterator sri( coral::CALOpcodes::FetchViewDescription, *(reply.get()) );
    //create a new set
    TableDescription td;

    sri.extract( td );

    std::string alias;

    sri.extract16( alias );
    //return the set
    return std::pair< TableDescription , std::string >(td, alias);
  }


  int
  ClientStub::deleteTableRows( Token /*sessionID*/, const std::string& /*schemaName*/, const std::string& /*tableName*/, const std::string& /*whereClause*/, const std::string& /*whereData*/ ) const
                    {
                      throw StubsException("ClientStub::deleteTableRows, This function is not available");
                    }

                    const std::string
                    ClientStub::formatRowBufferAsString( Token /*sessionID*/, const std::string& /*schemaName*/, const std::string& /*tableName*/ ) const
                    {
                      throw StubsException("ClientStub::formatRowBufferAsString, This function is not available");
                    }

                    void
                    ClientStub::insertRowAsString( Token /*sessionID*/, const std::string& /*schemaName*/, const std::string& /*tableName*/, const std::string& /*rowBufferAS*/ ) const
                    {
                      throw StubsException("ClientStub::insertRowAsString, This function is not available");
                    }

                    Token
                    ClientStub::bulkInsertAsString( Token /*sessionID*/, const std::string& /*schemaName*/, const std::string& /*tableName*/, const std::string& /*rowBufferAS*/, int /*rowCacheSizeDb*/ ) const
                    {
                      throw StubsException("ClientStub::bulkInsertAsString, This function is not available");
                    }

                    void
                    ClientStub::releaseBulkOp( Token /*bulkOpID*/ ) const
                    {
                      throw StubsException("ClientStub::releaseBulkOp, This function is not available");
                    }

                    void
                    ClientStub::processRows( Token /*bulkOpID*/, const std::vector<coral::AttributeList>& /*rowsAS*/ ) const
                    {
                      throw StubsException("ClientStub::processRows, This function is not available");
                    }

                    void
                    ClientStub::flush( Token /*bulkOpID*/ ) const
                    {
                      throw StubsException("ClientStub::flush, This function is not available");
                    }

                    void
                    ClientStub::callProcedure( Token /*sessionID*/, const std::string& /*schemaName*/, const std::string& /*procedureName*/, const coral::AttributeList& /*inputArguments*/ ) const
                    {
                      throw StubsException("ClientStub::callProcedure, This function is not available");
                    }

                  } }
