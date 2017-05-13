#ifndef CORAL_CONNECTIONSTABLE_H
#define CORAL_CONNECTIONSTABLE_H

#include "CoralKernel/Service.h"
#include "RelationalAccess/ISchema.h"
#include "RelationalAccess/AuthenticationCredentials.h"

#include "OpenSSLCipher.h"

namespace coral {
  class CredentialsTable
  {
  public:

    static const std::string defaultTableName()
    { return "CREDENTIALS_TBL"; }

    struct columnNames {
      static const std::string physConStr() { return "PHYS_CON_STR"; }
      static const std::string user() { return "USER"; }
      static const std::string passwd() { return "PSSWD"; }
      static const std::string dbRole() { return "DB_ROLE"; }
    };

    struct columnTypes {
      typedef std::string physConStr;
      typedef std::string user;
      typedef std::string passwd;
      typedef std::string dbRole;
    };

    // Standard Constructor
    CredentialsTable(ISchema& schema);

    /// Standard Destructor
    virtual ~CredentialsTable();

    /// create a connections table.
    static void createTable( ISchema& schema );

    /// drop the connections table
    static void deleteTable( ISchema& schema );

    /// add credentials to the table
    void addCredentials( const std::string& physConStr,
                         const std::string& user, const std::string& password,
                         const std::string& role );

    /// delete connection(s) from the table
    /// returns the no of deleted connections
    long delCredentials( const std::string& physConStr="",
                         const std::string& user="", const std::string& role="" );

    void listCredentials( const std::string& physConStr="",
                          const std::string& user="", const std::string& role="" );

    bool existCredentials( const std::string& physConStr="",
                           const std::string& user="", const std::string& role="");

    const AuthenticationCredentials getCredentials( const std::string& physConStr,
                                                    const std::string& role );

    const std::string decryptPasswd( const std::string & cryptPw );
    /*
      std::vector<unsigned int>
         getIds( const std::string& connectionString = "",
            const std::string & user = "", const std::string& role ="" );
    */
  private:
    void generateWhere( AttributeList & whereData, std::string& whereString,
                        bool wildcards, const std::string& table,
                        const std::string& physConStr, const std::string& user,
                        const std::string& role );


    /// the schema in which the table resides
    ISchema &m_schema;

    /// handle to the database table
    ITable &m_table;

    /// the cipher to en/decrypt the passwords
    OpenSSLCipher m_cipher;

  };

}

#endif
