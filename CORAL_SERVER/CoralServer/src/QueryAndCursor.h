#ifndef CORALSERVER_QUERYANDCURSOR_H
#define CORALSERVER_QUERYANDCURSOR_H 1

// Include files
#include "RelationalAccess/IQuery.h"

namespace coral
{

  namespace CoralServer
  {

    /** @class QueryAndCursor
     *
     *  This is just a triplet { IQuery*, ICursor&, AttributeList* }.
     *  It is needed to register all three in the IObjectStoreMgr.
     *
     *  @author Andrea Valassi
     *  @date   2008-02-15
     */

    class QueryAndCursor
    {

    public:

      /// Constructor.
      QueryAndCursor( IQuery* query,
                      ICursor& cursor,
                      AttributeList* buffer )
        : m_query( query ), m_cursor( cursor ), m_buffer( buffer ) {}

      /// Destructor.
      virtual ~QueryAndCursor()
      {
        delete m_query;
      }

      /// The query
      IQuery* query() const
      {
        return m_query;
      }

      /// The cursor
      ICursor& cursor() const
      {
        return m_cursor;
      }

      /// The buffer
      AttributeList* buffer() const
      {
        return m_buffer;
      }

    private:

      /// The query
      IQuery* m_query;

      /// The cursor
      ICursor& m_cursor;

      /// The buffer
      AttributeList* m_buffer;

    };

  }

}
#endif // CORALSERVER_QUERYANDCURSOR_H
