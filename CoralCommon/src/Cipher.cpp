#include "CoralCommon/Cipher.h"

char coral::Cipher::u64encMap(char c){
  char off = 0;
  if((int)c < 10) {
    off = 48;
  } else if ( ((int)c>=10) && ((int)c<38)) {
    off = 53;
  } else if ((int)c >=38) {
    off = 59;
  }
  return c+off;
}

char coral::Cipher::u64decMap(char c){
  char off = 0;
  if( (int)c>=48 && (int)c<58) {
    off = 48;
  } else if ((int)c>=63 && (int)c<91) {
    off = 53;
  } else if ((int)c>=97 && (int)c<123) {
    off = 59;
  }
  return c-off;
}

std::string coral::Cipher::u64enc(const std::string &text){
  char buffer[3] = {0};
  char tmp[5] = {0};
  std::string result("");
  for (std::string::size_type i = 0; i < text.size(); ++i ) {
    buffer[i%3] = text[i];
    if (i%3 == 2 || (i==(text.size()-1))) {
      u64enc(buffer,tmp);
      buffer[0]=buffer[1]=buffer[2]=0;
      tmp[4]='\0';
      result += tmp;
    }
  }
  return result;
}

std::string coral::Cipher::u64dec(const std::string &text){
  char buffer[4] = {0};
  char tmp[4] = {0};
  std::string result("");
  for (std::string::size_type i = 0; i < text.size(); ++i ) {
    buffer[i%4] = text[i];
    if (i%4 == 3 || (i==(text.size()-1))) {
      u64dec(buffer,tmp);
      buffer[0]=buffer[1]=buffer[2]=buffer[3]=0;
      tmp[3]='\0';
      result += tmp;
    }
  }
  return result;
}

void coral::Cipher::u64enc(char i[3], char o[4]) {
  char c0 = (i[0] & 0x3f);
  char c1 = (( i[0]>>6 )&0x3) | (((i[1] & 0xf) << 2)&0x3c);
  char c2 = (((i[1] & 0xf0) >> 4)&0xF) | (((i[2] & 0x3) << 4)&0x30 );
  char c3= (( i[2]>> 2)&0x3f);
  o[0] = u64encMap(c0);
  o[1] = u64encMap(c1);
  o[2] = u64encMap(c2);
  o[3] = u64encMap(c3);
}

std::string coral::Cipher::encode (const std::string &text, const std::string &key)
{
  std::string result("");

  char buffer[3] = {0};
  char tmp[5] = {0};

  std::string::size_type ks = key.size();
  for (std::string::size_type i = 0; i < text.size(); ++i ) {
    if(ks) {
      buffer[i%3] = text[i] ^ key[i % ks];
    } else {
      buffer[i%3] = text[i];
    }
    if (i%3 == 2 || (i==(text.size()-1))) {
      u64enc(buffer,tmp);
      if(i==(text.size()-1)) {
        // move converted string terminators to a single 'normal' terminator
        if(text.size()%3 == 1) {
          tmp[3]= tmp[2] = 0;
        } else if(text.size()%3 == 2) {
          tmp[3] = 0;
        }
      }
      buffer[0]=buffer[1]=buffer[2]=0;
      tmp[4] = 0;
      result += tmp;
    }
  }

  return result;
}

void coral::Cipher::u64dec(char i[4], char o[3])
{
  o[0] = u64decMap(i[0]) | (((u64decMap(i[1]) & 0x3) << 6)&0xc0);
  o[1] = ((u64decMap(i[1]) >> 2)&0xF) | (((u64decMap(i[2]) & 0xf) << 4)&0xF0);
  o[2] = ((u64decMap(i[2]) >> 4)&0x3) | ((u64decMap(i[3]) << 2)&0xFc);
}

std::string coral::Cipher::decode (const std::string &text, const std::string &key)
{
  std::string result("");

  char buffer[4] = {0};
  char tmp[4] = {0};

  size_t res = text.size()%4;

  std::string::size_type ks = key.size();

  int j = 0;
  for (std::string::size_type i = 0; i < (text.size()+res); ++i) {
    if(i<text.size()) {
      buffer[i%4] = text[i];
    } else {
      buffer[i%4] = u64encMap(0);
    }
    if ( i%4 == 3 ) {
      u64dec(buffer,tmp);
      buffer[0]=buffer[1]=buffer[2]=buffer[3]=0;
      if(ks) {
        tmp[0] ^= key[j++ % ks];
        tmp[1] ^= key[j++ % ks];
        tmp[2] ^= key[j++ % ks];
      }
      tmp[3] = 0;
      result += tmp;
    }
  }
  if ( text.size()%4 == 2 ) {
    result = result.substr(0,result.size()-2);
  } else if ( text.size()%4 == 3 ) {
    result = result.substr(0,result.size()-1);
  }


  return result;
}
