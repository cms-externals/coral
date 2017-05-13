#ifndef LFCREPLICASERVICE_LFCFILE_H
#define LFCREPLICASERVICE_LFCFILE_H

#include <string>

namespace coral {

  namespace LFCReplicaService {

    // Class for Guid Creation. This is taken from pool->LFCCatalogue for GUID generation.
    class Guid {
    public:

      /// Standard constructor (No initialization of data for performance reasons)
      Guid(){}

      /// Standard constructor (With possible initialization)
      explicit Guid(bool assign)  {
        if ( assign ) create(*this);
      }

      /// Constructor for Guid from char*
      explicit Guid(const char* s){
        fromString(s);
      }

      /// Constructor for Guid from string
      explicit Guid(const std::string& s) {
        fromString(s);
      }

      /// Copy constructor
      Guid(const Guid& c){
        *this = c;
      }

      /// Assignment operator
      Guid& operator=(const Guid& g);

      /// Smaller operator
      bool operator<(const Guid& g)  const;
      /// Equality operator
      bool operator==(const Guid& g)  const ;

      /// Non-equality operator
      bool operator!=(const Guid& g)  const;

      /// Automatic conversion from string reprentation
      std::string toString() const;
      /// Automatic conversion to string representation
      const Guid& fromString(const std::string& s);
      /// NULL-Guid: static class method
      static const Guid& null();
      /// Create a new Guid
      static void create(Guid& gUid);

      static bool isGuid(const std::string& s);

    private:

      unsigned int Data1;
      unsigned short Data2;
      unsigned short Data3;
      unsigned char Data4[8];

    };
  }
}

#endif
