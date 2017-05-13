#include "CoralBase/MessageStream.h"

int main( int, char** )
{
  // Create a message stream
  coral::MessageStream msg01("Test01");
  msg01 << coral::Always << "Hello world #1" << coral::MessageStream::endmsg;
  // Create a message stream
  coral::MessageStream msg02("Test02");
  msg02 << coral::Always << "Hello world #2" << coral::MessageStream::endmsg;
  msg02 << coral::Always << "Hello world #3" << coral::MessageStream::endmsg;
}
