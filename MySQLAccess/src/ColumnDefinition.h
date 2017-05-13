// $Id: ColumnDefinition.h,v 1.7 2011/03/22 10:29:54 avalassi Exp $
#ifndef CORAL_MYSQLACCESS_COLUMNDEFINITION_H
#define CORAL_MYSQLACCESS_COLUMNDEFINITION_H 1

#include <string>

namespace coral
{
  namespace MySQLAccess
  {
    enum ColumnDomain
      {
        Numeric, String, Blob, Date, TimeStamp
      };

    struct ColumnDefinition
    {
      /// Default constructor
      ColumnDefinition();

      /// Explicit constructor
      ColumnDefinition(  ColumnDomain dom, const std::string& n, const std::string& t
                         , unsigned int l, bool f, bool null=true, bool u=false, bool isunsigned=false );

      /// Build DDL fragment for column definition
      std::string sql();

      /// numeric? string? time?
      ColumnDomain domain;
      /// column name
      std::string name;
      /// column type
      std::string type;
      /// column length
      unsigned int length;
      /// variable size?
      bool fixed;
      /// nullable
      bool nullable;
      /// unique
      bool unique;
      /// signed/unsigned?
      bool is_unsigned;
    };
  }
}

#endif // CORAL_MYSQLACCESS_COLUMNDEFINITION_H
