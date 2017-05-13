#include "SegmentWriterIterator.h"

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
#include "CppTypes.h"

//C++ includes
#include <sstream>

#define VECTORSIZE uint16_t


namespace coral { namespace CoralStubs {

  SegmentWriterIterator::SegmentWriterIterator(CALOpcode opcode, bool cacheable, bool reply, size_t buffersize )
    : m_bufferpos( 0 )
    , m_bufferend( 0 )
    , m_currentbuffer( 0 )
    , m_iterbuffer( 0 )
    , m_opcode( opcode )
    , m_cacheable( cacheable )
    , m_proxy( false )
    , m_reply( reply )
    , m_nocache( getenv ( "CORALSTUBS_NEVERCACHEABLE" ) ? 1 : 0 )
    , m_buffersize( buffersize )
  {
    newbuffer();
    //check if the buffersize is big enough for the smallest type + header
    if(buffersize < (CALPACKET_HEADER_SIZE + 4 + 8))
      throw StreamBufferException("Buffersize is too small", "SegmentWriterIterator()");

    if ( m_nocache > 0 ) m_cacheable = false;
  }

  SegmentWriterIterator::~SegmentWriterIterator()
  {
    std::list<ByteBuffer*>::iterator i;
    for(i = m_buffers.begin(); i != m_buffers.end(); i++ )
    {
      delete *i;
    }

    if(m_iterbuffer) delete m_iterbuffer;

    if(m_currentbuffer) delete m_currentbuffer;
  }

  bool
  SegmentWriterIterator::next()
  {
    if(m_buffers.size() > 0)
    {
      if(m_iterbuffer) delete m_iterbuffer;

      m_iterbuffer = m_buffers.front();

      m_buffers.pop_front();

      return true;
    }

    if(m_iterbuffer) delete m_iterbuffer;

    m_iterbuffer = 0;

    return false;
  }

  const ByteBuffer&
  SegmentWriterIterator::currentBuffer() const
  {
    if(!m_iterbuffer)
      throw StreamBufferException("Called on an empty buffer", "SegmentWriterIterator::currentBuffer()");
    //for debug purpose
    //print();

    return *m_iterbuffer;
  }

  bool
  SegmentWriterIterator::isLast() const
  {
    if(!m_iterbuffer)
      throw StreamBufferException("Called on an empty buffer", "SegmentWriterIterator::isLast()");

    if(m_buffers.size() == 0) return true;

    return false;
  }

  const ByteBuffer&
  SegmentWriterIterator::singleBuffer()
  {
    if(!m_currentbuffer) newbuffer();

    if(m_buffers.size() > 0)
      throw StreamBufferException("Singlebuffer call on a multi buffer", "SegmentWriterIterator::singlebuffer()");

    size_t written = m_bufferpos - m_currentbuffer->data();

    m_currentbuffer->setUsedSize( written );
    //write the header
    CALOpcode opcode = m_opcode;

    if(m_reply) opcode = CALOpcodes::getCALReplyOK( m_opcode );

    CALPacketHeader header(opcode, m_proxy, m_cacheable, written - CALPACKET_HEADER_SIZE);

    memcpy(m_currentbuffer->data(), header.data(), CALPACKET_HEADER_SIZE);
    //set current buffer to zero
    m_iterbuffer = m_currentbuffer;

    m_currentbuffer = 0;

    m_bufferpos = 0;
    m_bufferend = 0;

    return *m_iterbuffer;
  }

  void
  SegmentWriterIterator::print() const
  {
    if(!m_iterbuffer) return;

    std::stringstream msg;

    unsigned char* buffer = m_iterbuffer->data();

    for(unsigned char* i = buffer; i < buffer + m_iterbuffer->usedSize(); i++ ) {
      char c = *(i);

      if((c > 126) || (c < 32))
        msg << "[#" << hexstring(&c,1).substr(2,2) << "]";
      else
        msg << "[ " << c << " ]";

      //  if(!((i + 1) % 20)) msg << "\n";

    }
    std::cout << "StreamBuffer::write::pos " << "\n" << msg.str() << std::endl;

  }

  void
  SegmentWriterIterator::flush()
  {
    if(!m_currentbuffer)
      throw StreamBufferException("Flush not possible on an empty buffer", "SegmentWriterIterator::flush()");
    //first flush the byte buffer
    //get the latest buffer
    //set the final end position
    size_t written = m_bufferpos - m_currentbuffer->data();

    m_currentbuffer->setUsedSize( written );
    //write the header
    CALOpcode opcode = m_opcode;

    if(m_reply) opcode = CALOpcodes::getCALReplyOK( m_opcode );

    CALPacketHeader header(opcode, m_proxy, m_cacheable, written - CALPACKET_HEADER_SIZE);

    memcpy(m_currentbuffer->data(), header.data(), CALPACKET_HEADER_SIZE);
    //add the buffer to the list
    m_buffers.push_back(m_currentbuffer);
    //set current buffer to zero
    m_currentbuffer = 0;

    m_bufferpos = 0;
    m_bufferend = 0;
  }

  size_t
  SegmentWriterIterator::written()
  {
    if(m_currentbuffer)
      return m_bufferpos - m_currentbuffer->data();
    else
      return 0;
  }

  void
  SegmentWriterIterator::setProxy(bool proxy)
  {
    m_proxy = proxy;
  }

  void
  SegmentWriterIterator::exception(const CALOpcode ecode1, const CALOpcode ecode2, const std::string msg1, const std::string msg2, const std::string msg3)
  {
    if(!m_currentbuffer) newbuffer();
    //set the exception format, which containts two chars after the CAL Header

    //delete the whole content
    m_bufferpos = m_currentbuffer->data() + CALPACKET_HEADER_SIZE;

    append(ecode1);
    append(ecode2);
    append16(msg1);
    append16(msg2);
    append16(msg3);

    //flush the the buffer
    size_t written = m_bufferpos - m_currentbuffer->data();

    m_currentbuffer->setUsedSize( written );

    //set the header as exception
    CALOpcode opcode = CALOpcodes::getCALReplyException( m_opcode );

    CALPacketHeader header(opcode, m_proxy, m_cacheable, written - CALPACKET_HEADER_SIZE);

    memcpy(m_currentbuffer->data(), header.data(), CALPACKET_HEADER_SIZE);
    //add the buffer to the list
    m_buffers.push_back(m_currentbuffer);
    //set current buffer to zero
    m_currentbuffer = 0;

    m_bufferpos = 0;
    m_bufferend = 0;
  }

  void
  SegmentWriterIterator::newbuffer()
  {
    //if we have already a current buffer
    //throw an exception
    if(m_currentbuffer)
      throw StreamBufferException("Buffer was already created", "SegmentWriterIterator::newbuffer()");
    //get a new byte buffer
    //FIXME use the bytebuffer factory
    m_currentbuffer = new ByteBuffer( m_buffersize );

    //keep the place for the header
    m_bufferend = m_currentbuffer->data() + m_buffersize;
    //set the pos depending on the header
    m_bufferpos = m_currentbuffer->data() + CALPACKET_HEADER_SIZE;
    //set the pos depending on the version
    //set the CAL procol version
    *m_bufferpos = (unsigned char)0x03;
    m_bufferpos++;
    //set the Format version
    *m_bufferpos = (unsigned char)__SIZEOF_LONG__;
    m_bufferpos++;

    *m_bufferpos = (unsigned char)__SIZEOF_LONGLONG__;
    m_bufferpos++;

    *m_bufferpos = (unsigned char)__SIZEOF_INT__;
    m_bufferpos++;
  }

  void
  SegmentWriterIterator::extend()
  {
    if(m_currentbuffer) flush();
    newbuffer();
  }

  void
  SegmentWriterIterator::append(const bool data)
  {
    if((m_bufferpos + 1) > m_bufferend)
      extend();

    *m_bufferpos = (unsigned char)data;
    m_bufferpos++;
  }

  void
  SegmentWriterIterator::append(const uint16_t data)
  {
    if((m_bufferpos + 2) > m_bufferend)
      extend();

    *((uint16_t*)(m_bufferpos)) = data;
    m_bufferpos += 2;
  }

  void
  SegmentWriterIterator::append(const uint32_t data)
  {
    if((m_bufferpos + 4) > m_bufferend)
      extend();

    *((uint32_t*)(m_bufferpos)) = data;
    m_bufferpos += 4;
  }

  void
  SegmentWriterIterator::append(const uint64_t data)
  {
    if((m_bufferpos + 8) > m_bufferend)
      extend();

    *((uint64_t*)(m_bufferpos)) = data;
    m_bufferpos += 8;
  }

  void
  SegmentWriterIterator::append(const uint128_t& data)
  {
    append(data[0]);
    append(data[1]);
  }

  void
  SegmentWriterIterator::appendN(const unsigned int data)
  {
    append((__TYPEOF_INT__)data);
  }

  void
  SegmentWriterIterator::appendN(const unsigned long data)
  {
    append((__TYPEOF_LONG__)data);
  }

  void
  SegmentWriterIterator::appendN(const unsigned long long data)
  {
    append((__TYPEOF_LONGLONG__)data);
  }

  void
  SegmentWriterIterator::append(const std::string& data, size_t size)
  {
    size_t diff01 = size;
    size_t offset = 0;

    while(m_bufferpos + diff01 > m_bufferend)
    {
      //get the difference between the end and the current pos
      size_t diff02 = m_bufferend - m_bufferpos;

      data.copy( (char*)m_bufferpos, diff02, offset );

      m_bufferpos = m_bufferpos + diff02;

      offset = offset + diff02;

      extend();

      diff01 = diff01 - diff02;
    }

    data.copy( (char*)m_bufferpos, diff01, offset );

    m_bufferpos = m_bufferpos + diff01;
  }

  void
  SegmentWriterIterator::append16(const std::string& data)
  {
    //get the size of the string
    uint16_t size = data.size();
    //write the size into the buffer
    append( size );

    append(data, size);
  }

  void
  SegmentWriterIterator::append32(const std::string& data)
  {
    //get the size of the string
    uint32_t size = data.size();
    //write the size into the buffer
    append( size );

    append(data, size);
  }

  void
  SegmentWriterIterator::append(const Blob& data)
  {
    //get the size of the string
    size_t size = data.size();
    //write the size into the buffer
    append((uint32_t)size);

    size_t diff01 = size;
    size_t offset = 0;

    while(m_bufferpos + diff01 > m_bufferend)
    {
      //get the difference between the end and the current pos
      size_t diff02 = m_bufferend - m_bufferpos;

      memcpy(m_bufferpos, (char*)data.startingAddress() + offset, diff02);

      m_bufferpos = m_bufferpos + diff02;

      offset = offset + diff02;

      extend();

      diff01 = diff01 - diff02;
    }

    memcpy(m_bufferpos, (char*)data.startingAddress() + offset, diff01);

    m_bufferpos = m_bufferpos + diff01;

  }

  void
  SegmentWriterIterator::append(const coral::Date& data)
  {
    append((uint16_t)data.year());
    append((uint16_t)data.month());
    append((uint16_t)data.day());
  }

  void
  SegmentWriterIterator::append(const coral::TimeStamp& data)
  {
    append((uint16_t)data.year());
    append((uint16_t)data.month());
    append((uint16_t)data.day());
    append((uint16_t)data.hour());
    append((uint16_t)data.minute());
    append((uint16_t)data.second());
    append((uint64_t)data.nanosecond());
  }

  void
  SegmentWriterIterator::append(const CALOpcode data)
  {
    if((m_bufferpos + 1) > m_bufferend)
      extend();

    //FIXME here must be a definition of the size of cal opcode
    *m_bufferpos = (unsigned char)data;
    m_bufferpos++;
  }

  void
  SegmentWriterIterator::append(const std::vector<std::string>& data)
  {
    VECTORSIZE size = data.size();
    append(size);
    for(VECTORSIZE i = 0; i < size; i++ )
      append16(data[i]);
  }

  void
  SegmentWriterIterator::append(const std::set<std::string>& data)
  {
    VECTORSIZE size = data.size();
    append(size);
    std::set<std::string>::const_iterator i;
    for(i = data.begin(); i != data.end(); i++ )
      append16(*i);
  }

  void
  SegmentWriterIterator::append(const std::vector< std::pair<std::string, std::string> >& data)
  {
    VECTORSIZE size = data.size();
    append(size);
    std::vector< std::pair< std::string, std::string > >::const_iterator i;
    for(i = data.begin(); i != data.end(); i++ )
    {
      append16(i->first);
      append16(i->second);
    }
  }

  void
  SegmentWriterIterator::append(const std::map< std::string, std::string >& data)
  {
    VECTORSIZE size = data.size();
    append(size);
    std::map< std::string, std::string >::const_iterator i;
    for(i = data.begin(); i != data.end(); i++ )
    {
      append16(i->first);
      append16(i->second);
    }
  }

  void
  SegmentWriterIterator::append(const TableDescription& data)
  {
    //write the table name
    append16(data.name());
    //write the table type
    append16(data.type());
    //write the table space name
    append16(data.tableSpaceName());
    //write the columns
    VECTORSIZE columns = data.numberOfColumns();
    append(columns);
    for(VECTORSIZE i = 0; i < columns; i++ )
    {
      const IColumn& column = data.columnDescription( i );
      append16(column.name());
      //FIXME use as in attribute list codes
      append16(column.type());
      append((uint32_t)column.size());
      append(column.isSizeFixed());
      append16(column.tableSpaceName());
    }
    //write unique constraints
    VECTORSIZE constraints = data.numberOfUniqueConstraints();
    append(constraints);
    for(VECTORSIZE i = 0; i < constraints; i++ )
    {
      const IUniqueConstraint& conststr = data.uniqueConstraint( i );
      append(conststr.columnNames());
      append16(conststr.name());
      append16(conststr.tableSpaceName());
    }
    //write primary keys
    bool isprikey = data.hasPrimaryKey();
    append(isprikey);
    if(isprikey)
    {
      const IPrimaryKey& prikey = data.primaryKey();
      append(prikey.columnNames());
      append16(prikey.tableSpaceName());
    }
    //write foreign keys
    VECTORSIZE foreignkeys = data.numberOfForeignKeys();
    append(foreignkeys);
    for(VECTORSIZE i = 0; i < foreignkeys; i++ )
    {
      const IForeignKey& forkey = data.foreignKey( i );
      append(forkey.columnNames());
      append16(forkey.name());
      append16(forkey.referencedTableName());
      append(forkey.referencedColumnNames());
    }
    //write indexes
    VECTORSIZE indexes = data.numberOfIndices();
    append(indexes);
    for(VECTORSIZE i = 0; i < indexes; i++ )
    {
      const IIndex& ind = data.index( i );
      append(ind.columnNames());
      append16(ind.name());
      append(ind.isUnique());
      append16(ind.tableSpaceName());
    }
  }

  void
  SegmentWriterIterator::append(const std::map< std::string, TableDescription >& data)
  {
    VECTORSIZE size = data.size();
    append(size);
    std::map< std::string, TableDescription >::const_iterator i;
    for(i = data.begin(); i != data.end(); i++ )
    {
      append16(i->first);
      append(i->second);
    }
  }

  void
  SegmentWriterIterator::append(const QueryDefinition& data)
  {
    append16(data.getSchemaName());
    //default values
    append(data.getDistinct());
    append(data.getOutputList());
    append(data.getTableList());
    append16(data.getCondition());
    appendV(data.getConditionData()); //attribute list
    append16(data.getGroupBy());
    append(data.getOrderList());
    //special values
    const std::map< std::string, QueryDefinition >& map = data.getSubQueries();
    append((VECTORSIZE)map.size());
    std::map< std::string, QueryDefinition >::const_iterator i;
    for(i = map.begin(); i != map.end(); i++ )
    {
      append16(i->first);
      append(i->second);
    }

    const std::pair<int,int>* limits = data.getRowLimitAndOffset();
    if(limits == NULL)
      append(false);
    else
    {
      append(true);
      append((uint32_t)limits->first);
      append((uint32_t)limits->second);
    }

    const std::pair<IQueryDefinition::SetOperation, QueryDefinition>* operations = data.getSetOperation();
    if(operations == NULL)
      append(false);
    else
    {
      append(true);
      append(operations->first);
      append(operations->second);
    }
  }

  void
  SegmentWriterIterator::append(const IQueryDefinition::SetOperation& data)
  {
    if(data == IQueryDefinition::Union)
      append((CALOpcode)0x01);
    else if(data == IQueryDefinition::Minus)
      append((CALOpcode)0x02);
    else if(data == IQueryDefinition::Intersect)
      append((CALOpcode)0x03);
    else
      append((CALOpcode)0x00);
  }

  template<class T> inline
  void
  SegmentWriterIterator::appendAttribute(const coral::CALOpcode& opcode, const bool isnull, const Attribute& attr)
  {
    append(opcode);
    if(!isnull) append( *( (const T*)attr.addressOfData() ) );
  }

  template<> inline
  void
  SegmentWriterIterator::appendAttribute<std::string>(const CALOpcode& opcode, const bool isnull, const Attribute& attr)
  {
    append(opcode);
    if(!isnull) append32( *( (const std::string*)attr.addressOfData() ) );
  }

  template<class T> inline
  void
  SegmentWriterIterator::appendAttributeN(const coral::CALOpcode& opcode, const bool isnull, const Attribute& attr)
  {
    append(opcode);
    if(!isnull) appendN( *( (const T*)attr.addressOfData() ) );
  }

  void
  SegmentWriterIterator::appendV(const AttributeList& data)
  {
    VECTORSIZE size = data.size();
    append(size);
    //initial the iterator, there is not default constructor
    AttributeList::const_iterator i( data.begin() );
    for(; i != data.end(); ++i )
    {
      const Attribute& attr = *i;
      const AttributeSpecification& spec = attr.specification();
      append16(spec.name());
      const std::type_info& type = spec.type();
      bool isnull = attr.isNull();
      append(isnull);

      if      ( type == typeid(bool) ) appendAttribute<bool>(0x01, isnull, attr);
      else if ( type == typeid(char) ) appendAttribute<unsigned char>(0x02, isnull, attr);
      else if ( type == typeid(unsigned char) ) appendAttribute<unsigned char>(0x03, isnull, attr);
      else if ( type == typeid(signed char) ) appendAttribute<unsigned char>(0x04, isnull, attr);
      else if ( type == typeid(short) ) appendAttribute<uint16_t>(0x05, isnull, attr);
      else if ( type == typeid(unsigned short) ) appendAttribute<uint16_t>(0x06, isnull, attr);
      else if ( type == typeid(int) ) appendAttributeN<unsigned int>(0x07, isnull, attr);
      else if ( type == typeid(unsigned int) ) appendAttributeN<unsigned int>(0x08, isnull, attr);
      else if ( type == typeid(long) ) appendAttributeN<unsigned long>(0x09, isnull, attr);
      else if ( type == typeid(unsigned long) ) appendAttributeN<unsigned long>(0x0a, isnull, attr);
      else if ( type == typeid(long long) ) appendAttributeN<unsigned long long>(0x0b, isnull, attr);
      else if ( type == typeid(unsigned long long) ) appendAttributeN<unsigned long long>(0x0c, isnull, attr);
      else if ( type == typeid(float) ) appendAttribute<uint32_t>(0x0d, isnull, attr);
      else if ( type == typeid(double) ) appendAttribute<uint64_t>(0x0e, isnull, attr);
      else if ( type == typeid(long double) ) appendAttribute<uint128_t>(0x0f, isnull, attr);
      else if ( type == typeid(std::string) ) appendAttribute<std::string>(0x10, isnull, attr);
      else if ( type == typeid(coral::Blob) ) appendAttribute<coral::Blob>(0x11, isnull, attr);
      else if ( type == typeid(coral::Date) ) appendAttribute<coral::Date>(0x12, isnull, attr);
      else if ( type == typeid(coral::TimeStamp) ) appendAttribute<coral::TimeStamp>(0x13, isnull, attr);
      else throw StreamBufferException("Not supported attribute type", "WStreamBuffer::write<const AttributeList&>");
    }
  }

  void
  SegmentWriterIterator::appendE(const AttributeList& data)
  {
    //write onlye the properties no data
    VECTORSIZE size = data.size();
    append(size);
    //initial the iterator, there is not default constructor
    AttributeList::const_iterator i( data.begin() );
    for(; i != data.end(); ++i )
    {
      //get the attribute
      const Attribute& attr = *i;
      //get the specification
      const AttributeSpecification& spec = attr.specification();
      append16(spec.name());
      const std::type_info& type = spec.type();

      if      ( type == typeid(bool) ) append((CALOpcode)0x01);
      else if ( type == typeid(char) ) append((CALOpcode)0x02);
      else if ( type == typeid(unsigned char) ) append((CALOpcode)0x03);
      else if ( type == typeid(signed char) ) append((CALOpcode)0x04);
      else if ( type == typeid(short) ) append((CALOpcode)0x05);
      else if ( type == typeid(unsigned short) ) append((CALOpcode)0x06);
      else if ( type == typeid(int) ) append((CALOpcode)0x07);
      else if ( type == typeid(unsigned int) ) append((CALOpcode)0x08);
      else if ( type == typeid(long) ) append((CALOpcode)0x09);
      else if ( type == typeid(unsigned long) ) append((CALOpcode)0x0a);
      else if ( type == typeid(long long) ) append((CALOpcode)0x0b);
      else if ( type == typeid(unsigned long long) ) append((CALOpcode)0x0c);
      else if ( type == typeid(float) ) append((CALOpcode)0x0d);
      else if ( type == typeid(double) ) append((CALOpcode)0x0e);
      else if ( type == typeid(long double) ) append((CALOpcode)0x0f);
      else if ( type == typeid(std::string) ) append((CALOpcode)0x10);
      else if ( type == typeid(coral::Blob) ) append((CALOpcode)0x11);
      else if ( type == typeid(coral::Date) ) append((CALOpcode)0x12);
      else if ( type == typeid(coral::TimeStamp) ) append((CALOpcode)0x13);
      else throw StreamBufferException("Not supported attribute type", "WStreamBuffer::write<const AttributeList&>");
    }
  }

  void writeAttribute_bool( const void* address, SegmentWriterIterator& swi )
  {
    swi.append(*((bool*)address ));
  }

  void writeAttribute_char( const void* address, SegmentWriterIterator& swi )
  {
    swi.append(*( (unsigned char*)address ));
  }

  void writeAttribute_short( const void* address, SegmentWriterIterator& swi )
  {
    swi.append(*( (uint16_t*)address ));
  }

  void writeAttribute_uint( const void* address, SegmentWriterIterator& swi )
  {
    swi.appendN(*( (const unsigned int*)address ));
  }

  void writeAttribute_ul( const void* address, SegmentWriterIterator& swi )
  {
    swi.appendN(*( (const unsigned long*)address ));
  }

  void writeAttribute_ull( const void* address, SegmentWriterIterator& swi )
  {
    swi.appendN(*( (const unsigned long long*)address ));
  }

  void writeAttribute_float( const void* address, SegmentWriterIterator& swi )
  {
    swi.append(*((uint32_t*)address ));
  }

  void writeAttribute_double( const void* address, SegmentWriterIterator& swi )
  {
    swi.append(*((uint64_t*)address ));
  }

  void writeAttribute_longdouble( const void* address, SegmentWriterIterator& swi )
  {
    swi.append(*((uint128_t*)address ));
  }

  void writeAttribute_string( const void* address, SegmentWriterIterator& swi )
  {
    swi.append32(*((std::string*)address ));
  }

  void writeAttribute_blob( const void* address, SegmentWriterIterator& swi )
  {
    swi.append(*((const coral::Blob*)address));
  }

  void writeAttribute_date( const void* address, SegmentWriterIterator& swi )
  {
    swi.append(*((coral::Date*)address ));
  }

  void writeAttribute_time( const void* address, SegmentWriterIterator& swi )
  {
    swi.append(*((coral::TimeStamp*)address ));
  }

  typedef void (*writeAttribute_fct)( const void*, SegmentWriterIterator& );

  struct attrcft_t
  {
    const Attribute* attr;
    writeAttribute_fct fct;
  };

  static const size_t size_attrcft_t = sizeof(attrcft_t);

  void*
  SegmentWriterIterator::getStructure(const AttributeList& data)
  {
    //allocate enough memory
    attrcft_t* structure = (attrcft_t*)malloc( (data.size() + 1) * size_attrcft_t );

    attrcft_t* pos = structure;
    //initial the iterator, there is no default constructor
    AttributeList::const_iterator i( data.begin() );
    for(; i != data.end(); ++i )
    {
      const Attribute& attr = *i;

      const AttributeSpecification& spec = attr.specification();
      const std::type_info& type = spec.type();

      pos->attr = &attr;

      if      ( type == typeid(bool) ) pos->fct = &writeAttribute_bool;
      else if ( type == typeid(char) ) pos->fct = &writeAttribute_char;
      else if ( type == typeid(unsigned char) ) pos->fct = &writeAttribute_char;
      else if ( type == typeid(signed char) ) pos->fct = &writeAttribute_char;
      else if ( type == typeid(short) ) pos->fct = &writeAttribute_short;
      else if ( type == typeid(unsigned short) ) pos->fct = &writeAttribute_short;
      else if ( type == typeid(int) ) pos->fct = &writeAttribute_uint;
      else if ( type == typeid(unsigned int) ) pos->fct = &writeAttribute_uint;
      else if ( type == typeid(long) ) pos->fct = &writeAttribute_ul;
      else if ( type == typeid(unsigned long) ) pos->fct = &writeAttribute_ul;
      else if ( type == typeid(long long) ) pos->fct = &writeAttribute_ull;
      else if ( type == typeid(unsigned long long) ) pos->fct = &writeAttribute_ull;
      else if ( type == typeid(float) ) pos->fct = &writeAttribute_float;
      else if ( type == typeid(double) ) pos->fct = &writeAttribute_double;
      else if ( type == typeid(long double) ) pos->fct = &writeAttribute_longdouble;
      else if ( type == typeid(std::string) ) pos->fct = &writeAttribute_string;
      else if ( type == typeid(coral::Blob) ) pos->fct = &writeAttribute_blob;
      else if ( type == typeid(coral::Date) ) pos->fct = &writeAttribute_date;
      else if ( type == typeid(coral::TimeStamp) ) pos->fct = &writeAttribute_time;
      else throw StreamBufferException("Not supported attribute type", "WStreamBuffer::write<const AttributeList&>");

      pos++;

    }

    pos->attr = 0;
    return structure;
  }


  void
  SegmentWriterIterator::appendD(void* structure )
  {
    //do some casting
    attrcft_t* pos = (attrcft_t*)structure;
    //start the loop
    while(pos->attr)
    {
      //get the attribute
      const Attribute* attr = pos->attr;

      if(attr->isNull())
      {
        append( true );
      }
      else
      {
        append( false );
        //write data
        (*pos->fct)( attr->addressOfData() , *this );
      }

      //increase the position
      pos++;
    }
  }

  void
  SegmentWriterIterator::appendD(void* structure, const AttributeList& data)
  {
    //do some casting
    attrcft_t* pos = (attrcft_t*)structure;

    AttributeList::const_iterator i( data.begin() );
    for(; i != data.end(); ++i )
    {
      const Attribute& attr = *i;

      if(attr.isNull())
      {
        append( true );
      }
      else
      {
        append( false );
        //write data
        (*pos->fct)( attr.addressOfData() , *this );
      }

      pos++;
    }
  }


} }
