// $Id: TypeConverter.h,v 1.6 2011/03/22 10:29:55 avalassi Exp $
#ifndef MySQLACCESS_TYPECONVERTER_H
#define MySQLACCESS_TYPECONVERTER_H 1

#include <map>
#include <string>
#include "RelationalAccess/ITypeConverter.h"

namespace coral
{

  namespace MySQLAccess
  {

    struct SqlTypeInfo
    {
      std::string driverTypeName; ///< Name as returned by MySQL driver
      std::string odbcTypeName; ///< ODBC type name
      long odbcTypeId; ///< ODBC type ID
      size_t columnSize; ///< Max. column size
      bool isNullable; ///< Can be set to NULL?
      bool isUnsigned; ///< Unsigned (if applicable)
      size_t maxScale; ///< Scale
    };

    /**
       @class MySQLAccess::TypeConverter TypeConverter.h

       Implementation of the IRelationalTypeConverter interface for the MySQLAccess module

    */
    class TypeConverter : public coral::ITypeConverter
    {
    public:
      /// Constructor initializing the default mappings
      explicit TypeConverter();

      /// Destructor
      virtual ~TypeConverter() {}

      /// Returns the SQL types supported by the particular plugin.
      virtual std::set<std::string> supportedSqlTypes() const;

      /// Returns the C++ types supported by the particular plugin.
      virtual std::set<std::string> supportedCppTypes() const;

      /**
       * Returns the default C++ type name for the given SQL type.
       * An empty string is returned in case of an invalid C++ type name.
       *
       * @param sqlType The SQL type name
       *
       */
      virtual std::string defaultCppTypeForSqlType( const std::string& sqlType ) const;

      /**
       * Returns the currently registered C++ type name for the given SQL type.
       * An empty string is returned in case of an invalid C++ type name.
       *
       * @param sqlType
       *
       */
      virtual std::string cppTypeForSqlType( const std::string& sqlType ) const;

      /**
       * Registers a C++ type name for the given SQL type overriding the existing mapping.
       * Returns false if the given SQL or C++ types are not supported.
       *
       * @param cppType The new C++ type name
       *
       * @param sqlType The SQL type name
       *
       */
      virtual void setCppTypeForSqlType( const std::string& cppType, const std::string& sqlType );

      /**
       * Returns the default SQL type name for the given C++ type.
       * An empty string is returned in case of an invalid SQL type name.
       *
       * @param cppType The C++ type name
       *
       */
      virtual std::string defaultSqlTypeForCppType( const std::string& cppType ) const;

      /**
       * Returns the currently registered SQL type name for the given C++ type.
       * An empty string is returned in case of an invalid SQL type name.
       *
       * @param cppType The C++ type name
       *
       */
      virtual std::string sqlTypeForCppType( const std::string& cppType ) const;

      /**
       * Registers an SQL type name for the given C++ type overriding the existing mapping.
       * Returns false if the given SQL or C++ types are not supported.
       *
       * @param sqlType The new SQL type name
       *
       * @param cppType The C++ type name
       *
       */
      virtual void setSqlTypeForCppType( const std::string& sqlType, const std::string& cppType );

    public:
      /**
       * Deduces the C++ type given an MySQL type
       */
      bool deduceCppType( const std::string& sqlType ) const;

    private:
      /// Hidden
      TypeConverter( const TypeConverter& );

      /// Initialize the default mappings
      void initialize();

    private:
      mutable std::set<std::string>             m_supportedCppTypes;
      mutable std::set<std::string>             m_supportedSqlTypes;
      mutable std::map<std::string,std::string> m_defaultCppToSqlMapping;
      mutable std::map<std::string,std::string> m_defaultSqlToCppMapping;
      mutable std::map<std::string,std::string> m_currentCppToSqlMapping;
      mutable std::map<std::string,std::string> m_currentSqlToCppMapping;
    };
  }
}

#endif // MYSQLACCESS_TYPECONVERTER_H
