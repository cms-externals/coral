// $Id: Query.h,v 1.4.2.1 2010/12/20 09:10:09 avalassi Exp $
#ifndef CORALACCESS_QUERY_H
#define CORALACCESS_QUERY_H 1

// Include files
#include <memory>
#include "RelationalAccess/IQuery.h"
#include "CoralServerBase/ICoralFacade.h"
#include "CoralServerBase/QueryDefinition.h"

namespace coral
{

  namespace CoralAccess
  {

    // Forward declarations
    class Cursor;
    class SessionProperties;

    /** @class Query
     *
     *  @author Andrea Valassi
     *  @date   2007-12-05
     */

    class Query : public QueryDefinition, virtual public IQuery
    {

    public:

      /// Constructor
      Query( const SessionProperties& sessionProperties,
             const std::string& schemaName,
             const std::string& tableName = "" );

      /// Destructor
      virtual ~Query();

      /**
       * Instructs the server to lock the rows involved in the result set.
       */
      void setForUpdate();

      /**
       * Defines the client cache size in rows
       */
      void setRowCacheSize( int numberOfCachedRows );

      /**
       * Defines the client cache size in MB.
       */
      void setMemoryCacheSize( int sizeInMB );

      /**
       * Defines the output types of a given variable in the result set.
       */
      void defineOutputType( const std::string& outputIdentifier,
                             const std::string& cppTypeName );

      /**
       * Defines the output data buffer for the result set.
       */
      void defineOutput( AttributeList& outputDataBuffer );

      /**
       * Executes the query and returns a reference to the undelying ICursor object
       * in order for the user to loop over the result set.
       */
      ICursor& execute();

      /// Returns the facade for the CORAL server connection.
      const ICoralFacade& facade() const;

    private:

      /// The properties of this remote database session.
      const SessionProperties& m_sessionProperties;

      /// The cache size (in #rows or MBs)
      size_t m_cacheSize;

      /// Is the cache size in MBs (or in #rows)?
      bool m_cacheSizeInMB;

      /// The externally defined output types
      std::map< std::string, std::string > m_outputTypes;

      /// The output buffer (owned by this Query)
      AttributeList* m_pOutputBuffer;

      /// The cursor object (owned by this Query)
      Cursor* m_pCursor;

    };

  }

}
#endif // CORALACCESS_QUERY_H
