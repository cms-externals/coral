// $Id: ColumnDefinition.cpp,v 1.14 2011/03/22 10:29:54 avalassi Exp $
#include "MySQL_headers.h"

#include <sstream>

#include "ColumnDefinition.h"

namespace coral
{
  namespace MySQLAccess
  {
    ColumnDefinition::ColumnDefinition()
      : domain(Numeric)
      , name()
      , type()
      , length(0)
      , fixed(true)
      , nullable(true)
      , unique(false)
      , is_unsigned( false )
    {
    }

    ColumnDefinition::ColumnDefinition(  ColumnDomain dom, const std::string& n, const std::string& t, unsigned int l, bool f, bool null /*=true*/, bool u /*=false*/, bool isunsigned /*=false*/)
      : domain(dom)
      , name(n)
      , type(t)
      , length(l)
      , fixed(f)
      , nullable(null)
      , unique(u)
      , is_unsigned( isunsigned )
    {
    }

    std::string ColumnDefinition::sql()
    {
      std::stringstream ss;
      std::stringstream sstemp;
      std::string sqlType;

      ss << "\"" << name << "\" ";

      if( domain == String )
      {
        if( length > 0 )
        {
          if( length <=255 ) // FIXME - should check the mysql server version due its silent conversions
          {
            if( fixed )
              sstemp << "CHAR(";
            else
              sstemp << "VARCHAR(";
            sstemp << length << ") BINARY "; // Adding binary attribute to enable case sensitive comparison, bug #26484

            //            sstemp << " BINARY";  // To ensure case sensitive comparisons, FIXME - check effect of collations in 4.1 & later
          }
          else // Long string, make it CLOB, FIXME - needs update for 4.1 & later
          {
            if( length == 256 )
              sstemp << "TINYTEXT";
            else if( length > 256 && length < 65536 /* 2^16 */ )
              sstemp << "TEXT";
            else if( length >= 65536 && length < 16777216 /* 2^24 */ )
              sstemp << "MEDIUMTEXT";
            else // length >= 16777216 && length < 4294967296 /* 2^32 */ - anything longer gets cut down to the maximum size
              sstemp << "LONGTEXT";
          }
        }
        else // 0 - Means here undefined so we try to be safe
        {
          sstemp << type;
          /* AV 11-01-2006
          sstemp << "VARCHAR(255)"; // It's user's responsibility to provide more explicit hints for columns, this is more-less safe default
//          sstemp << "TEXT"; // Needs additional hints for indexing
//          sstemp << " BINARY";  // To ensure case sensitive comparisons, FIXME - check effect of collations in 4.1 & later
*/
        }
      }
      else if( domain == Blob )
      {
        if( length > 0 )
        {
          if( length <= 256 )
            sstemp << "TINYBLOB";
          else if( length > 256 && length < 65536 /* 2^16 */ )
            sstemp << "BLOB";
          else if( length >= 65536 && length < 16777216 /* 2^24 */ )
            sstemp << "MEDIUMBLOB";
          else // length >= 16777216 && length < 4294967296 /* 2^32 */ - anything longer gets cut down to the maximum size
            sstemp << "LONGBLOB";
        }
        else // 0 - Means here undefined so we try to be safe
        {
          sstemp << type;
        }
      }
      else
      {
        if( length != 0 )
        {
          sstemp << type;

          if( is_unsigned )
          {
            sstemp << "(" << length << ")";
            sstemp << " UNSIGNED";
          }
        }
        else
        {
          sstemp << type;

          if( is_unsigned )
            sstemp << " UNSIGNED";
        }
      }

      // Update column definition
      type = sstemp.str();

      ss << type;

      if( ! nullable )
        ss << " NOT NULL";

      // FIXME - MySQL allows to add unique constraint even if column is defined as unqiue which
      // FIXME - results in having 2 unique indices being generated silently
      //       if( unique )
      //         ss << " UNIQUE";

      return ss.str();
    }
  }
}
