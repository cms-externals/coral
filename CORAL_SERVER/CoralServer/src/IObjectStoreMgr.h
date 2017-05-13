// $Id: IObjectStoreMgr.h,v 1.2.2.1 2010/12/20 09:10:10 avalassi Exp $
#ifndef CORALSERVER_IOBJECTSTOREMGR_H
#define CORALSERVER_IOBJECTSTOREMGR_H 1

// Include files
#include <map>
#include <typeinfo>
#include "CoralBase/Exception.h"
#include "CoralServerBase/ICoralFacade.h"

namespace coral
{

  namespace CoralServer
  {

    /** @class IObjectStoreMgr
     *
     *  @author Andrea Valassi
     *  @date   2007-11-26 (python)
     *  @date   2008-01-22 (C++)
     */

    class IObjectStoreMgr
    {

    public:

      /// Destructor.
      virtual ~IObjectStoreMgr() {}

      /// Register an object in the store and return its ID.
      /// Indicate a parent which cannot be deleted before it
      /// (parentID=0 means that the object itself is a parent).
      /// Throw an exception if no parent is found with that ID.
      /// NB: Transfer ownership to the object store.
      template<class T> Token registerObject( T* pObj,
                                              Token parentID )
      {
        const std::type_info* pType = &typeid( T );
        if ( !isSupportedType( *pType ) )
          throw Exception( "Object type is not supported",
                           "IObjectStoreMgr::registerObject<T>",
                           "coral::CoralServer" );
        ObjectAndType obj( pObj, pType );
        return registerObject( obj, parentID );
      }

      /// Find an object in the store, given its ID.
      template<class T> T* findObject( Token ID )
      {
        ObjectAndType obj = findObject( ID );
        void* pObj = obj.first;
        const std::type_info& type = *(obj.second);
        if ( type != typeid( T ) )
          throw Exception( "Object was registered with a different type",
                           "IObjectStoreMgr::findObject<T>",
                           "coral::CoralServer" );
        T* pTObj;
        try
        {
          pTObj = static_cast<T*>( pObj );
        }
        catch (...) {}
        if ( pTObj == 0 )
          throw Exception( "Static cast failed",
                           "IObjectStoreMgr::findObject<T>",
                           "coral::CoralServer" );
        return pTObj;
      }

      /// Delete an object in the store, given its ID.
      /// Throw an exception if no object is found with that ID.
      virtual void releaseObject( Token ID ) = 0;

    protected:

      /// Object pointer and type.
      typedef std::pair< void*, const std::type_info* > ObjectAndType;

    private:

      /// Does the object store support this object type?
      virtual bool isSupportedType( const std::type_info& type ) = 0;

      /// Register an object in the store and return its ID.
      /// Indicate a parent which cannot be deleted before it
      /// (parentID=0 means that the object itself is a parent).
      /// Throw an exception if no parent is found with that ID.
      virtual Token registerObject( ObjectAndType obj,
                                    Token parentID ) = 0;

      /// Find an object in the store, given its ID.
      /// Throw an exception if no object is found with that ID.
      virtual ObjectAndType findObject( Token ID ) = 0;

    };

  }

}
#endif // CORALSERVER_IOBJECTSTOREMGR_H
