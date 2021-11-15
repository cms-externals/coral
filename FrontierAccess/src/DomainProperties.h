// -*- C++ -*-
// $Id: DomainProperties.h,v 1.4 2011/03/22 10:36:50 avalassi Exp $
#ifndef FRONTIER_ACCESS_FRONTIERDOMAIN_PROPERTIES_H
#define FRONTIER_ACCESS_FRONTIERDOMAIN_PROPERTIES_H

#include "CoralKernel/Service.h"
#include "CoralKernel/IHandle.h"

#include "RelationalAccess/IConnectionService.h"

#include <string>

namespace coral
{
  class IWebCacheControl;

  namespace FrontierAccess
  {

    class Domain;

    /**
     * A simple class holding the global domain properties
     */

    class DomainProperties
    {
    public:
      /// Constructor
      explicit DomainProperties( Domain* service );

      /// Destructor
      ~DomainProperties();

      /// Sets the table space for the tables
      void setTableSpaceForTables( const std::string& tableSpace );

      /// Sets the table space for the indices
      void setTableSpaceForIndices( const std::string& tableSpace );

      /// Sets the table space for the lobs
      void setTableSpaceForLobs( const std::string& tableSpace );

      /// Sets the lob chunk size
      void setLobChunkSize( int lobChunkSize );

      /// Returns the pointer to the service
      coral::Service* service() const;

      /// Returns the table space for the tables
      std::string tableSpaceForTables() const;

      /// Returns the table space for the indices
      std::string tableSpaceForIndices() const;

      /// Returns the table space for the lobs
      std::string tableSpaceForLobs() const;

      /// Returns the lob chunk size
      int lobChunkSize() const;

      /// Access to Web cache control
      const coral::IWebCacheControl& cacheControl() const;

      /// Access to the global lock
      static boost::mutex& lock();

    protected:
      coral::IHandle<coral::IConnectionService> connectionService() const;

    private:
      /// The pointer to the service
      Domain*              m_service;

      /// The default table space for tables
      std::string m_tableSpaceForTables;

      /// The default table space for indices
      std::string m_tableSpaceForIndices;

      /// The default table space for lobs
      std::string m_tableSpaceForLobs;

      /// The chunk size in bytes for a LOB retrieval
      int m_lobChunkSize;

      /// The class global lock
      static boost::mutex s_lock;

    };

  }

}

inline boost::mutex& coral::FrontierAccess::DomainProperties::lock()
{
  return s_lock;
}


#endif
