#ifndef CORALBASE_IHANDLE_H
#define CORALBASE_IHANDLE_H

#include "RefCounted.h"

namespace coral {


  template< typename T > class IHandle {
  public:
    /// Constructor
    IHandle( T* object = 0 ) :
      m_object( 0 )
    {
      this->operator=( object );
    }

    /// Destructor
    ~IHandle() {
      if ( m_object )
        dynamic_cast< RefCounted* >( m_object )->removeReference();
    }

    /// Copy constructor
    IHandle( const IHandle& rhs ) :
      m_object( 0 )
    {
      if( rhs.m_object )
      {
        m_object = rhs.m_object;
        dynamic_cast< RefCounted* >( m_object )->addReference();
      }
    }

    /// Assignment operator
    IHandle& operator=( const IHandle& rhs )
    {
      if( this != &rhs )
      {
        if ( m_object )
        {
          dynamic_cast< RefCounted* >( m_object )->removeReference();
          m_object = 0;
        }

        if( rhs.m_object )
        {
          m_object = rhs.m_object;
          dynamic_cast< RefCounted* >( m_object )->addReference();
        }
      }

      return *this;
    }

    /// Assignment operator from a pointer. Steals a reference...
    IHandle operator=( T* object )
    {
      if ( object && dynamic_cast<RefCounted*>( object ) )
      {
        if ( m_object )
        {
          dynamic_cast< RefCounted* >( m_object )->removeReference();
          m_object = 0;
        }

        m_object = object;
      }

      return *this;
    }

    /// The dereference operators
    T& operator*() {
      return *m_object;
    }

    const T& operator*() const {
      return *m_object;
    }

    T* operator->() {
      return m_object;
    }

    const T* operator->() const {
      return m_object;
    }

    T* get() {
      return m_object;
    }

    const T* get() const {
      return m_object;
    }

    /// Checks the validity of the pointer
    bool isValid(){
      return ( m_object != 0 );
    }

  private:
    /// The object
    T* m_object;
  };

}

#endif
