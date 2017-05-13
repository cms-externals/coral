// $Id: ColumnConstraint.h,v 1.4 2011/03/22 10:29:54 avalassi Exp $
#ifndef CORAL_MYSQLACCESS_COLUMNCONSTRAINT_H
#define CORAL_MYSQLACCESS_COLUMNCONSTRAINT_H 1

#include <string>
#include <vector>

namespace coral
{
  namespace MySQLAccess
  {
    enum ConstraintType
      {
        None /* Index only */, Primary, Unique, Foreign
      };

    typedef std::pair<std::string,unsigned int> IndexedColumn;
    typedef std::vector< IndexedColumn >        IndexedColumns;

    struct ColumnConstraint
    {
      ColumnConstraint();

      ColumnConstraint( const std::string& n,
                        bool uniqueIdx=false,
                        ConstraintType cc=None );

      void processColumns( std::stringstream& os );

      void processRefColumns( std::stringstream& os );

      std::string sql();

      void sqlForPrimaryKey( std::stringstream& os );

      void sqlForUniqueConstraint( std::stringstream& os );

      void sqlForForeignKey( std::stringstream& os );

      void sqlForIndices( std::stringstream& os );

      // FIXME - Add index handling
      // FIXME - Add foreign keys handling
      // FIXME - Add composite foreign keys handling

      /// column name
      std::string name;
      /// unique index?, only if it is an index
      bool uniqueIndex;
      /// constraint: unique, primary, foreign key
      ConstraintType type;
      /// constraint column name(s)
      IndexedColumns keys;
      /// fk table name(s)
      std::string refTable;
      /// fk column name(s)
      std::vector<std::string> refColumns;

    };
  }
}

#endif // CORAL_MYSQLACCESS_COLUMNCONSTRAINT_H
