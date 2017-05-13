// $Id: Cursor.h,v 1.4.2.1 2010/12/20 09:10:09 avalassi Exp $
#ifndef CORALACCESS_CURSOR_H
#define CORALACCESS_CURSOR_H 1

// Include files
#include <memory>
#include <vector>
#include "RelationalAccess/ICursor.h"
#include "CoralServerBase/ICoralFacade.h"
#include "CoralServerBase/QueryDefinition.h"

namespace coral
{

  namespace CoralAccess
  {

    // Forward declarations
    class Cursor;
    class SessionProperties;

    /** @class Cursor
     *
     *  @author Andrea Valassi
     *  @date   2007-12-05
     */

    class Cursor : virtual public ICursor
    {

    public:

      /// Constructor
      Cursor( const SessionProperties& sessionProperties,
              const QueryDefinition& queryDef,
              coral::AttributeList* pOutputBuffer,
              size_t cacheSize,
              bool cacheSizeInMB );

      /// Constructor
      Cursor( const SessionProperties& sessionProperties,
              const QueryDefinition& queryDef,
              const std::map< std::string, std::string > outputTypes,
              size_t cacheSize,
              bool cacheSizeInMB );

      /// Destructor
      virtual ~Cursor();

      /**
       * Positions the cursor to the next available row in the result set.
       * If there are no more rows in the result set false is returned.
       */
      bool next();

      /**
       * Returns a reference to output buffer holding the data of the last
       * row fetched.
       */
      const AttributeList& currentRow() const;

      /**
       * Explicitly closes the cursor, releasing the resources on the server.
       */
      void close();

      /// Returns the facade for the CORAL server connection.
      const ICoralFacade& facade() const;

    private:

      /// The properties of this remote database session.
      const SessionProperties& m_sessionProperties;

      /// The row iterator.
      IRowIteratorPtr m_rows;

    };

  }

}
#endif // CORALACCESS_CURSOR_H
