#ifndef CONNECTIONSERVICE_CONNECTIONMAP_H
#define CONNECTIONSERVICE_CONNECTIONMAP_H

#include "ConnectionHandle.h"
#include <map>

namespace coral {

  class ISession;

  namespace ConnectionService {

    /// wrapper on the std::map container for the storage of coral ISession objects
    class ConnectionMap {

    public:

      /// constructor
      ConnectionMap();

      /// destructor
      virtual ~ConnectionMap();

      class Iterator {
      public:
        explicit Iterator(std::multimap<std::string, ConnectionHandle >::iterator theIterator) : m_iterator(theIterator){}
        virtual ~Iterator() {}
        Iterator(const Iterator& rhs) : m_iterator(rhs.m_iterator){}
        Iterator& operator=(const Iterator& rhs){
          m_iterator = rhs.m_iterator;
          return *this;
        }
        bool operator==( const Iterator& rhs ) const { return m_iterator == rhs.m_iterator; }
        bool operator!=( const Iterator& rhs ) const { return m_iterator != rhs.m_iterator; }
        void operator++() { ++m_iterator; }
        ConnectionHandle* operator->() { return &m_iterator->second; }
        ConnectionHandle& operator*() { return m_iterator->second; }
      private:
        friend class ConnectionMap;
        std::multimap<std::string, ConnectionHandle >::iterator m_iterator;

      };

      /// adds the specified session in the map associated to the specified session handle key.
      bool add( const ConnectionHandle& sessionHandle );

      /// lookup the specified connection string in the available sessions
      Iterator lookup( const std::string& technology, const std::string& serviceName );

      /// removes the session entry for the specified sessionId key
      bool remove( const ConnectionHandle& connectionHandle );

      /// removes the session entry for the specified sessionId ke
      void remove( Iterator theElement );

      /// returns the valid map iterator
      Iterator find( const ConnectionHandle& connectionHandle );

      /// returns the valid map iterator
      Iterator begin();

      /// returns the end map iterator
      Iterator end();

      /// clear the underlying map
      void clear();

      /// returns the size of the map
      size_t size();

    private:

      /// map of the available connections
      std::multimap<std::string, ConnectionHandle > m_connections;

    };

  }

}

#endif
