// $Id: ColumnConstraint.cpp,v 1.11 2011/03/22 10:29:54 avalassi Exp $
#include "MySQL_headers.h"

#include <sstream>

#include "ColumnConstraint.h"

namespace coral
{
  namespace MySQLAccess
  {
    ColumnConstraint::ColumnConstraint()
      : name()
      , uniqueIndex(false)
      , type( None )
    {
    }

    ColumnConstraint::ColumnConstraint( const std::string& n,
                                        bool uniqueIdx /*=false*/,
                                        ConstraintType cc /*=None*/ )
      : name(n)
      , uniqueIndex( uniqueIdx )
      , type(cc)
    {
    }

    void ColumnConstraint::processColumns( std::stringstream& os )
    {
      size_t numColumns = keys.size();

      if( numColumns > 1 )
      {
        os << " ( ";

        for( size_t iColumn = 0; iColumn < numColumns; ++iColumn )
        {
          if( iColumn > 0 )
            os << " , ";
          os << " \"" << keys[iColumn].first << "\" ";
          if( keys[iColumn].second > 0 )
            os << " ( " << keys[iColumn].second << " ) ";
        }

        os << " ) ";
      }
      else
      {
        if( keys[0].second > 0 )
          os << " ( \"" << keys[0].first << "\" ( " << keys[0].second << " ) ) ";
        else
          os << " ( \"" << keys[0].first << "\" ) ";
      }
    }

    void ColumnConstraint::processRefColumns( std::stringstream& os )
    {
      // FIXME - check if there is at least 1 referenced column and throw an exception if none is defined

      size_t numColumns = refColumns.size();

      if( numColumns > 1 )
      {
        os << " (";

        for( size_t iColumn = 0; iColumn < numColumns; ++iColumn )
        {
          if( iColumn > 0 )
            os << ", ";
          os << " \"" << refColumns[iColumn] << "\" ";
        }

        os << ") ";
      }
      else
      {
        os << " (\"" << refColumns[0] << "\") ";
      }

    }

    std::string ColumnConstraint::sql()
    {
      if( keys.empty() )
        return "";

      std::stringstream ss;

      if( type == Primary )
      {
        sqlForPrimaryKey( ss );
      }
      else if( type == Unique )
      {
        sqlForUniqueConstraint( ss );
      }
      else if( type == Foreign )
      {
        sqlForForeignKey( ss );
      }
      else
      {
        // FIXME - May be None, we try to build an index
        sqlForIndices( ss );
      }

      return ss.str();
    }

    void ColumnConstraint::sqlForPrimaryKey( std::stringstream& os )
    {
      os <<  " CONSTRAINT ";

      if( ! name.empty() )
        os << " \"" << name << "\" ";

      os << " PRIMARY KEY ";

      processColumns( os );
    }

    void ColumnConstraint::sqlForUniqueConstraint( std::stringstream& os )
    {
      os <<  " CONSTRAINT ";

      if( ! name.empty() )
        os << " \"" << name << "\" ";

      os << " UNIQUE KEY ";

      processColumns( os );
    }

    void ColumnConstraint::sqlForForeignKey( std::stringstream& os )
    {
      os <<  " CONSTRAINT ";

      if( ! name.empty() )
        os << " \"" << name << "\" ";

      os << " FOREIGN KEY ";

      processColumns( os );

      os << " REFERENCES " << refTable;

      processRefColumns( os );
    }

    void ColumnConstraint::sqlForIndices( std::stringstream& os )
    {
      if( uniqueIndex )
        os << " CONSTRAINT UNIQUE ";

      os << " KEY ";

      if( ! name.empty() )
        os << " \"" << name << "\" ";

      processColumns( os );
    }
  }
}
