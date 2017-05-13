#ifndef FRONTIER_ACCESS_FRONTIERTYPECONVERTER_H
#define FRONTIER_ACCESS_FRONTIERTYPECONVERTER_H

#include "RelationalAccess/ITypeConverter.h"
#include <map>

namespace coral {

  namespace FrontierAccess {

    class DomainProperties;

    /**
       @class TypeConverter TypeConverter.h

       Implementation of the IRelationalTypeConverter interface for the FrontierAccess module
    */

    class TypeConverter : virtual public coral::ITypeConverter
    {
    public:
      /// Constructor initializing the default mappings
      explicit TypeConverter( const DomainProperties& properties );

      /// Destructor
      virtual ~TypeConverter() {}

      /// Resets the type converter for a given version
      void reset( int majorVersion );

      /**
       * Returns the SQL types supported by the particular database server.
       */
      std::set<std::string> supportedSqlTypes() const;

      /**
       * Returns the C++ types supported by the particular implementation.
       */
      std::set<std::string> supportedCppTypes() const;

      /**
       * Returns the default C++ type name for the given SQL type.
       * If an invalid SQL type name is specified, an UnSupportedSqlTypeException is thrown.
       *
       */
      std::string defaultCppTypeForSqlType( const std::string& sqlType ) const;

      /**
       * Returns the currently registered C++ type name for the given SQL type.
       * If an invalid SQL type name is specified, an UnSupportedSqlTypeException is thrown.
       *
       */
      std::string cppTypeForSqlType( const std::string& sqlType) const;

      /**
       * Registers a C++ type name for the given SQL type overriding the existing mapping.
       * If any of the types specified is not supported the relevant TypeConverterException is thrown.
       */
      void setCppTypeForSqlType( const std::string& cppType, const std::string& sqlType );

      /**
       * Returns the default SQL type name for the given C++ type.
       * If an invalid C++ type name is specified, an UnSupportedCppTypeException is thrown.
       *
       */
      std::string defaultSqlTypeForCppType( const std::string& cppType ) const;

      /**
       * Returns the currently registered SQL type name for the given C++ type.
       * If an invalid C++ type name is specified, an UnSupportedCppTypeException is thrown.
       */
      std::string sqlTypeForCppType( const std::string& cppType ) const;

      /**
       * Registers an SQL type name for the given C++ type overriding the existing mapping.
       * If any of the types specified is not supported the relevant TypeConverterException is thrown.
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

    private:
      /**
       * Deduces the C++ type given an Oracle type
       */
      bool deduceCppType( const std::string& sqlType ) const;
    };

  }

}

#endif
