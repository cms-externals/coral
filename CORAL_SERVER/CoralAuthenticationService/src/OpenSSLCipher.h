#ifndef CORALAUTHENTICATIONSERVICE_OPENSSLCIPHER_H
#define CORALAUTHENTICATIONSERVICE_OPENSSLCIPHER_H

#include <string>
#include <CoralBase/Exception.h>

namespace coral
{

  std::string EncodeBase64( const std::string& orig );
  std::string DecodeBase64( const std::string& orig );

  class OpenSSLCipher
  {
  public:
    OpenSSLCipher();
    ~OpenSSLCipher();
    std::string Encrypt( const std::string& password,
                         const std::string& data);
    std::string Decrypt( const std::string& password,
                         const std::string& data);
  private:
    std::string createRandomSalt();
  };

  class OpenSSLCipherException : public coral::Exception
  {
  public:
    OpenSSLCipherException( const std::string& message,
                            const std::string& methodName )
      : Exception( message, methodName, "CoralAuthenticationService" ) {};
    virtual ~OpenSSLCipherException() throw() {};
  };

}
#endif // CORAL_OPENSSLCIPHER_H
