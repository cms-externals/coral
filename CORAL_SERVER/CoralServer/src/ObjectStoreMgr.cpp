// $Id: ObjectStoreMgr.cpp,v 1.7.2.1 2010/12/20 09:10:10 avalassi Exp $

// Include files
#include <sstream>
#include "CoralServer/TokenGenerator.h"
#include "RelationalAccess/IBulkOperation.h"

// Local include files
#include "ObjectStoreMgr.h"
#include "QueryAndCursor.h"
#include "SessionAndFlag.h"

// Logger
#define LOGGER_NAME "CoralServer::ObjectStoreMgr"
#include "logger.h"

// Namespace
using namespace coral::CoralServer;

//-----------------------------------------------------------------------------

ObjectStoreMgr::ObjectStoreMgr( ITokenPool* tokenPool )
  : m_tokenPool( tokenPool ? tokenPool : new TokenGenerator() )
  , m_ownTokenPool( tokenPool ? false : true )
  , m_dict()
  , m_children()
  , m_parent()
  , m_mutex()
{
  logger << Debug << "Create ObjectStoreMgr" << endlog;
}

//-----------------------------------------------------------------------------

ObjectStoreMgr::~ObjectStoreMgr()
{
  logger << Debug << "Delete ObjectStoreMgr" << endlog;
  std::map< Token, ObjectAndType >::iterator objIt;
  for ( objIt = m_dict.begin(); objIt != m_dict.end(); objIt++ )
  {
    Token ID = objIt->first;
    logger << Warning << "Deleting unreleased object #" << ID << endlog;
    deleteObject( objIt->second, false );
  }
  if ( m_ownTokenPool ) delete m_tokenPool;
}

//-----------------------------------------------------------------------------

coral::Token ObjectStoreMgr::registerObject( ObjectAndType obj,
                                             coral::Token parentID )
{
  boost::mutex::scoped_lock lock( m_mutex );
  // Check parentID
  if ( parentID != 0 && m_dict.find( parentID ) == m_dict.end() )
    throw Exception( "Cannot register object: parentID is unknown",
                     "ObjectStoreMgr::registerObject",
                     "coral::CoralServer" );
  // Register the object
  Token ID = m_tokenPool->allocateToken();
  m_dict[ ID ] = obj;
  // Update the list of children of its parent
  if ( parentID != 0 )
  {
    m_parent[ ID ] = parentID;
    m_children[parentID].insert( ID );
  }
  return ID;
}

//-----------------------------------------------------------------------------

void ObjectStoreMgr::releaseObject( coral::Token ID )
{
  boost::mutex::scoped_lock lock( m_mutex );
  i_releaseObject( ID, true );
}

//-----------------------------------------------------------------------------

void ObjectStoreMgr::i_releaseObject( coral::Token ID, bool erase )
{
  // Unregister the object from the list of children of its parent
  if ( m_parent.find( ID ) != m_parent.end() )
  {
    m_children[ m_parent[ ID ] ].erase( ID );
    m_parent.erase( ID );
  }
  // Release and delete all children
  else if ( m_children.find( ID ) != m_children.end() )
  {
    std::set<coral::Token> children = m_children[ ID ]; // copy
    std::set<coral::Token>::iterator child;
    for ( child = children.begin(); child != children.end(); child++ )
    {
      logger << Warning << "Deleting unreleased child #" << *child << " of parent #" << ID << endlog;
      i_releaseObject( *child, false ); // do not erase... the user may release it later!
    }
  }
  // Delete the object
  ObjectAndType obj = i_findObject( ID );
  if ( erase ) m_dict.erase( ID );
  else
  {
    m_dict[ID].first = 0;
  }
  bool throwIfNull = !erase; // do not throw on user calls (erase=true)
  deleteObject( obj, throwIfNull );
  // Release the token
  m_tokenPool->releaseToken( ID );
}

//-----------------------------------------------------------------------------

IObjectStoreMgr::ObjectAndType ObjectStoreMgr::findObject( coral::Token ID )
{
  boost::mutex::scoped_lock lock( m_mutex );
  return i_findObject( ID );
}

//-----------------------------------------------------------------------------

IObjectStoreMgr::ObjectAndType ObjectStoreMgr::i_findObject( coral::Token ID )
{
  if ( m_dict.find( ID ) != m_dict.end() )
  {
    return m_dict[ID];
  }
  else
  {
    std::stringstream msg;
    msg << "No object found with ID=" << ID;
    throw Exception( msg.str(), "ObjectStoreMgr::findObject", "coral::CoralServer" );
  }
}

//-----------------------------------------------------------------------------

void ObjectStoreMgr::deleteObject( ObjectAndType obj, bool throwIfNull )
{
  void* pObj = obj.first;
  if ( !pObj )
  {
    if ( !throwIfNull ) return;
    throw Exception( "PANIC! Attempt to delete null object",
                     "ObjectStoreMgr::deleteObject",
                     "coral::CoralServer" );
  }
  const std::type_info& type = *(obj.second);
  if ( type == typeid( IBulkOperation ) )
    deleteObject<IBulkOperation>( pObj );
  else if ( type == typeid( QueryAndCursor ) )
    deleteObject<QueryAndCursor>( pObj );
  else if ( type == typeid( SessionAndFlag ) )
    deleteObject<SessionAndFlag>( pObj );
  else
    throw Exception( "PANIC! Unsupported type!",
                     "ObjectStoreMgr::deleteObject",
                     "coral::CoralServer" );
}

//-----------------------------------------------------------------------------

bool ObjectStoreMgr::isSupportedType( const std::type_info& type )
{
  if ( type == typeid( IBulkOperation ) ||
       type == typeid( QueryAndCursor ) ||
       type == typeid( SessionAndFlag ) )
    return true;
  else
    return false;
}

//-----------------------------------------------------------------------------
