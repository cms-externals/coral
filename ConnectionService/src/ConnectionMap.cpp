#include "ConnectionMap.h"
#include "ConnectionString.h"

/// constructor
coral::ConnectionService::ConnectionMap::ConnectionMap() :
  m_connections(){
}

/// destructor
coral::ConnectionService::ConnectionMap::~ConnectionMap(){
}

/// adds the specified session in the map associated to the specified session handle key.
bool
coral::ConnectionService::ConnectionMap::add( const ConnectionHandle& connectionHandle ){
  if(!connectionHandle) return false;
  m_connections.insert(std::make_pair(ConnectionString::serviceKey(connectionHandle.technologyName(),connectionHandle.serviceName()),connectionHandle));
  return true;
}

/// lookup the specified connection string in the available sessions
coral::ConnectionService::ConnectionMap::Iterator
coral::ConnectionService::ConnectionMap::lookup( const std::string& technology, const std::string& serviceName ) {
  return Iterator(m_connections.find(ConnectionString::serviceKey(technology,serviceName)));
}

/// removes the session entry for the specified element
void
coral::ConnectionService::ConnectionMap::remove( coral::ConnectionService::ConnectionMap::Iterator theElement ){
  m_connections.erase( theElement.m_iterator );
}

/// removes the session entry for the specified sessionId key
bool
coral::ConnectionService::ConnectionMap::remove( const ConnectionHandle& connectionHandle ){
  bool ret = false;
  typedef std::multimap<std::string, ConnectionHandle >::iterator _iterator;
  _iterator theIterator = m_connections.end();
  std::pair<_iterator,_iterator> range = m_connections.equal_range(ConnectionString::serviceKey(connectionHandle.technologyName(),
                                                                                                connectionHandle.serviceName()));
  for(_iterator i=range.first; i!=range.second; ++i) {
    if(i->second.connectionId()==connectionHandle.connectionId()) {
      theIterator = i;
      ret = true;
      break;
    }
  }
  if(ret) m_connections.erase( theIterator );
  return ret;
}

/// lookup the specified connection string in the available sessions
coral::ConnectionService::ConnectionMap::Iterator
coral::ConnectionService::ConnectionMap::find( const ConnectionHandle& connectionHandle ) {
  typedef std::multimap<std::string, ConnectionHandle >::iterator _iterator;
  _iterator theIterator(m_connections.end());
  std::pair<_iterator,_iterator> range = m_connections.equal_range(ConnectionString::serviceKey(connectionHandle.technologyName(),
                                                                                                connectionHandle.serviceName()));
  for(_iterator i=range.first; i!=range.second; ++i) {
    if(i->second.connectionId()==connectionHandle.connectionId()) {
      theIterator = i;
      break;
    }
  }
  return Iterator(theIterator);
}


/// returns the valid map iterator
coral::ConnectionService::ConnectionMap::Iterator
coral::ConnectionService::ConnectionMap::begin(){
  return Iterator(m_connections.begin());
}

/// returns the end map iterator
coral::ConnectionService::ConnectionMap::Iterator
coral::ConnectionService::ConnectionMap::end(){
  return Iterator(m_connections.end());
}

/// clear the underlying map
void
coral::ConnectionService::ConnectionMap::clear(){
  m_connections.clear();
}

/// returns the size of the map
size_t
coral::ConnectionService::ConnectionMap::size(){
  return m_connections.size();
}
