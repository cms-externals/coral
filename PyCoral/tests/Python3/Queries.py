import os
os.environ['CORAL_DBLOOKUP_PATH']='/afs/cern.ch/sw/lcg/app/pool/db'
os.environ['CORAL_AUTH_PATH']='/afs/cern.ch/sw/lcg/app/pool/db'
import coral
import pickle
import math


class Queries: 
 "A Simple Test for Queries"
 m_connectionString = 0
 m_workingSchema = 0
 m_svc = 0
 def __init__( self, connectionString, workingSchema ):
  print("Queries Constructor") 
  try:
   print("In Queries" , " ConnectionString = " , connectionString)
   print("In Queries" , " WorkingSchema = " , workingSchema)
   self.m_connectionString = connectionString
   self.m_workingSchema = workingSchema
   self.m_svc = coral.ConnectionService()

  except Exception as e:
   raise Exception("Error in Initializer: " + str(e)) 

 def run(self):
  "Run method of Queries"
  try:
    print("Queries: In run")
    session = self.m_svc.connect(self.m_connectionString, coral.access_Update)

    session.transaction().start()

    workingSchema = session.schema(self.m_workingSchema)

    # Run a query on a table
    query0 = workingSchema.tableHandle( "T1" ).newQuery()
    query0.addToOutputList( "Data" )
    query0.addToOutputList( "Z" )
    query0.addToOrderList( "id" )
    cursor0 = query0.execute()
    i = 0
    while ( cursor0.next() ):
      print("Row ", i + 1)
      blob = cursor0.currentRow()[0].data()
      p = pickle.load(blob)
      #print p
      expectedBlobSize = 1000 * ( i + 1 );
      if ( len(p) != expectedBlobSize ):
        print("Error : returned blob size = " , len(p) , " while expected is " , expectedBlobSize)
        del query0
        raise Exception( "Unexpected blob data" )

      for j in range( 0, expectedBlobSize ):
        if ( p[j] != ( i + j )%256 ):
          del query0;
          raise Exception( "Unexpected blob data" )

      if ( i%4 == 2 ):
        nullNess = True
      else:
        nullNess = False
        
      if ( cursor0.currentRow()[1].isNull() != nullNess ):
        del query0
        raise Exception( "Unexpected null-ness" )
      i = i + 1

    del query0
    if ( i != 7 ):
       raise Exception( "Unexpected number of returned rows" )

    # Run a wildcard query on the table
    query1 = workingSchema.newQuery()
    query1.addToTableList( "T3" )
    cursor1 = query1.execute()
    #while ( cursor1.next() ):
    #  print str(cursor1.currentRow())
    for row in cursor1:
      print(str(row))

    del query1

    # Run a wildcard query on the view
    query2 = workingSchema.newQuery()
    query2.addToTableList( "V0" )
    cursor2 = query2.execute()
    while ( cursor2.next() ):
      print(str(cursor2.currentRow()))

    del query2

    # Run a query on a table
    print("Gitika......") 
    query3 = workingSchema.tableHandle( "T3" ).newQuery()
    query3.addToOutputList( "id" )
    query3.addToOrderList( "Qn" )
    query3.setRowCacheSize( 100 )
    query3.limitReturnedRows( 2, 2 )
    cursor3 = query3.execute()
    nRows = 0
    while ( cursor3.next() ):
      print(str(cursor3.currentRow()))
      nRows = nRows +1

    del query3
    if ( nRows != 2 ):
      raise Exception( "Unexpected number of rows returned" )

    # Perform a query involving two tables.
    query4 = workingSchema.newQuery()
    query4.addToOutputList( "T1.X", "x" )
    query4.addToOutputList( "T1.t", "t" )
    query4.addToOutputList( "T3.Qn", "Qn" )
    query4.addToTableList( "T1" )
    query4.addToTableList( "T3" )
    condition = "T1.id = T3.fk1 AND T1.Z<:zmax"
    conditionData = coral.AttributeList()
    conditionData.extend( "zmax","float" )
    query4.setCondition( condition, conditionData )
    query4.setMemoryCacheSize( 5 )
    conditionData[0].setData(3)
    nRows = 0
    for row4 in query4.execute():
      print(str(row4))
      nRows = nRows +1

    del query4
    if ( nRows != 2 ):
      raise Exception( "Unexpected number of rows returned" )

    # Perform a query involving a subquery.
    query5 = workingSchema.newQuery()
    query5.addToOutputList( "A.t", "t" )
    query5.addToOutputList( "B.tx", "tx" )

    subQuery1 = query5.defineSubQuery( "A" )
    subQuery1.addToOutputList( "t" )
    subQuery1.addToOutputList( "id" )
    subQuery1.addToTableList( "T1" )
    cond1 = coral.AttributeList()
    cond1.extend( "xmin","float" );
    subQuery1.setCondition( "X < :xmin", cond1 )
    query5.addToTableList( "A" )

    subQuery2 = query5.defineSubQuery( "B" )
    subQuery2.addToOutputList( "id" )
    subQuery2.addToOutputList( "TX", "tx" )
    subQuery2.addToTableList( "V0" )
    cond2 = coral.AttributeList()
    cond2.extend( "tymax","float" )
    subQuery2.setCondition( "TX < :tymax", cond2 )
    query5.addToTableList( "B" )

    query5.setCondition( "A.id = B.id", coral.AttributeList() )

    cond1[0].setData(1.005)
    cond2[0].setData(210)
    nRows = 0
    cursor5 = query5.execute()
    while ( cursor5.next() ): 
      print(str(cursor5.currentRow()))
      nRows = nRows +1

    del query5
    if ( nRows != 3 ):
      raise Exception( "Unexpected number of rows returned" )

    session.transaction().commit()
    del session


    return True

  except Exception as e:
   raise Exception("Error in Run method: " + str(e)) 
   return False

 def __del__( self ):
  print("Queries Destructor") 
