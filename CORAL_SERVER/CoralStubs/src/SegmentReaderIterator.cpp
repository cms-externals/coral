#include "SegmentReaderIterator.h"

//CoralServer includes
#include "CoralServerBase/hexstring.h"
#include "CoralServerBase/CALPacketHeader.h"

//Coral includes
#include "RelationalAccess/IIndex.h"
#include "RelationalAccess/IForeignKey.h"
#include "RelationalAccess/IPrimaryKey.h"
#include "RelationalAccess/IUniqueConstraint.h"
#include "RelationalAccess/IColumn.h"

//CoralServerStubs includes
#include "Exceptions.h"
#include "AttributeUtils.h"

//C++ includes
#include <sstream>

#define VECTORSIZE uint16_t

namespace coral { namespace CoralStubs {

  SegmentReaderIterator::SegmentReaderIterator(CALOpcode opcode, IByteBufferIterator& iterator)
    : m_opcode( opcode )
    , m_reply( opcode != 0 )
    , m_bi( iterator )
    , m_bufferpos( 0 )
    , m_bufferend( 0 )
    , m_sl( 0 )
    , m_si( 0 )
    , m_sll( 0 )
    , m_proxy( false )
    , m_cacheable( false )
  {
  }

  SegmentReaderIterator::~SegmentReaderIterator()
  {
  }

  bool
  SegmentReaderIterator::proxy() const
  {
    return m_proxy;
  }

  bool
  SegmentReaderIterator::cacheable() const
  {
    return m_cacheable;
  }

  void
  SegmentReaderIterator::exception()
  {
    CALOpcode fl01;
    CALOpcode fl02;

    extract(fl01);
    extract(fl02);

    if(fl01 == 0x00)
      //all CoralExceptions here
    {
      if(fl02 == 0x01)
      {
        //std::exception
        std::string s1;
        extract16(s1);
        std::string s2;
        extract16(s2);
        //throw it
        throw Exception( "Remote std::exception from CoralServer: " + s1, s2, "coral::CoralStubs" );
      }
      else if(fl02 == 0x02)
      {
        //coral exception
        std::string part01;
        extract16(part01);
        std::string part02;
        extract16(part02);
        std::string part03;
        extract16(part03);
        //throw it
        throw Exception( "Remote coral::Exception from CoralServer: " + part01, part02, part03 );
      }
      else if(fl02 == 0x10)
      {
        //stubs exception
        std::string s1;
        extract16(s1);
        std::string s2;
        extract16(s2);
        //throw it
        throw InvalidMessageException( "Remote InvalidMessage exception from CoralServer: " + s1, s2 );
      }
    }
    else if(fl01 == 0x01)
    {
      //andy's exceptions (exceptions from the dbproxy)
      //get the size of the message inside the rest of the payload
      /*
                        size_t maxsize = replyBuffer.usedSize() - CALPACKET_HEADER_SIZE - 2;
                        //create a buffer
                        char* buffer = (char*)malloc(sizeof(char) * (maxsize + 1));
                        //copy the string into the buffer
                        memcpy((void*)buffer, replyBuffer.data() + CALPACKET_HEADER_SIZE + 2, maxsize);
                        //throw the exception
                        buffer[maxsize] = 0;

                        throw DBProxyException(buffer);
      */
    }
    else if(fl01 == 0x02)
    {
      throw Exception( "Remote exception from CoralServer: wrong CAL Protocol Version", "ClientStub::prepareExceptionMessage", "coral::CoralStubs" );
    }
    std::ostringstream s;
    s << "Unknown remote exception from CoralServer (" << hexstring(&fl01,1) << ")(" << hexstring(&fl02,1) << ")";
    throw Exception(s.str(), "ClientStub::prepareExceptionMessage", "coral::CoralStubs");
  }

  void
  SegmentReaderIterator::nextBuffer()
  {
    //get the next buffer
    if(!m_bi.next())
      //the iterator contains no buffers anymore
      //throw an exception here
      throw StreamBufferException("ByteBuffer iterator is empty", "SegmentReaderIterator::extend()");

    const ByteBuffer& bb = m_bi.currentBuffer();
    //set the positions
    m_bufferpos = bb.data() + CALPACKET_HEADER_SIZE;
    m_bufferend = bb.data() + bb.usedSize();
    //check the opcode
    CALPacketHeader header(bb.data(), CALPACKET_HEADER_SIZE);

    CALOpcode opcode01 = header.opcode();

    if(m_reply)
    {
      CALOpcode opcode02 = CALOpcodes::getCALRequest( opcode01 );

      if(m_opcode != opcode02)
      {
        std::ostringstream s;
        s << "Reply opcode (" << hexstring(&opcode02,1) << ") is different from request opcode (" << hexstring(&opcode01,1) <<")";
        throw InvalidMessageException(s.str(), "SegmentReaderIterator::extend()");
      }

      //check if we have an exception
      if(CALOpcodes::isCALReplyException(opcode01))
        exception();

      //check if we have a valid reply ok message
      if(!CALOpcodes::isCALReplyOK(opcode01))
        throw InvalidMessageException("Message is not a reply", "SegmentReaderIterator::extend()");
    }
    else
    {
      if( !CALOpcodes::isCALRequest(opcode01) )
        throw InvalidMessageException("Message is not a request", "SegmentReaderIterator::extend()");

      //check if we have an exception
      if(CALOpcodes::isCALReplyException(opcode01))
        exception();

      m_opcode = opcode01;
    }
    //set the from proxy flag
    m_proxy = header.fromProxy();
    //set the cacheable flag
    m_cacheable = header.cacheable();
  }

  void
  SegmentReaderIterator::CALVersion()
  {
    //get the CAL protocol version
    unsigned char version = *m_bufferpos;
    if(version != 0x03)
    {
      std::ostringstream s;
      s << "Wrong CAL version [" << (short)version << "]";
      throw StreamBufferException(s.str(), "SegmentReaderIterator::extend()");
    }

    m_bufferpos++;
    //get the Format version
    m_sl = *m_bufferpos;
    m_bufferpos++;

    m_sll = *m_bufferpos;
    m_bufferpos++;

    m_si = *m_bufferpos;
    m_bufferpos++;

    //  std::cout << "size [" << bb.usedSize() << "] long [" << (short int)m_sl << "] int [" << (short int)m_si << "] long long [" << (short int)m_sll << "]" << std::endl;
  }

  void
  SegmentReaderIterator::extend()
  {
    nextBuffer();
    CALVersion();
  }

  CALOpcode
  SegmentReaderIterator::opcode()
  {
    return m_opcode;
  }

  void
  SegmentReaderIterator::noextract()
  {
    if(m_bufferpos == 0)
      extend();
  }

  void
  SegmentReaderIterator::empty()
  {
    if(m_bufferpos == 0)
    {
      nextBuffer();
      if(m_bufferpos != m_bufferend)
      {
        //we have content here
        CALVersion();
      }
    }
  }

  void
  SegmentReaderIterator::extract(bool& data)
  {
    if((m_bufferpos + 1) > m_bufferend)
      extend();

    data = *m_bufferpos;
    m_bufferpos++;
  }

  void
  SegmentReaderIterator::extract(uint16_t& data)
  {
    if((m_bufferpos + 2) > m_bufferend)
      extend();

    data = *((uint16_t*)m_bufferpos);
    m_bufferpos += 2;
  }

  void
  SegmentReaderIterator::extract(uint32_t& data)
  {
    if((m_bufferpos + 4) > m_bufferend)
      extend();

    data = *((uint32_t*)m_bufferpos);
    m_bufferpos += 4;
  }

  void
  SegmentReaderIterator::extract(uint64_t& data)
  {
    if((m_bufferpos + 8) > m_bufferend)
      extend();

    data = *((uint64_t*)m_bufferpos);
    m_bufferpos += 8;
  }

  void
  SegmentReaderIterator::extract(uint128_t& data)
  {
    extract(data[0]);
    extract(data[1]);
  }

  void
  SegmentReaderIterator::extractN(unsigned int& data)
  {
    if(m_si == 2)
      extract((uint16_t&)data);
    else if(m_si == 4)
      extract((uint32_t&)data);
    else if(m_si == 8)
      extract((uint64_t&)data);
    else
      throw StreamBufferException("Can't convert integer", "SegmentReaderIterator::extractN()");
  }

  void
  SegmentReaderIterator::extractN(int& data)
  {
    if(m_si == 2)
    {
      if((m_bufferpos + 2) > m_bufferend)
        extend();

      data = *((int16_t*)m_bufferpos);
      m_bufferpos += 2;
    }
    else if(m_si == 4)
    {
      if((m_bufferpos + 4) > m_bufferend)
        extend();

      data = *((int32_t*)m_bufferpos);
      m_bufferpos += 4;
    }
    else if(m_si == 8)
    {
      if((m_bufferpos + 8) > m_bufferend)
        extend();

      data = *((int64_t*)m_bufferpos);
      m_bufferpos += 8;
    }
    else
      throw StreamBufferException("Can't convert integer", "SegmentReaderIterator::extractN()");
  }

  void
  SegmentReaderIterator::extractN(unsigned long& data)
  {
    if(m_sl == 4)
      extract((uint32_t&)data);
    else if(m_sl == 8)
      extract((uint64_t&)data);
    else
      throw StreamBufferException("Can't convert long", "SegmentReaderIterator::extractN()");
  }

  void
  SegmentReaderIterator::extractN(long& data)
  {
    if(m_sl == 4)
    {
      if((m_bufferpos + 4) > m_bufferend)
        extend();

      data = *((int32_t*)m_bufferpos);
      m_bufferpos += 4;
    }
    else if(m_sl == 8)
    {
      if((m_bufferpos + 8) > m_bufferend)
        extend();

      data = *((int64_t*)m_bufferpos);
      m_bufferpos += 8;
    }
    else
      throw StreamBufferException("Can't convert long", "SegmentReaderIterator::extractN()");
  }

  void
  SegmentReaderIterator::extractN(unsigned long long& data)
  {
    if(m_sll == 8)
      extract((uint64_t&)data);
    else
      throw StreamBufferException("Can't convert long long", "SegmentReaderIterator::extractN()");
  }

  void
  SegmentReaderIterator::extractN(long long& data)
  {
    if(m_sll == 8)
    {
      if((m_bufferpos + 8) > m_bufferend)
        extend();

      data = *((int64_t*)m_bufferpos);
      m_bufferpos += 8;
    }
    else
      throw StreamBufferException("Can't convert long long", "SegmentReaderIterator::extractN()");
  }

  void
  SegmentReaderIterator::extract(std::string& data, size_t size)
  {
    data.clear();

    size_t diff01 = size;

    while(m_bufferpos + diff01 > m_bufferend)
    {
      //    std::cout << "size : " << size << " diff : " << (m_bufferend - m_bufferpos) << " diff02 : " << diff01 << std::endl;
      //get the difference between the end and the current pos
      size_t diff02 = m_bufferend - m_bufferpos;

      data.append( (char*)m_bufferpos, diff02 );

      m_bufferpos = m_bufferpos + diff02;

      extend();

      diff01 = diff01 - diff02;
    }

    data.append( (char*)m_bufferpos, diff01 );

    m_bufferpos = m_bufferpos + diff01;
  }

  void
  SegmentReaderIterator::extract16(std::string& data)
  {
    //get the size of the string
    uint16_t size = 0;
    extract( size );

    extract(data, size);
  }

  void
  SegmentReaderIterator::extract32(std::string& data)
  {
    //get the size of the string
    uint32_t size = 0;
    extract( size );

    extract(data, size);
  }

  void
  SegmentReaderIterator::extract(Blob& data)
  {
    //get the size of the string
    uint32_t size = 0;
    extract( size );

    size_t diff01 = size;
    size_t offset = 0;
    //malloc the size for the blob
    data.resize( size );

    while(m_bufferpos + diff01 > m_bufferend)
    {
      //get the difference between the end and the current pos
      size_t diff02 = m_bufferend - m_bufferpos;

      memcpy((char*)data.startingAddress() + offset, m_bufferpos, diff02);

      m_bufferpos = m_bufferpos + diff02;

      offset = offset + diff02;

      extend();

      diff01 = diff01 - diff02;
    }

    memcpy((char*)data.startingAddress() + offset, m_bufferpos, diff01);

    m_bufferpos = m_bufferpos + diff01;
  }

  void
  SegmentReaderIterator::extract(coral::Date& data)
  {
    uint16_t year;
    extract(year);
    uint16_t month;
    extract(month);
    uint16_t day;
    extract(day);
    data = coral::Date(year, month, day);
  }

  void
  SegmentReaderIterator::extract(coral::TimeStamp& data)
  {
    uint16_t year;
    extract(year);
    uint16_t month;
    extract(month);
    uint16_t day;
    extract(day);
    uint16_t hour;
    extract(hour);
    uint16_t minute;
    extract(minute);
    uint16_t second;
    extract(second);
    uint64_t nsecond;
    extract(nsecond);
    data = coral::TimeStamp( year, month,  day, hour, minute, second, nsecond);
  }

  void
  SegmentReaderIterator::extract(CALOpcode& data)
  {
    if((m_bufferpos + 1) > m_bufferend)
      extend();

    //FIXME here must be a definition of the size of cal opcode
    data = (unsigned char)(*m_bufferpos);
    m_bufferpos++;
  }

  void
  SegmentReaderIterator::extract(std::vector<std::string>& data)
  {
    //clear the vector
    data.clear();
    //continue
    VECTORSIZE size;
    extract(size);
    for(VECTORSIZE i = 0; i < size; i++ ) {
      std::string s;
      extract16(s);
      data.push_back(s);
    }
  }

  void
  SegmentReaderIterator::extract(std::set<std::string>& data)
  {
    //clear the set
    data.clear();
    //continue
    VECTORSIZE size;
    extract(size);
    for(VECTORSIZE i = 0; i < size; i++ )
    {
      std::string s;
      extract16(s);
      data.insert(s);
    }
  }

  void
  SegmentReaderIterator::extract(std::vector< std::pair<std::string, std::string> >& data)
  {
    //clear the vector
    data.clear();
    //continue
    VECTORSIZE size;
    extract(size);
    for(VECTORSIZE i = 0; i < size; i++ )
    {
      std::string s1;
      extract16(s1);
      std::string s2;
      extract16(s2);
      data.push_back(std::pair<std::string, std::string>(s1, s2));
    }
  }

  void
  SegmentReaderIterator::extract(std::map< std::string, std::string >& data)
  {
    data.clear();

    VECTORSIZE size;
    extract(size);
    for(VECTORSIZE i = 0; i < size; i++ )
    {
      std::string s1;
      extract16(s1);
      std::string s2;
      extract16(s2);
      data.insert(std::pair< std::string, std::string >(s1, s2));
    }
  }

  void
  SegmentReaderIterator::extract(TableDescription& data)
  {
    //read the table name
    std::string tableName;
    extract16(tableName);
    data.setName(tableName);
    //read the table type
    std::string tableType;
    extract16(tableType);
    data.setType(tableType);
    //read the table space name
    std::string tableSpaceName;
    extract16(tableSpaceName);
    data.setTableSpaceName(tableSpaceName);
    //get the columns
    VECTORSIZE columns;
    extract(columns);
    for(VECTORSIZE i = 0; i < columns; i++ )
    {
      std::string cname;
      extract16(cname);
      std::string ctype;
      extract16(ctype);
      uint32_t csize;
      extract(csize);
      bool cfixedsize;
      extract(cfixedsize);
      std::string cspacename;
      extract16(cspacename);
      data.insertColumn( cname, ctype, csize, cfixedsize, cspacename );
    }
    //set the unique constraints
    VECTORSIZE constraints;
    extract(constraints);
    for(VECTORSIZE i = 0; i < constraints; i++ )
    {
      std::vector<std::string> cnames;
      extract(cnames);
      std::string cname;
      extract16(cname);
      std::string cspacename;
      extract16(cspacename);
      data.setUniqueConstraint( cnames, cname, true, cspacename );
    }
    //set primary key
    bool isprikey;
    extract(isprikey);
    if(isprikey)
    {
      std::vector<std::string> cnames;
      extract(cnames);
      std::string cspacename;
      extract16(cspacename);
      data.setPrimaryKey( cnames, cspacename );
    }
    //set foreign keys
    VECTORSIZE foreignkeys;
    extract(foreignkeys);
    for(VECTORSIZE i = 0; i < foreignkeys; i++ ) {
      std::vector<std::string> cnames;
      extract(cnames);
      std::string cname;
      extract16(cname);
      std::string crtablename;
      extract16(crtablename);
      std::vector<std::string> crcolumns;
      extract(crcolumns);
      data.createForeignKey( cname, cnames, crtablename, crcolumns );
    }
    //set indexes
    VECTORSIZE indexes;
    extract(indexes);
    for(VECTORSIZE i = 0; i < indexes; i++ ) {
      std::vector<std::string> cnames;
      extract(cnames);
      std::string cname;
      extract16(cname);
      bool cisunique;
      extract(cisunique);
      std::string cspacename;
      extract16(cspacename);
      data.createIndex( cname, cnames, cisunique, cspacename );
    }
  }

  void
  SegmentReaderIterator::extract(std::map< std::string, TableDescription >& data)
  {
    VECTORSIZE size;
    extract(size);
    for(VECTORSIZE i = 0; i < size; i++ ) {
      std::string s;
      extract16(s);
      TableDescription td;
      extract(td);
      data.insert(std::pair< std::string, TableDescription >(s, td));
    }
  }

  void
  SegmentReaderIterator::extract(QueryDefinition& data)
  {
    //set some iterators
    std::vector< std::pair<std::string,std::string> >::iterator i;
    std::vector< std::string >::iterator j;

    std::string schemaname;
    extract16(schemaname);
    data.setSchemaName( schemaname );
    //default values
    bool distinct;
    extract(distinct);
    if(distinct) data.setDistinct();

    std::vector< std::pair<std::string,std::string> > outputlist;
    extract(outputlist);
    for(i = outputlist.begin(); i != outputlist.end(); i++ ) data.addToOutputList(i->first, i->second);

    std::vector< std::pair<std::string,std::string> > tablelist;
    extract(tablelist);
    for(i = tablelist.begin(); i != tablelist.end(); i++ ) data.addToTableList(i->first, i->second);

    std::string condition;
    extract16(condition);

    AttributeList conditionData;
    extractV(conditionData);
    data.setCondition( condition, conditionData );

    std::string groupby;
    extract16(groupby);
    data.groupBy(groupby);

    std::vector< std::string > orderlist;
    extract(orderlist);
    for(j = orderlist.begin(); j != orderlist.end(); j++ ) data.addToOrderList(*j);

    //special values
    VECTORSIZE size;
    extract(size);
    for(VECTORSIZE k = 0; k < size; k++ )
    {
      std::string alias;
      extract16(alias);
      QueryDefinition& qd = dynamic_cast<QueryDefinition&>(data.defineSubQuery( alias ));
      extract(qd);
    }

    bool limits;
    extract(limits);
    if(limits)
    {
      uint32_t l01;
      extract(l01);
      uint32_t l02;
      extract(l02);
      data.limitReturnedRows( l01, l02 );
    }

    bool operations;
    extract(operations);
    if(operations)
    {
      IQueryDefinition::SetOperation setopt;
      extract(setopt);
      QueryDefinition& qd = dynamic_cast<QueryDefinition&>(data.applySetOperation( setopt ));
      extract(qd);
    }
  }

  void
  SegmentReaderIterator::extract(IQueryDefinition::SetOperation& data)
  {
    CALOpcode res = 0x00;
    extract(res);
    if(res == 0x01)
      data = IQueryDefinition::Union;
    else if(res == 0x02)
      data = IQueryDefinition::Minus;
    else if(res == 0x03)
      data = IQueryDefinition::Intersect;
  }

  template<class T> inline
  void
  SegmentReaderIterator::extractAttribute(AttributeList& data, const bool isnull, const std::string& name, const std::type_info& type)
  {
    data.extend( name, type);
    Attribute& attr = data[data.size() - 1];

    if(isnull) attr.setNull( true );
    else extract( *( (T*)attr.addressOfData() ) );
  }

  template<> inline
  void
  SegmentReaderIterator::extractAttribute<std::string>(AttributeList& data, const bool isnull, const std::string& name, const std::type_info& type)
  {
    data.extend( name, type);
    Attribute& attr = data[data.size() - 1];

    if(isnull) attr.setNull( true );
    else extract32( *( (std::string*)attr.addressOfData() ) );
  }

  template<class T> inline
  void
  SegmentReaderIterator::extractAttributeN(AttributeList& data, const bool isnull, const std::string& name, const std::type_info& type)
  {
    data.extend( name, type);
    Attribute& attr = data[data.size() - 1];

    if(isnull) attr.setNull( true );
    else extractN( *( (T*)attr.addressOfData() ) );
  }

  void
  SegmentReaderIterator::extractAttributeListV(AttributeList& attr)
  {
    VECTORSIZE size;
    extract(size);
    for(VECTORSIZE i = 0; i < size; i++ )
    {
      //get the attribute name
      std::string name;
      extract16(name);
      //get is null
      bool isnull;
      extract(isnull);
      //get the type of attribute
      CALOpcode topcode;
      extract(topcode);
      //check all opcodes and do the transformation
      if      ( topcode == 0x01 ) extractAttribute<bool>(attr, isnull, name, typeid(bool));
      else if ( topcode == 0x02 ) extractAttribute<unsigned char>(attr, isnull, name, typeid(char));
      else if ( topcode == 0x03 ) extractAttribute<unsigned char>(attr, isnull, name, typeid(unsigned char));
      else if ( topcode == 0x04 ) extractAttribute<unsigned char>(attr, isnull, name, typeid(signed char));
      else if ( topcode == 0x05 ) extractAttribute<uint16_t>(attr, isnull, name, typeid(short));
      else if ( topcode == 0x06 ) extractAttribute<uint16_t>(attr, isnull, name, typeid(unsigned short));
      else if ( topcode == 0x07 ) extractAttributeN<int>(attr, isnull, name, typeid(int));
      else if ( topcode == 0x08 ) extractAttributeN<unsigned int>(attr, isnull, name, typeid(unsigned int));
      else if ( topcode == 0x09 ) extractAttributeN<long>(attr, isnull, name, typeid(long));
      else if ( topcode == 0x0a ) extractAttributeN<unsigned long>(attr, isnull, name, typeid(unsigned long));
      else if ( topcode == 0x0b ) extractAttributeN<long long>(attr, isnull, name, typeid(long long));
      else if ( topcode == 0x0c ) extractAttributeN<unsigned long long>(attr, isnull, name, typeid(unsigned long long));
      else if ( topcode == 0x0d ) extractAttribute<uint32_t>(attr, isnull, name, typeid(float));
      else if ( topcode == 0x0e ) extractAttribute<uint64_t>(attr, isnull, name, typeid(double));
      else if ( topcode == 0x0f ) extractAttribute<uint128_t>(attr, isnull, name, typeid(long double));
      else if ( topcode == 0x10 ) extractAttribute<std::string>(attr, isnull, name, typeid(std::string));
      else if ( topcode == 0x11 ) extractAttribute<coral::Blob>(attr, isnull, name, typeid(coral::Blob));
      else if ( topcode == 0x12 ) extractAttribute<coral::Date>(attr, isnull, name, typeid(coral::Date));
      else if ( topcode == 0x13 ) extractAttribute<coral::TimeStamp>(attr, isnull, name, typeid(coral::TimeStamp));
      else throw StreamBufferException("Not supported attribute opcode", "RStreamBuffer::read<AttributeList&>");
    }
  }

  void
  SegmentReaderIterator::extractV(AttributeList& data)
  {
    if(data.size() > 0)
    {
      //we have already something in the list
      //unfortunately there is no delete method in the attributelist api
      //we create a new attribute list an copy this one to the original
      AttributeList attr;
      extractAttributeListV(attr);
      //copy the attribute lists
      copyAttributeLists(data, attr);
    }
    else
    {
      extractAttributeListV(data);
    }
  }

  void
  SegmentReaderIterator::extractAttributeListE(AttributeList& attr)
  {
    VECTORSIZE size;
    extract(size);
    for(VECTORSIZE i = 0; i < size; i++ )
    {
      //get the attribute name
      std::string name;
      extract16(name);
      //get the type of attribute
      CALOpcode topcode;
      extract(topcode);
      //check all opcodes and do the transformation
      if      ( topcode == 0x01 ) attr.extend( name, typeid(bool));
      else if ( topcode == 0x02 ) attr.extend( name, typeid(char));
      else if ( topcode == 0x03 ) attr.extend( name, typeid(unsigned char));
      else if ( topcode == 0x04 ) attr.extend( name, typeid(signed char));
      else if ( topcode == 0x05 ) attr.extend( name, typeid(short));
      else if ( topcode == 0x06 ) attr.extend( name, typeid(unsigned short));
      else if ( topcode == 0x07 ) attr.extend( name, typeid(int));
      else if ( topcode == 0x08 ) attr.extend( name, typeid(unsigned int));
      else if ( topcode == 0x09 ) attr.extend( name, typeid(long));
      else if ( topcode == 0x0a ) attr.extend( name, typeid(unsigned long));
      else if ( topcode == 0x0b ) attr.extend( name, typeid(long long));
      else if ( topcode == 0x0c ) attr.extend( name, typeid(unsigned long long));
      else if ( topcode == 0x0d ) attr.extend( name, typeid(float));
      else if ( topcode == 0x0e ) attr.extend( name, typeid(double));
      else if ( topcode == 0x0f ) attr.extend( name, typeid(long double));
      else if ( topcode == 0x10 ) attr.extend( name, typeid(std::string));
      else if ( topcode == 0x11 ) attr.extend( name, typeid(coral::Blob));
      else if ( topcode == 0x12 )
      {
        //default constructor of coral does not set the correct timestamp
        //keep this until coral230 will be used
        attr.extend( name, typeid(coral::Date));
        coral::Date date( boost::posix_time::microsec_clock::universal_time() );
        attr[i].data<coral::Date>() = date;
      }
      else if ( topcode == 0x13 )
      {
        //default constructor of coral does not set the correct timestamp
        //keep this until coral230 will be used
        attr.extend( name, typeid(coral::TimeStamp));
        coral::TimeStamp time( boost::posix_time::microsec_clock::universal_time() );
        attr[i].data<coral::TimeStamp>() = time;
      }
      else throw StreamBufferException("Not supported attribute opcode", "RStreamBuffer::read<AttributeList&>");
    }
  }

  void
  SegmentReaderIterator::extractE(AttributeList& data)
  {
    //reads an empty attribute list
    //skip all data
    if(data.size() > 0)
    {
      //we have already something in the list
      //unfortunately there is no delete method in the attributelist api
      //we create a new attribute list an copy this one to the original
      AttributeList attr;
      extractAttributeListE(attr);
      //copy the attribute lists
      checkAttributeLists(data, attr);
    }
    else
    {
      extractAttributeListE(data);
    }
  }

  void readAttribute_bool( void* address, SegmentReaderIterator& sri )
  {
    sri.extract(*((bool*)address ));
  }

  void readAttribute_char( void* address, SegmentReaderIterator& sri )
  {
    sri.extract(*( (unsigned char*)address ));
  }

  void readAttribute_short( void* address, SegmentReaderIterator& sri )
  {
    sri.extract(*( (uint16_t*)address ));
  }

  void readAttribute_uint( void* address, SegmentReaderIterator& sri )
  {
    sri.extractN(*( (unsigned int*)address ));
  }

  void readAttribute_ul( void* address, SegmentReaderIterator& sri )
  {
    sri.extractN(*( (unsigned long*)address ));
  }

  void readAttribute_ull( void* address, SegmentReaderIterator& sri )
  {
    sri.extractN(*( (unsigned long long*)address ));
  }

  void readAttribute_float( void* address, SegmentReaderIterator& sri )
  {
    sri.extract(*((uint32_t*)address ));
  }

  void readAttribute_double( void* address, SegmentReaderIterator& sri )
  {
    sri.extract(*((uint64_t*)address ));
  }

  void readAttribute_longdouble( void* address, SegmentReaderIterator& sri )
  {
    sri.extract(*((uint128_t*)address ));
  }

  void readAttribute_string( void* address, SegmentReaderIterator& sri )
  {
    sri.extract32(*((std::string*)address ));
  }

  void readAttribute_blob( void* address, SegmentReaderIterator& sri )
  {
    sri.extract(*((coral::Blob*)address ));
  }

  void readAttribute_date( void* address, SegmentReaderIterator& sri )
  {
    sri.extract(*((coral::Date*)address ));
  }

  void readAttribute_time( void* address, SegmentReaderIterator& sri )
  {
    sri.extract(*((coral::TimeStamp*)address ));
  }

  typedef void (*readAttribute_fct)( void*, SegmentReaderIterator& );

  struct attrcft_t
  {
    Attribute* attr;
    readAttribute_fct fct;
  };

  void*
  SegmentReaderIterator::getStructure(AttributeList& data)
  {
    //allocate enough memory
    attrcft_t* structure = (attrcft_t*)malloc((data.size() + 1) * (sizeof(attrcft_t)));
    attrcft_t* pos = structure;
    //initial the iterator, there is not default constructor

    AttributeList::iterator i( data.begin() );
    for(; i != data.end(); ++i )
    {
      Attribute& attr = *i;

      const AttributeSpecification& spec = attr.specification();
      const std::type_info& type = spec.type();

      pos->attr = &attr;
      if      ( type == typeid(bool) ) pos->fct = &readAttribute_bool;
      else if ( type == typeid(char) ) pos->fct = &readAttribute_char;
      else if ( type == typeid(unsigned char) ) pos->fct = &readAttribute_char;
      else if ( type == typeid(signed char) ) pos->fct = &readAttribute_char;
      else if ( type == typeid(short) ) pos->fct = &readAttribute_short;
      else if ( type == typeid(unsigned short) ) pos->fct = &readAttribute_short;
      else if ( type == typeid(int) ) pos->fct = &readAttribute_uint;
      else if ( type == typeid(unsigned int) ) pos->fct = &readAttribute_uint;
      else if ( type == typeid(long) ) pos->fct = &readAttribute_ul;
      else if ( type == typeid(unsigned long) ) pos->fct = &readAttribute_ul;
      else if ( type == typeid(long long) ) pos->fct = &readAttribute_ull;
      else if ( type == typeid(unsigned long long) ) pos->fct = &readAttribute_ull;
      else if ( type == typeid(float) ) pos->fct = &readAttribute_float;
      else if ( type == typeid(double) ) pos->fct = &readAttribute_double;
      else if ( type == typeid(long double) ) pos->fct = &readAttribute_longdouble;
      else if ( type == typeid(std::string) ) pos->fct = &readAttribute_string;
      else if ( type == typeid(coral::Blob) ) pos->fct = &readAttribute_blob;
      else if ( type == typeid(coral::Date) ) pos->fct = &readAttribute_date;
      else if ( type == typeid(coral::TimeStamp) ) pos->fct = &readAttribute_time;
      else throw StreamBufferException("Not supported attribute type", "WStreamBuffer::write<const AttributeList&>");

      pos++;

    }
    pos->attr = 0;

    return structure;
  }

  void
  SegmentReaderIterator::extractD(void* structure)
  {
    //do some casting
    attrcft_t* pos = (attrcft_t*)structure;
    //start the loop
    while(pos->attr)
    {
      //get the attribute
      Attribute& attr = *(pos->attr);

      bool isnull;
      extract( isnull );

      if( isnull )
      {
        attr.setNull( true );
      }
      else
      {
        attr.setNull( false );
        //read data
        if(pos->fct)
          pos->fct( attr.addressOfData() , *this );
      }
      //increase the position
      pos++;
    }
  }


} }
