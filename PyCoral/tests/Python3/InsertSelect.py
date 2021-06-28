import os
os.environ['CORAL_DBLOOKUP_PATH']='/afs/cern.ch/sw/lcg/app/pool/db'
os.environ['CORAL_AUTH_PATH']='/afs/cern.ch/sw/lcg/app/pool/db'
import coral
import pickle
import math
import time


class InsertSelect:
 "A Simple Test for InsertSelect"
 m_connectionString = 0
 m_svc = 0
 def __init__( self, connectionString ):
  print("InsertSelect Constructor") 
  try:
   print("In InsertSelect" , " ConnectionString = " , connectionString)
   self.m_svc = coral.ConnectionService()
   self.m_connectionString = connectionString

  except Exception as e:
   raise Exception("Error in Initializer: " + str(e)) 

 def run(self):
  "Run method of InsertSelect"
  try:
    print("InsertSelect: In run")
    session = self.m_svc.connect(self.m_connectionString, coral.access_Update)

    session.transaction().start()

    workingSchema = session.nominalSchema()
    workingSchema.dropIfExistsTable( "T4" )
    description = coral.TableDescription(workingSchema.tableHandle( "T2" ).description())
    description.setName( "T4" )
    table = workingSchema.createTable( description )
    table.privilegeManager().grantToPublic( coral.privilege_Select )

    operation = table.dataEditor().insertWithQuery()
    query = operation.query()
    query.addToTableList( "T2" )
    query.setCondition( "id<10", coral.AttributeList() )
    numberOfRowsInserted = operation.execute()
    del operation
    if ( numberOfRowsInserted != 6 ):
      raise Exception( "Unexpected number of rows inserted.") 

    bulkOperation = table.dataEditor().bulkInsertWithQuery( 3 )
    queryb = bulkOperation.query()
    queryb.addToTableList( "T2" )
    condition = coral.AttributeList()
    condition.extend( "idmin", "long")
    condition.extend( "idmax", "long")
    queryb.setCondition( "id>:idmin AND id<:idmax", condition )

    idmin = 10;
    idmax = 13;
    for i in range( 0, 5 ):
      condition[0].setData(idmin)
      condition[1].setData(idmax)
      bulkOperation.processNextIteration()
      idmin += 4
      idmax += 4

    bulkOperation.flush()
    del bulkOperation

    # add three more rows (for the next tests to make better sense)
    rowBuffer = coral.AttributeList()

    table.dataEditor().rowBuffer( rowBuffer )
    rowBuffer[0].setData(100)
    rowBuffer[1].setData(200)
    rowBuffer[2].setData(300)
    for i in range(0, 3 ):
      table.dataEditor().insertRow( rowBuffer )
      rowBuffer[0].setData(rowBuffer[0].data() + 1)
      rowBuffer[1].setData(rowBuffer[1].data() + 2)
      rowBuffer[2].setData(rowBuffer[2].data() + 3)

    session.transaction().commit()
    del session
    time.sleep( 1 )

    return True

  except Exception as e:
   raise Exception("Error in Run method: " + str(e)) 
   return False

 def __del__( self ):
  print("InsertSelect Destructor") 
