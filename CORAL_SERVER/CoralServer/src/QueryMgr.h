// $Id: QueryMgr.h,v 1.3.2.1 2010/12/20 09:10:10 avalassi Exp $
#ifndef CORALSERVER_QUERYMGR_H
#define CORALSERVER_QUERYMGR_H 1

// Include files
#include "RelationalAccess/IQuery.h"
#include "RelationalAccess/ISessionProxy.h"
#include "CoralServerBase/QueryDefinition.h"

namespace coral
{

  namespace CoralServer
  {

    /** @class QueryMgr QueryMgr.h
     *
     *  Relational query manager.
     *
     *  @author Andrea Valassi
     *  @date   2007-06-19 (as part of cool::RalQueryMgr)
     *  @date   2008-01-22 (as coral::CoralProtocol::QueryMgr)
     */

    class QueryMgr
    {

    public:

      /// Create a new coral::IQuery from its definition.
      /// A pointer to an AttributeList output buffer can be provided.
      static coral::IQuery* newQuery( coral::AttributeList* pOutputBuffer,
                                      coral::ISessionProxy& session,
                                      const coral::QueryDefinition& qd );

      /// Create a new coral::IQuery from its definition,
      /// A map of output types must be provided.
      static coral::IQuery* newQuery( const std::map< std::string, std::string > outputTypes,
                                      coral::ISessionProxy& session,
                                      const coral::QueryDefinition& qd );

    private:

      /// Fill a coral::IQuery from its definition.
      /// This assumes that the target query is empty.
      static void prepareQuery( coral::IQueryDefinition& query,
                                const coral::QueryDefinition& qd );

    };
  }
}
#endif // CORALSERVER_QUERYMGR_H
