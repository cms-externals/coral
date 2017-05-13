#ifndef CORALSTUBS_SEGMENTREADERITERATOR_H
#define CORALSTUBS_SEGMENTREADERITERATOR_H

//CoralServer includes
#include "CoralServerBase/ByteBuffer.h"
#include "CoralServerBase/CALOpcode.h"
#include "CoralServerBase/QueryDefinition.h"
#include "CoralServerBase/IByteBufferIterator.h"

//CoralServerStubs includes
#include "CppTypes.h"

//Coral includes
#include "CoralBase/AttributeList.h"
#include "CoralBase/Attribute.h"
#include "RelationalAccess/IQueryDefinition.h"
#include "RelationalAccess/TableDescription.h"
#include "CoralBase/Blob.h"
#include "CoralBase/Date.h"
#include "CoralBase/TimeStamp.h"

//C++ includes
#include <string>
#include <list>
#include <map>
#include <set>

namespace coral {

  namespace CoralStubs {

    class SegmentReaderIterator
    {
    public:

      SegmentReaderIterator(CALOpcode opcode, IByteBufferIterator& iterator);

      ~SegmentReaderIterator();

      bool proxy() const;

      bool cacheable() const;
      //extracts the header
      //check on CAL version
      void noextract();
      //extracts only the header
      //no checks on CAL version
      //if content is empty
      void empty();

      CALOpcode opcode();

      void extract(bool&);

      void extract(uint16_t&);

      void extract(uint32_t&);

      void extract(uint64_t&);

      void extract(uint128_t&);

      void extractN(unsigned int&);

      void extractN(int&);

      void extractN(unsigned long&);

      void extractN(long&);

      void extractN(unsigned long long&);

      void extractN(long long&);

      void extract16(std::string&);

      void extract32(std::string&);

      void extract(Blob&);

      void extract(coral::Date&);

      void extract(coral::TimeStamp&);

      void extract(CALOpcode&);

      void extract(std::vector<std::string>&);

      void extract(std::set<std::string>&);

      void extract(std::vector< std::pair<std::string, std::string> >&);

      void extract(std::map< std::string, std::string >&);

      void extract(TableDescription&);

      void extract(std::map< std::string, TableDescription >&);

      void extract(QueryDefinition&);

      void extract(IQueryDefinition::SetOperation&);

      void extractV(AttributeList&);

      void extractE(AttributeList&);

      void extractD(void*);

      void* getStructure(AttributeList&);

    private:

      void nextBuffer();

      void CALVersion();

      void extend();

      void extract(std::string&, size_t);

      void extractAttributeListE(AttributeList&);

      void extractAttributeListV(AttributeList&);

      template<class T> inline
      void extractAttribute(AttributeList&, const bool, const std::string&, const std::type_info&);

      template<class T> inline
      void extractAttributeN(AttributeList&, const bool, const std::string&, const std::type_info&);

      void exception();

      CALOpcode m_opcode;

      bool m_reply;

      IByteBufferIterator& m_bi;

      const unsigned char* m_bufferpos;

      const unsigned char* m_bufferend;

      unsigned char m_sl;

      unsigned char m_si;

      unsigned char m_sll;

      bool m_proxy;

      bool m_cacheable;


    };

  }

}

#endif
