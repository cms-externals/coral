#ifndef SQLITEACCESS_TYPECONVERTER_H
#define SQLITEACCESS_TYPECONVERTER_H

#include "RelationalAccess/ITypeConverter.h"
#include <map>

namespace coral {
  namespace SQLiteAccess {
    class DomainProperties;
    /**
       @class TypeConverter TypeConverter.h

       Implementation of the ITypeConverter interface for the SQLiteAccess module
       @author Zhen Xie
    */

    class TypeConverter : virtual public ITypeConverter {

    public:
      /// Constructor initializing the default mappings
      explicit TypeConverter( const DomainProperties& properties);

      /// Destructor
      virtual ~TypeConverter() {}

      /// Resets the type converter for a given version
      void reset( int majorVersion );

      /// Returns the SQL types supported by the particular plugin.
      std::set<std::string> supportedSqlTypes() const;

      /// Returns the C++ types supported by the particular plugin.
      std::set<std::string> supportedCppTypes() const;

      /**
       * Returns the default C++ type name for the given SQL type.
       * An empty string is returned in case of an invalid C++ type name.
       *
       * @param sqlType The SQL type name
       *
       */
      std::string defaultCppTypeForSqlType( const std::string& sqlType ) const;

      /**
       * Returns the currently registered C++ type name for the given SQL type.
       * An empty string is returned in case of an invalid C++ type name.
       *
       * @param sqlType
       *
       */
      std::string cppTypeForSqlType( const std::string& sqlType ) const;

      /**
       * Registers a C++ type name for the given SQL type overriding the existing mapping.
       *
       * @param cppType The new C++ type name
       *
       * @param sqlType The SQL type name
       *
       */
      void setCppTypeForSqlType( const std::string& cppType, const std::string& sqlType );

      /**
       * Returns the default SQL type name for the given C++ type.
       * An empty string is returned in case of an invalid SQL type name.
       *
       * @param cppType The C++ type name
       *
       */
      std::string defaultSqlTypeForCppType( const std::string& cppType ) const;

      /**
       * Returns the currently registered SQL type name for the given C++ type.
       * An empty string is returned in case of an invalid SQL type name.
       *
       * @param cppType The C++ type name
       *
       */
      std::string sqlTypeForCppType( const std::string& cppType ) const;

      /**
       * Registers an SQL type name for the given C++ type overriding the existing mapping.
       * Returns false if the given SQL or C++ types are not supported.
       *
       * @param sqlType The new SQL type name
       *
       * @param cppType The C++ type name
       *
       */
      void setSqlTypeForCppType( const std::string& sqlType, const std::string& cppType );

    private:
      const DomainProperties&                   m_properties;
      std::set<std::string>                     m_supportedCppTypes;
      mutable std::set<std::string>             m_supportedSqlTypes;
      std::map<std::string,std::string>         m_defaultCppToSqlMapping;
      mutable std::map<std::string,std::string> m_defaultSqlToCppMapping;
      std::map<std::string,std::string>         m_currentCppToSqlMapping;
      mutable std::map<std::string,std::string> m_currentSqlToCppMapping;

      //private:
      /**
       * Deduces the C++ type given an SQLite type
       */
      //bool deduceCppType( const std::string& sqlType ) const;
    };

  }

}

#endif
