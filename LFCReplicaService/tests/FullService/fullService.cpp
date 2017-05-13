#include <iostream>
#include <sstream>
#include <stdexcept>
#include "CoralKernel/Context.h"
#include "CoralBase/Exception.h"
#include "RelationalAccess/ILookupService.h"
#include "RelationalAccess/IAuthenticationService.h"
#include "RelationalAccess/IAuthenticationCredentials.h"
#include "RelationalAccess/IDatabaseServiceSet.h"
#include "RelationalAccess/IDatabaseServiceDescription.h"

#include "CoralBase/MessageStream.h"

#define LOG_DEBUG( msg ) { coral::MessageStream logger( "LFCNode" ); logger << coral::Debug << msg << coral::MessageStream::endmsg; }

void
runTest()
{
  coral::Context& ctx = coral::Context::instance();
  ctx.loadComponent( "CORAL/Services/LFCReplicaService" );

  coral::IHandle<coral::ILookupService> lookupSvc = ctx.query<coral::ILookupService>();
  coral::IHandle<coral::IAuthenticationService> authenticationSvc = ctx.query<coral::IAuthenticationService>();

  coral::IDatabaseServiceSet* dbSet = lookupSvc->lookup( "CondDB" );

  int numberOfReplicas = dbSet->numberOfReplicas();

  LOG_DEBUG(" Found [" << numberOfReplicas << "] replicas");
  for ( int i = 0; i < numberOfReplicas; ++i )
  {
    if( !dbSet->replica( i ).authenticationMechanism().empty() )
    {
      const coral::IAuthenticationCredentials& creds = authenticationSvc->credentials(dbSet->replica( i ).connectionString());
      LOG_DEBUG(" Found [" << creds.numberOfItems() << "] credential items");
      for(int it = 0; it < creds.numberOfItems(); ++it )
      {
        LOG_DEBUG("For item=" << creds.itemName(it) << " value=" << creds.valueForItem(creds.itemName(it)));
      }
      const coral::IAuthenticationCredentials& creds0 = authenticationSvc->credentials(dbSet->replica( i ).connectionString(),"reader");
      LOG_DEBUG(" Found [" << creds0.numberOfItems() << "] credential items for role 'reader'");
      for(int it = 0; it < creds0.numberOfItems(); ++it )
      {
        LOG_DEBUG("For item=" << creds0.itemName(it) << " value=" << creds0.valueForItem(creds.itemName(it)));
      }
    }
  }

  delete dbSet;
  // Do it again but this time the data should be cached
  dbSet = lookupSvc->lookup( "CondDB", coral::Update, "password" );

  numberOfReplicas = dbSet->numberOfReplicas();

  LOG_DEBUG(" Found [" << numberOfReplicas << "] replicas");
  for ( int i = 0; i < numberOfReplicas; ++i )
  {
    if( !dbSet->replica( i ).authenticationMechanism().empty() )
    {
      const coral::IAuthenticationCredentials& creds = authenticationSvc->credentials(dbSet->replica( i ).connectionString());
      LOG_DEBUG(" Found [" << creds.numberOfItems() << "] credential items");
      for(int it = 0; it < creds.numberOfItems(); ++it )
      {
        LOG_DEBUG("For item=" << creds.itemName(it) << " value=" << creds.valueForItem(creds.itemName(it)));
      }
    }
  }

  delete dbSet;

}

int main( int, char** )
{

  try
  {
    runTest();

    /*
    int numberOfReplicas = 0;


    coral::IDatabaseServiceSet* dbSet = lookupSvc->lookup( "test1/ecal_calib0");
    numberOfReplicas = dbSet->numberOfReplicas();
    std::cout <<" Found ["<<numberOfReplicas<<"] replicas"<<std::endl;
    for ( int i = 0; i < numberOfReplicas; ++i ){
      reportReplica( dbSet->replica( i ), "test1/ecal_calib0" );
      if(dbSet->replica( i ).authenticationMechanism()!=""){
        std::cout <<" Default role:"<<std::endl;
        const coral::IAuthenticationCredentials& creds = authenticationSvc->credentials(dbSet->replica( i ).connectionString());
        std::cout <<" Found ["<<creds.numberOfItems()<<"] credential items"<<std::endl;
        for(int it=0;it<creds.numberOfItems();++it){
          std::cout <<"For item="<<creds.itemName(it)<<" value="<<creds.valueForItem(creds.itemName(it))<<std::endl;
        }
        std::cout <<" Role: calib_reader"<<std::endl;
        try{
          const coral::IAuthenticationCredentials& creds0 = authenticationSvc->credentials(dbSet->replica( i ).connectionString(),"calib_reader");
          std::cout <<" Found ["<<creds0.numberOfItems()<<"] credential items"<<std::endl;
          for(int it=0;it<creds0.numberOfItems();++it){
            std::cout <<"For item="<<creds0.itemName(it)<<" value="<<creds0.valueForItem(creds0.itemName(it))<<std::endl;
          }
        } catch (coral::Exception& e){
          std::cout << "Exception: "<<e.what()<<std::endl;
        }
        std::cout <<" Role: calib_writer"<<std::endl;
        try{
          const coral::IAuthenticationCredentials& creds1 = authenticationSvc->credentials(dbSet->replica( i ).connectionString(),"calib_writer");
          std::cout <<" Found ["<<creds1.numberOfItems()<<"] credential items"<<std::endl;
          for(int it=0;it<creds1.numberOfItems();++it){
            std::cout <<"For item="<<creds1.itemName(it)<<" value="<<creds1.valueForItem(creds1.itemName(it))<<std::endl;
          }
        } catch (coral::Exception& e){
          std::cout << "Exception: "<<e.what()<<std::endl;
        }
      }
    }
    delete dbSet;
    dbSet = 0;
    dbSet = lookupSvc->lookup( "test1/ecal_calib0", coral::ReadOnly);
    numberOfReplicas = dbSet->numberOfReplicas();
    std::cout <<" Found ["<<numberOfReplicas<<"] replicas"<<std::endl;
    for ( int i = 0; i < numberOfReplicas; ++i ){
      reportReplica( dbSet->replica( i ), "test1/ecal_calib0" );
      if(dbSet->replica( i ).authenticationMechanism()!=""){
        std::cout <<" Default role:"<<std::endl;
        const coral::IAuthenticationCredentials& creds = authenticationSvc->credentials(dbSet->replica( i ).connectionString());
        std::cout <<" Found ["<<creds.numberOfItems()<<"] credential items"<<std::endl;
        for(int it=0;it<creds.numberOfItems();++it){
          std::cout <<"For item="<<creds.itemName(it)<<" value="<<creds.valueForItem(creds.itemName(it))<<std::endl;
        }
        std::cout <<" Role: calib_reader"<<std::endl;
        try{
          const coral::IAuthenticationCredentials& creds0 = authenticationSvc->credentials(dbSet->replica( i ).connectionString(),"calib_reader");
          std::cout <<" Found ["<<creds0.numberOfItems()<<"] credential items"<<std::endl;
          for(int it=0;it<creds0.numberOfItems();++it){
            std::cout <<"For item="<<creds0.itemName(it)<<" value="<<creds0.valueForItem(creds0.itemName(it))<<std::endl;
          }
        } catch (coral::Exception& e){
          std::cout << "Exception: "<<e.what()<<std::endl;
        }
        std::cout <<" Role: calib_writer"<<std::endl;
        try{
          const coral::IAuthenticationCredentials& creds1 = authenticationSvc->credentials(dbSet->replica( i ).connectionString(),"calib_writer");
          std::cout <<" Found ["<<creds1.numberOfItems()<<"] credential items"<<std::endl;
          for(int it=0;it<creds1.numberOfItems();++it){
            std::cout <<"For item="<<creds1.itemName(it)<<" value="<<creds1.valueForItem(creds1.itemName(it))<<std::endl;
          }
        } catch (coral::Exception& e){
          std::cout << "Exception: "<<e.what()<<std::endl;
        }
      }
    }
    delete dbSet;
    std::cout << "Exiting..." << std::endl;
    */
  }
  catch ( std::exception& e ) {
    std::cerr << "Standard C++ exception : " << e.what() << std::endl;
    return 1;
  }
  catch ( ... ) {
    std::cerr << "Exception caught (...)" << std::endl;
    return 1;
  }
  std::cout << "[OVAL] Success" << std::endl;
  return 0;
}
