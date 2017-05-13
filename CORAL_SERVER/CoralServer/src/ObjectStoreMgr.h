// $Id: ObjectStoreMgr.h,v 1.5.2.2 2010/12/20 13:04:03 avalassi Exp $
#ifndef CORALSERVER_OBJECTSTOREMGR_H
#define CORALSERVER_OBJECTSTOREMGR_H 1

// Include files
#include <map>
#include "CoralBase/boost_thread_headers.h"

// Local include files
#include "IObjectStoreMgr.h"

namespace coral
{

  namespace CoralServer
  {

    // Forward declaration
    class ITokenPool;

    /** @class ObjectStoreMgr
     *
     *  @author Andrea Valassi
     *  @date   2007-11-26 (python)
     *  @date   2008-01-22 (C++)
     */

    class ObjectStoreMgr : virtual public IObjectStoreMgr
    {

    public:

      /// Standard constructor.
      /// A token pool (not owned by this instance) may be specified:
      /// if it is 0 , the object store manager creates its own token pool.
      ObjectStoreMgr( ITokenPool* tokenPool );

      /// Destructor.
      virtual ~ObjectStoreMgr();

      /// Delete an object in the store, given its ID.
      /// Throw an exception if no object is found with that ID.
      /// Lock the mutex for the duration of the call.
      void releaseObject( coral::Token ID );

    private:

      /// Delete an object in the store, given its ID.
      /// Throw an exception if no object is found with that ID.
      /// If the flag is true, erase the pointer in the map.
      /// If the flag is false, reset to 0 the pointer in the map.
      void i_releaseObject( coral::Token ID, bool erase );

      /// Does the object store support this object type?
      bool isSupportedType( const std::type_info& type );

      /// Register an object in the store and return its ID.
      /// Indicate a parent which cannot be deleted before it
      /// (parentID=0 means that the object itself is a parent).
      /// Throw an exception if no parent is found with that ID.
      /// Lock the mutex for the duration of the call.
      coral::Token registerObject( ObjectAndType obj,
                                   Token parentID  );

      /// Find an object in the store, given its ID.
      /// Throw an exception if no object is found with that ID.
      /// Lock the mutex for the duration of the call.
      ObjectAndType findObject( coral::Token ID );

      /// Find an object in the store, given its ID.
      /// Throw an exception if no object is found with that ID.
      /// Do not lock the mutex for the duration of the call.
      ObjectAndType i_findObject( coral::Token ID );

      /// Delete an object given its pointer and type info.
      void deleteObject( ObjectAndType obj, bool throwIfNull );

      /// Delete an object given its pointer and templated type.
      template<class T> void deleteObject( void* pObj )
      {
        T* pTObj;
        try
        {
          pTObj = static_cast<T*>( pObj );
        }
        catch (...) {}
        if ( pTObj == 0 )
          throw Exception( "Static cast failed",
                           "ObjectStoreMgr::deleteObject<T>",
                           "coral::CoralServer" );
        delete pTObj;
      }

    private:

      // Token pool
      ITokenPool* m_tokenPool;

      // Is this an own token pool?
      bool m_ownTokenPool;

      // The object store map.
      std::map< coral::Token, ObjectAndType > m_dict;

      // The 1-to-N parent to children map.
      // For each session, get the vector of its cursors/bulkops.
      std::map< coral::Token, std::set<coral::Token> > m_children;

      // The 1-to-1 child to parent map.
      // For each cursor/bulkop, get the parent session.
      std::map< coral::Token, coral::Token > m_parent;

      // The mutex lock
      boost::mutex m_mutex;

    };

  }

}
#endif // CORALSERVER_OBJECTSTOREMGR_H
