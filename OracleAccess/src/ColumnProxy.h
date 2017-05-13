#ifndef COLUMN_PROXY_H
#define COLUMN_PROXY_H

#include "RelationalAccess/IColumn.h"

namespace coral {

  namespace OracleAccess {

    class IConstraintRegistry;

    class ColumnProxy : virtual public coral::IColumn
    {
    public:
      /// Constructor
      ColumnProxy( const coral::IColumn& columnReference,
                   const IConstraintRegistry& constraintRegistry );

      /// Destructor
      virtual ~ColumnProxy();

      /**
       * Returns the name of the column.
       */
      std::string name() const;

      /**
       * Returns the C++ type of the column.
       */
      std::string type() const;

      /**
       * Returns the column id in the table.
       */
      int indexInTable() const;

      /**
       * Returns the NOT-NULL-ness of the column.
       */
      bool isNotNull() const;

      /**
       * Returns the uniqueness of the column.
       */
      bool isUnique() const;

      /**
       * Returns the maximum size in bytes of the data object which can be held in this column.
       */
      long size() const;

      /**
       * Informs whether the size of the object is fixed or it can be variable.
       * This makes sense mostly for string types.
       */
      bool isSizeFixed() const;

      /**
       * Returns the name of table space for the data. This makes sence mainly for LOBs.
       */
      std::string tableSpaceName() const;

    private:
      /// The actual column
      const coral::IColumn& m_columnReference;

      /// The constraint registry
      const IConstraintRegistry&  m_constraintRegistry;
    };

  }

}

#endif
