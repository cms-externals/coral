#ifndef CORALCOMMON_CIPHER_H
#define CORALCOMMON_CIPHER_H

#include <string>

namespace coral {

  class Cipher {

  public:

    static std::string encode (const std::string &text, const std::string &key);
    static std::string decode (const std::string &text, const std::string &key);

    static std::string u64enc(const std::string &text);
    static std::string u64dec(const std::string &text);

    static char u64encMap(char c);
    static char u64decMap(char c);


  private:

    static void u64dec(char i[4], char o[3]);
    static void u64enc(char i[3], char o[4]);

  };

}
#endif
