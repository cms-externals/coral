#ifndef SQLITEACCESS_CURSOR_H
#define SQLITEACCESS_CURSOR_H

#include "RelationalAccess/ICursor.h"

namespace coral {

  namespace SQLiteAccess {

    class SQLiteStatement;

    class Cursor : virtual public coral::ICursor
    {
    public:
      /// Constructor
      Cursor( SQLiteStatement* statement,
              coral::AttributeList& rowBuffer );

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
      SQLiteStatement* m_statement;
      /// The row buffer
      coral::AttributeList& m_rowBuffer;
    };

  }

}

#endif
