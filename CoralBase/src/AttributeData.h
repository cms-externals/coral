#ifndef CORALBASE_ATTRIBUTEDATA_H
#define CORALBASE_ATTRIBUTEDATA_H

#include <iosfwd>
#include <iostream>

namespace coral {

  /**
     @class AttributeData AttributeData.h

     Base class of the structure holding the actual data of an attribute
  */

  class AttributeData {
  protected:
    /// Constructor
    AttributeData() : m_counter( 1 ), m_sharedData( 0 ) {}

    /// Destructor
    virtual ~AttributeData() {
      if ( m_sharedData ) m_sharedData->release();
    }

  public:
    /// Copies data from an external address. By definition this is an unsafe operation.
    virtual void setValueFromAddress( const void* externalAddress ) = 0;

    /// Copies data to an external address. By definition this is an unsafe operation.
    virtual void copyValueToAddress( void* externalAddress ) const = 0;

    /// Returns the addess of the actual data.
    virtual void* addressOfData() = 0;
    virtual const void* addressOfData() const = 0;

    /// Returns the size of the data
    virtual int size() const = 0;

    /// Equal operator
    virtual bool operator==( const AttributeData& rhs ) const = 0;

    /// Comparison operator
    virtual bool operator!=( const AttributeData& rhs ) const = 0;

    /// Shares the data with another Attribute.
    void shareData( const AttributeData& sourceData )
    {
      if ( &sourceData != this) {
        if (m_sharedData) {
          if (this->m_sharedData != sourceData.m_sharedData) {
            m_sharedData->shareData(sourceData);
          }
        } else {
          m_sharedData = const_cast< AttributeData*>( &sourceData );
          m_sharedData->addRef();
        }
      }
    }


    /// Binds external data
    virtual void bind( void* externalAddress ) = 0;

    /// Increments the ref counter
    void addRef() const { ++m_counter; }

    /// Decrements the ref counter
    void release() const {
      --m_counter;
      if ( m_counter == 0 ) delete this;
    }

    /// Dumps data to an output stream
    virtual std::ostream& toOutputStream( std::ostream& os ) const = 0;

    /// Sets the null-ness of the variable
    virtual void setNull( bool isVariableNull ) = 0;

    /// Retrieves the null-ness of the variable
    virtual bool isNull() const = 0;

    /// Check if the variable is producing a NOT NULL value in ALL of the backends supported.
    virtual bool isValidData() const = 0;

  private:
    /// Reference counter
    mutable int m_counter;

  protected:
    AttributeData* m_sharedData;
  };

}

#endif
