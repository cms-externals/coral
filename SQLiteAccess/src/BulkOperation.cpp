#include <iostream>
#include "BulkOperation.h"
#include "SessionProperties.h"
#include "DomainProperties.h"
#include "SQLiteStatement.h"

#include "RelationalAccess/SchemaException.h"

#include "CoralBase/AttributeList.h"
#include "CoralBase/Attribute.h"
#include "CoralBase/AttributeSpecification.h"
#include "CoralBase/Date.h"
#include "CoralBase/TimeStamp.h"
#include "CoralBase/Blob.h"
#include "CoralBase/MessageStream.h"

#include "CoralKernel/Service.h"

coral::SQLiteAccess::BulkOperation::BulkOperation( boost::shared_ptr<const SessionProperties> properties,
                                                   const coral::AttributeList& inputBuffer,
                                                   int cacheSize,
                                                   const std::string& statement ) :
  m_properties( properties ),
  m_rowCacheSize( cacheSize ),
  m_inputRow( inputBuffer ),
  m_sqlstatement( statement ),
  m_dataCache(),
  m_rowsInCache( 0 )
{

  unsigned int specsize=m_inputRow.size();
  for(size_t i=0; i<specsize; ++i ) {
    std::vector< std::pair< boost::any, bool > >* myvec=new std::vector< std::pair< boost::any, bool > >();
    myvec->reserve(m_rowCacheSize);
    m_dataCache.push_back(myvec);
  }
}

coral::SQLiteAccess::BulkOperation::~BulkOperation()
{

  this->reset();
}

void
coral::SQLiteAccess::BulkOperation::processNextIteration(){

  /*#ifdef _DEBUG
    coral::MessageStream log( m_properties->domainProperties().service(),
    m_properties->domainProperties().service()->name(),
    coral::Nil );
    log<<coral::Debug<<"SQLiteAccess::SQLiteBulkOperation::processNextIteration"<<coral::MessageStream::endmsg;
    #endif
  */
  ++m_rowsInCache;
  size_t colidx=0;
  for ( coral::AttributeList::const_iterator iAttribute=m_inputRow.begin();
        iAttribute!=m_inputRow.end(); ++iAttribute, ++colidx) {
    //std::string myname=iAttribute->specification().name();
    std::vector< std::pair< boost::any, bool > >& mycol=*(m_dataCache[colidx]);
    int stub = 0;
    if ( iAttribute->isNull() ) {
      mycol.push_back( std::make_pair( stub, true ) );
    }
    else {
      const std::type_info& attributeType=iAttribute->specification().type();
      if( attributeType==typeid(float) ) {
        float me=iAttribute->data<float>();
        mycol.push_back(std::make_pair(me,false));
      }else if( attributeType == typeid(double) ) {
        double me=iAttribute->data<double>();
        mycol.push_back(std::make_pair(me,false));
      }else if ( attributeType == typeid(long double) ) {
        long double me=iAttribute->data<long double>();
        mycol.push_back(std::make_pair(me,false));
      }else if ( attributeType == typeid(int) ) {
        int me=iAttribute->data<int>();
        mycol.push_back(std::make_pair(me,false));
      }else if ( attributeType == typeid(unsigned int) ) {
        unsigned int me=iAttribute->data<unsigned int>();
        mycol.push_back(std::make_pair(me,false));
      }else if ( attributeType == typeid(short) ) {
        short me=iAttribute->data<short>();
        mycol.push_back(std::make_pair(me,false));
      }else if ( attributeType == typeid(unsigned short) ) {
        unsigned short me=iAttribute->data<unsigned short>();
        mycol.push_back(std::make_pair(me,false));
      }else if ( attributeType == typeid(long long) ) {
        long long me=iAttribute->data<long long>();
        mycol.push_back(std::make_pair(me,false));
      }else if ( attributeType == typeid(unsigned long long) ) {
        unsigned long long me=iAttribute->data<unsigned long long>();
        mycol.push_back(std::make_pair(me,false));
      }else if ( attributeType == typeid(long) ) {
        long me=iAttribute->data<long>();
        mycol.push_back(std::make_pair(me,false));
      }else if ( attributeType == typeid(unsigned long) ) {
        unsigned long me=iAttribute->data<unsigned long>();
        mycol.push_back(std::make_pair(me,false));
      }else if ( attributeType == typeid(bool) ) {
        bool me=iAttribute->data<bool>();
        mycol.push_back(std::make_pair(me,false));
      }else if ( attributeType == typeid(char) ) {
        char me=iAttribute->data<char>();
        mycol.push_back(std::make_pair(me,false));
      }else if ( attributeType == typeid(unsigned char) ) {
        unsigned char me=iAttribute->data<unsigned char>();
        mycol.push_back(std::make_pair(me,false));
      }else if ( attributeType == typeid(std::string) ) {
        std::string me=iAttribute->data<std::string>();
        mycol.push_back(std::make_pair(me,false));
      }else if( attributeType == typeid(coral::Date) ) {
        coral::Date me=iAttribute->data<coral::Date>();
        mycol.push_back(std::make_pair(me,false));
      }else if( attributeType == typeid(coral::TimeStamp) ) {
        coral::TimeStamp me=iAttribute->data<coral::TimeStamp>();
        mycol.push_back(std::make_pair(me,false));
      }else if( attributeType == typeid(coral::Blob) ) {
        coral::Blob me=iAttribute->data<coral::Blob>();
        mycol.push_back(std::make_pair(me,false));
      }else{
        coral::MessageStream log( m_properties->domainProperties().service()->name() );
        log<<coral::Error << "Unsupported type : "<<iAttribute->specification().typeName()<< coral::MessageStream::endmsg;
        this->reset();
        throw coral::DataEditorException( m_properties->domainProperties().service()->name(),"Could not prepare a new bulk operation","IBulkOperation" );
      }
    }
  }
  if ( m_rowsInCache >= m_rowCacheSize ) {
    return this->flush();
  }
}

void
coral::SQLiteAccess::BulkOperation::flush(){

  /*#ifdef _DEBUG
  coral::MessageStream log( m_properties->domainProperties().service(),
                           m_properties->domainProperties().service()->name(),
                           coral::Nil );
  log<<coral::Debug<<"SQLiteAccess::BulkOperation::flush"<<coral::MessageStream::endmsg;
#endif
  */
  // Define, prepare and bind a statement
  if ( m_rowsInCache == 0 || m_dataCache.empty() || m_dataCache.size()==0 ) return;
  coral::SQLiteAccess::SQLiteStatement statement(m_properties);
  if(!statement.prepare(m_sqlstatement) ) {
    coral::MessageStream log( m_properties->domainProperties().service()->name() );
    log<<coral::Error<<"failed prepare statement"<<coral::MessageStream::endmsg;
    this->reset();
    throw coral::DataEditorException( m_properties->domainProperties().service()->name(),"Could not prepare a new bulk operation","IBulkOperation" );
  }
  for( size_t irow=0; irow<m_rowsInCache; ++irow) {
    coral::AttributeList myrow;
    size_t icol=0;
    for( coral::AttributeList::const_iterator iAttr=m_inputRow.begin();
         iAttr!=m_inputRow.end(); ++iAttr, ++icol) {
      std::string myname=iAttr->specification().name();
      std::vector<std::pair< boost::any, bool > >& mycols=*(m_dataCache[icol]);
      boost::any value=mycols[irow].first;
      const std::type_info& attributeType=iAttr->specification().type();
      myrow.extend(myname,attributeType);
      try{
        if ( mycols[irow].second ) {
          myrow[myname].setNull();
        }
        else {
          if(attributeType==typeid(float) ) {
            float me=boost::any_cast<float>(value);
            myrow[myname].data<float>()=me;
          }else if( attributeType== typeid(double) ) {
            double me=boost::any_cast<double>(value);
            myrow[myname].data<double>()=me;
          }else if ( attributeType == typeid(long double) ) {
            long double me=boost::any_cast<long double>(value);
            myrow[myname].data<long double>()=me;
          }else if ( attributeType == typeid(int) ) {
            int me=boost::any_cast<int>(value);;
            myrow[myname].data<int>()=me;
          }else if ( attributeType == typeid(unsigned int) ) {
            unsigned int me=boost::any_cast<unsigned int>(value);;
            myrow[myname].data<unsigned int>()=me;
          }else if ( attributeType == typeid(short) ) {
            short me=boost::any_cast<short>(value);;
            myrow[myname].data<short>()=me;
          }else if ( attributeType == typeid(unsigned short) ) {
            unsigned short me=boost::any_cast<unsigned short>(value);;
            myrow[myname].data<unsigned short>()=me;
          }else if ( attributeType == typeid(long long) ) {
            long long me=boost::any_cast< long long >(value);
            myrow[myname].data< long long >()=me;
          }else if ( attributeType == typeid(unsigned long long) ) {
            unsigned long long me=boost::any_cast< unsigned long long >(value);
            myrow[myname].data< unsigned long long >()=me;
          }else if ( attributeType == typeid(long) ) {
            long me=boost::any_cast<long>(value);;
            myrow[myname].data<long>()=me;
          }else if ( attributeType == typeid(unsigned long) ) {
            unsigned long me=boost::any_cast<unsigned long>(value);;
            myrow[myname].data<unsigned long>()=me;
          }else if ( attributeType == typeid(bool) ) {
            bool me=boost::any_cast< bool >(value);;
            myrow[myname].data<bool>()=me;
          }else if ( attributeType == typeid(char) ) {
            char me=boost::any_cast< char >(value);;
            myrow[myname].data<char>()=me;
          }else if ( attributeType == typeid(unsigned char) ) {
            unsigned char me=boost::any_cast< unsigned char >(value);;
            myrow[myname].data<unsigned char>()=me;
          }else if ( attributeType == typeid(std::string) ) {
            std::string me=boost::any_cast< std::string >(value);;
            myrow[myname].data<std::string>()=me;
          }else if ( attributeType == typeid(coral::Date) ) {
            coral::Date me=boost::any_cast< coral::Date >(value);;
            myrow[myname].data<coral::Date>()=me;
          }else if ( attributeType == typeid(coral::TimeStamp) ) {
            coral::TimeStamp me=boost::any_cast< coral::TimeStamp >(value);;
            myrow[myname].data<coral::TimeStamp>()=me;
          }else if ( attributeType == typeid(coral::Blob) ) {
            coral::Blob me=boost::any_cast< coral::Blob >(value);;
            myrow[myname].data<coral::Blob>()=me;
          }else{
            coral::MessageStream log( m_properties->domainProperties().service()->name() );
            log<<coral::Error << "Unsupported type : "<<iAttr->specification().typeName()<< coral::MessageStream::endmsg;
            this->reset();
            throw coral::DataEditorException( m_properties->domainProperties().service()->name(),"Could not prepare a new bulk operation","IBulkOperation" );
          }
        }
      }catch(const boost::bad_any_cast& e) {
        coral::MessageStream log( m_properties->domainProperties().service()->name() );
        log<<coral::Error<<e.what()<<coral::MessageStream::endmsg;
        this->reset();
        throw coral::DataEditorException( m_properties->domainProperties().service()->name(),"Could not prepare a new bulk operation","IBulkOperation" );
      }
    }
    //myrow.toOutputStream( std::cout ) << std::endl;
    if( !statement.bind(myrow) ) {
      coral::MessageStream log( m_properties->domainProperties().service()->name() );
      log<<coral::Error<<"failed bind statement"<<coral::MessageStream::endmsg;
      this->reset();
      throw coral::DataEditorException( m_properties->domainProperties().service()->name(),"Could not prepare a new bulk operation","IBulkOperation" );
    }
    if( !statement.execute() ) {
      coral::MessageStream log( m_properties->domainProperties().service()->name() );
      log<<coral::Error<<"failed insert"<< coral::MessageStream::endmsg;
      this->reset();
      throw coral::DataEditorException( m_properties->domainProperties().service()->name(),"Could not prepare a new bulk operation","IBulkOperation" );
    }
    if( !statement.reset() ) {
      coral::MessageStream log( m_properties->domainProperties().service()->name() );
      log<<coral::Error<<"failed reset statement"<< coral::MessageStream::endmsg;
      this->reset();
      throw coral::DataEditorException( m_properties->domainProperties().service()->name(),"Could not prepare a new bulk operation","IBulkOperation" );
    }
  }
  // Clear the cache.
  for(std::vector< std::vector< std::pair< boost::any, bool > >*>::iterator it=m_dataCache.begin(); it!=m_dataCache.end(); ++it) {
    (*it)->clear();
  }
  m_rowsInCache=0;
}

void
coral::SQLiteAccess::BulkOperation::reset()
{

  /*#ifdef _DEBUG
  coral::MessageStream log( m_properties->domainProperties().service(),
                           m_properties->domainProperties().service()->name(),
                           coral::Nil );
  log<<coral::Debug<<"SQLiteAccess::BulkOperation::reset"<<coral::MessageStream::endmsg;
#endif
  */
  for(std::vector< std::vector< std::pair< boost::any, bool > >*>::iterator it=m_dataCache.begin(); it!=m_dataCache.end(); ++it) {
    (*it)->clear();
    delete *it;
  }
  m_dataCache.clear();
  m_rowCacheSize = 0;
  //  m_inputRow.reset();
  m_sqlstatement="";
  m_rowsInCache = 0;
}
