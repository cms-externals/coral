#ifndef ORACLEACCESS_CURSOR_H
#define ORACLEACCESS_CURSOR_H 1

#include <memory>
#include "RelationalAccess/ICursor.h"

namespace coral
{
  namespace OracleAccess
  {

    class OracleStatement;

    class Cursor : virtual public coral::ICursor
    {
    public:

      /// Constructor
      Cursor( std::auto_ptr<OracleStatement> statement,
              const coral::AttributeList& rowBuffer );

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
      const coral::AttributeList& currentRow() const;

      /**
       * Explicitly closes the cursor, releasing the resources on the server.
       */
      void close();

    private:

      /// The statement handler
      std::auto_ptr<OracleStatement> m_statement; // fix memory leak bug #90898

      /// The row buffer
      const coral::AttributeList& m_rowBuffer;

    };

  }
}
#endif
