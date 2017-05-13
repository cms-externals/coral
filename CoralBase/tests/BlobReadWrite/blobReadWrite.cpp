#include "CoralBase/Blob.h"

#include <iostream>
#include <stdexcept>

int main( int, char** )
{
  try {
    coral::Blob blob1;

    coral::Blob blob1cp( blob1 );

    coral::Blob blob1cpasgn = blob1;

    blob1cp = blob1cpasgn;

    blob1cpasgn += blob1cpasgn;

    int numberOfElements = 1024;

    blob1.extend( numberOfElements * sizeof(float) );

    float* addressToElement = static_cast< float* >( blob1.startingAddress() );

    for ( int i = 0; i < numberOfElements; ++i, ++addressToElement ) {
      *addressToElement = (float)i;
    }

    int extraElements = 512;
    blob1.extend( extraElements * sizeof(float) );
    addressToElement = static_cast< float* >( blob1.startingAddress() );
    for ( int i = 0; i < numberOfElements; ++i, ++addressToElement ) {}
    for ( int i = numberOfElements; i < numberOfElements + extraElements; ++i, ++addressToElement ) {
      *addressToElement = (float)i;
    }

    int numberOfDroppedElements = 126;
    numberOfElements += extraElements - numberOfDroppedElements;

    blob1.resize( numberOfElements * sizeof(float) );
    long blobSize = blob1.size();

    if ( blobSize != static_cast< long >( numberOfElements * sizeof( float ) ) ) {
      std::cerr << "Blob size : " << blobSize << std::endl;
      std::cerr << "Expected : " << numberOfElements * sizeof( float ) << std::endl;
      throw std::runtime_error( "Unexpected size of the blob" );
    }

    coral::Blob blob2( 100 * sizeof(float) );
    addressToElement = static_cast< float* >( blob2.startingAddress() );
    for ( int i = 0; i < 100; ++i, ++addressToElement ) {
      *addressToElement = (float)i + numberOfElements;
    }

    blob1 += blob2;

    numberOfElements += 100;
    if ( blob1.size() != static_cast<long>( numberOfElements*sizeof(float) ) )
      throw std::runtime_error( "Unexpected size of the blob after appending" );

    addressToElement = static_cast< float* >( blob1.startingAddress() );
    for ( int i = 0; i < numberOfElements; ++i, ++addressToElement ) {
      if ( *addressToElement != i )
        throw std::runtime_error( "Unexpected data found" );
    }


    for ( int i = 0; i < 10; ++i ) {
      blob1 = coral::Blob();
      long targetSize = 1000 * ( i + 1 );
      while ( targetSize > 0 ) {
        long chunkSize = 1024;
        coral::Blob chunk( chunkSize );
        targetSize -= chunkSize;
        if ( targetSize < 0 ) chunkSize += targetSize;
        chunk.resize( chunkSize );
        blob1 += chunk;
      }
    }

  }
  catch ( std::exception& e ) {
    std::cerr << e.what() << std::endl;
    return 1;
  }
  catch ( ... ) {
    std::cerr << "Unknown exception ..." << std::endl;
    return 1;
  }
  std::cout << "[OVAL] : Test successful" << std::endl;
  return 0;
}
