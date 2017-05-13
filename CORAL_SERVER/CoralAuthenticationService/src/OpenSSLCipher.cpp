#include "OpenSSLCipher.h"

#include <ctime>
#include <cstdlib>
#include <cstring>
#include <iostream>
#include <stdint.h> // For uint32_t on gcc46 (bug #89762)

#ifdef HAVE_OPENSSL
#include <openssl/evp.h>
#endif

using namespace coral;

static const unsigned char base64[65]="ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
static const unsigned char debase64[256]={
  0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,
  0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,
  0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0x3e,0xff,0xff,0xff,0x3f,
  0x34,0x35,0x36,0x37,0x38,0x39,0x3a,0x3b,0x3c,0x3d,0xff,0xff,0xff,0xff,0xff,0xff,
  0xff,0x00,0x01,0x02,0x03,0x04,0x05,0x06,0x07,0x08,0x09,0x0a,0x0b,0x0c,0x0d,0x0e,
  0x0f,0x10,0x11,0x12,0x13,0x14,0x15,0x16,0x17,0x18,0x19,0xff,0xff,0xff,0xff,0xff,
  0xff,0x1a,0x1b,0x1c,0x1d,0x1e,0x1f,0x20,0x21,0x22,0x23,0x24,0x25,0x26,0x27,0x28,
  0x29,0x2a,0x2b,0x2c,0x2d,0x2e,0x2f,0x30,0x31,0x32,0x33,0xff,0xff,0xff,0xff,0xff,
  0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,
  0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,
  0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,
  0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,
  0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,
  0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,
  0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,
  0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff
};

std::string coral::EncodeBase64(const std::string& orig)
{
  std::string result;
  size_t length=orig.length();
  const unsigned char * str=(const unsigned char*)orig.c_str();
  result.reserve(length*4/3);

  while (length>=3)
  {
    uint32_t bytes=(str[0]<<16)+(str[1]<<8)+(str[2]<<0);
    result+=base64[ (bytes >> 18) &0x3f ];
    result+=base64[ (bytes >> 12) &0x3f ];
    result+=base64[ (bytes >>  6) &0x3f ];
    result+=base64[ (bytes >>  0) &0x3f ];
    length-=3;
    str+=3;
  };
  if (length > 0 )
  {
    uint32_t bytes=str[0]<<16;
    if (length==2)
      bytes+=str[1] << 8;
    result+=base64[ (bytes >> 18) &0x3f ];
    result+=base64[ (bytes >> 12) &0x3f ];
    if (length==1) {
      result+='=';
      result+='=';
    } else {
      result+=base64[ (bytes >>  6) &0x3f ];
      result+='=';
    };
  }
  return result;
}

std::string coral::DecodeBase64( const std::string& orig )
{
  std::string result;
  size_t length=orig.length();
  const char * str=orig.c_str();
  result.reserve(length*3/4);
#if 0
  char decode[256];
  for ( int i = 0; i< 255; i++)
    decode[i]=0xff;

  for ( int i= 0; i < 64; i++)
    decode[base64[i]]=i;

  for ( int i =0; i<255; i++)
    printf(",0x%02x",decode[i]);
#endif
  int byte_count=0;
  uint32_t bytes=0;
  while (length>0 && *str!='=')
  {
    if (debase64[(unsigned int)*str]!=255) {
      bytes<<=6;
      bytes+=debase64[(unsigned int)*str];
      byte_count+=1;
      if ( byte_count==4 )
      {
        result+=(char) (bytes >> 16) &0xFF;
        result+=(char) (bytes >>  8) &0xFF;
        result+=(char) (bytes >>  0) &0xFF;
        byte_count=0;
        bytes=0;
      };
    };
    str++;
    length--;
  }
  if ( length>0 && byte_count>0)
  {
    switch (byte_count) {
    case 1:
      throw OpenSSLCipherException("Bytes are missing!", "OpenSSLCipher::DecodeBase64()");
    case 2:
      if ( length <2 || str[0]!='=' || str[1]!='=')
        throw OpenSSLCipherException("Bytes are missing!", "OpenSSLCipher::DecodeBase64()");
      result+=(char) (bytes >>  4) &0xFF;
      break;
    case 3:
      if ( length <1 || str[0]!='=' )
        throw OpenSSLCipherException("Bytes are missing!", "OpenSSLCipher::DecodeBase64()");
      result+=(char) (bytes >> 10) &0xFF;
      result+=(char) (bytes >>  2) &0xFF;
      break;
    }
  };

  return result;
}

//----------------------------------------------------------------------------

OpenSSLCipher::OpenSSLCipher()
{
#ifdef HAVE_OPENSSL
  OpenSSL_add_all_algorithms();
  SSLeay_add_all_algorithms();
  //SSL_load_error_strings();
  //ERR_load_crypto_strings();
  srandom( time( 0 ) );
#endif
}

//----------------------------------------------------------------------------

OpenSSLCipher::~OpenSSLCipher()
{
}

//----------------------------------------------------------------------------

std::string OpenSSLCipher::createRandomSalt( )
{
  std::string salt;
  // FIXME we could use a better random generator here... and probably we should init it
  for ( int i = 0; i < 8; i++ )
    salt += (char) ( random() & 0xFF);
  return salt;
}

//----------------------------------------------------------------------------

#ifndef HAVE_OPENSSL

std::string OpenSSLCipher::Encrypt( const std::string& /*password*/,
                                    const std::string& data )
{
  return data;
}

#else

std::string OpenSSLCipher::Encrypt( const std::string& password,
                                    const std::string& data )
{
  if (data.empty())
    return "";

  std::string salt( createRandomSalt() );

  const EVP_CIPHER *cipher = EVP_get_cipherbyname("aes-128-cbc");
  const EVP_MD *md = EVP_get_digestbyname("md5");

  unsigned char *key= (unsigned char*)malloc(EVP_CIPHER_key_length(cipher));
  unsigned char *iv = (unsigned char*)malloc(EVP_CIPHER_iv_length(cipher));

  if ( salt.length() != 8 )
    throw OpenSSLCipherException("Error salt is not of length 8!", "OpenSSLCipher::Encrypt()");

  // initializing key and iv from password and salt
  EVP_BytesToKey(cipher,md,
                 (const unsigned char*) salt.data(),
                 (const unsigned char *) password.data(), password.length(), 1,
                 key,iv);

  // initializing context
  EVP_CIPHER_CTX ctx;
  EVP_EncryptInit(&ctx, cipher, key, NULL);

  int messageLen= data.length();

  unsigned char *outb=(unsigned char*) malloc( messageLen + 2*EVP_CIPHER_block_size( cipher ) );
  int outblen=0;

  if (!outb)
    throw OpenSSLCipherException("Could not allocate enough memory for crypted data!",
                                 "OpenSSLCipher::Encrypt()");

  int len =0;
  if ( EVP_EncryptUpdate(&ctx, outb, &len, (const unsigned char*)data.data(), messageLen) == 0 )
    throw OpenSSLCipherException("EncryptUpdate failed!", "OpenSSLCipher::Encrypt()");
  outblen+=len;

  if ( EVP_CipherFinal_ex(&ctx,  outb+outblen, &len) == 0 )
    throw OpenSSLCipherException("CipherFinal failed!", "OpenSSLCipher::Encrypt()");
  outblen+=len;

  std::string ret( (const char*)outb, (size_t)outblen);
  free( outb );
  free( key );
  free( iv );

  return salt+ret;
}

#endif

//----------------------------------------------------------------------------

#ifndef HAVE_OPENSSL

std::string OpenSSLCipher::Decrypt( const std::string& /*password*/,
                                    const std::string& saltdata )
{
  return saltdata;
}

#else

std::string OpenSSLCipher::Decrypt( const std::string& password,
                                    const std::string& saltdata )
{
  if (saltdata.empty())
    return "";

  const EVP_CIPHER *cipher = EVP_get_cipherbyname("aes-128-cbc");
  const EVP_MD *md = EVP_get_digestbyname("md5");

  unsigned char *key= (unsigned char*)malloc(EVP_CIPHER_key_length(cipher));
  unsigned char *iv = (unsigned char*)malloc(EVP_CIPHER_iv_length(cipher));

  if (saltdata.length() < (size_t)(8+EVP_CIPHER_block_size( cipher )) )
    throw OpenSSLCipherException("Encrypted data is smaller than salt size + block size!",
                                 "OpenSSLCipher::Decrypt()");

  std::string salt= saltdata.substr( 0, 8 );
  std::string data= saltdata.substr( 8, saltdata.length() );

  EVP_BytesToKey(cipher,md,
                 (const unsigned char*) salt.data(),
                 (const unsigned char *) password.data(), password.length(), 1,
                 key,iv);

  EVP_CIPHER_CTX ctx;

  EVP_DecryptInit(&ctx, cipher, key, NULL);

  unsigned char *demessage=(unsigned char*) malloc(  data.length() + 2*EVP_CIPHER_block_size( cipher ) );
  int demessageLen=0;
  if ( !demessage )
    throw OpenSSLCipherException("Could not allocate memory for decrypted data!",
                                 "OpenSSLCipher::Decrypt");

  int len=0;
  if ( EVP_DecryptUpdate(&ctx, demessage, &len,
                         (const unsigned char*)data.data(), data.length()) == 0 )
    throw OpenSSLCipherException("EncryptUpdate failed!", "OpenSSLCipher::Encrypt()");
  demessageLen+=len;

  if ( EVP_CipherFinal_ex(&ctx,  demessage+demessageLen, &len) == 0 )
    throw OpenSSLCipherException("CipherUpdate failed!", "OpenSSLCipher::Encrypt()");
  demessageLen+=len;

  std::string ret( (const char*)demessage, (size_t)demessageLen);
  free( demessage );
  free( key );
  free( iv );

  return ret;
}

#endif

//----------------------------------------------------------------------------
