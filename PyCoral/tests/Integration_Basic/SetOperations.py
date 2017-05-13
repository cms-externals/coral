import os
os.environ['CORAL_DBLOOKUP_PATH']='/afs/cern.ch/sw/lcg/app/pool/db'
os.environ['CORAL_AUTH_PATH']='/afs/cern.ch/sw/lcg/app/pool/db'
import coral
import sys


class SetOperations:
 "A Simple Test for SetOperations"
 m_connectionString = 0
 m_workingSchema = 0
 m_svc = 0

 def __init__( self, connectionString, workingSchema ):
  print "SetOperations Constructor" 
  try:
   print "In SetOperations" , " ConnectionString = " , connectionString
   print "In SetOperations" , " WorkingSchema = " , workingSchema
   self.m_connectionString = connectionString
   self.m_workingSchema = workingSchema
   self.m_svc = coral.ConnectionService()


  except Exception, e:
   raise Exception("Error in SetOperations Initializer: " + str(e)) 

 def run(self):
  "Run method of SetOperations"
  try:
    print "In run"

    session = self.m_svc.connect(self.m_connectionString, coral.access_Update)

    session.transaction().start()

    workingSchema = session.schema(self.m_workingSchema)

    #print "Query 0:"
    #query0 = workingSchema.newQuery()
    #query0.addToTableList( "T2" )
    #query0.setRowCacheSize( 100 )
    #nRows = 0
    #cursor0 = query0.execute()
    #while ( cursor0.next() ):
    #  print str(cursor0.currentRow())
    #  nRows = nRows + 1
#
#    del query0
#    print nRows
#
#    print "Query 10:"
#    query10 = workingSchema.newQuery()
#    query10.addToTableList( "T4" )
#    query10.setRowCacheSize( 100 )
#    nRows = 0
#    cursor10 = query10.execute()
#    while ( cursor10.next() ):
#      print str(cursor10.currentRow())
#      nRows = nRows + 1
#
#    del query10
#    print nRows



    print " Query 1:"
    query1 = workingSchema.newQuery()
    query1.addToTableList( "T2" )
    rhs1 = query1.applySetOperation( coral.query_Union )
    rhs1.addToTableList( "T4" )
    query1.setRowCacheSize( 100 )
    nRows = 0
    cursor1 = query1.execute()
    while ( cursor1.next() ):
      print str(cursor1.currentRow())
      nRows = nRows + 1

    del query1
    if ( nRows != 28 ):
      print nRows
      raise RuntimeError( "Unexpected number of rows" )

    print " Query 2:"
    query2 = workingSchema.newQuery()
    query2.addToTableList( "T4" )
    rhs2 = query2.applySetOperation( coral.query_Minus )
    rhs2.addToTableList( "T2" )
    nRows = 0
    cursor2 = query2.execute()
    while ( cursor2.next() ):
      print str(cursor2.currentRow())
      nRows = nRows + 1

    del query2
    if ( nRows != 3 ):
      print nRows
      raise RuntimeError( "Unexpected number of rows" )


    print " Query 3:"
    query3 = workingSchema.newQuery()
    query3.addToTableList( "T2" )
    rhs3 = query3.applySetOperation( coral.query_Minus )
    rhs3.addToTableList( "T4" )
    nRows = 0
    cursor3 = query3.execute()
    while ( cursor3.next() ):
      print str(cursor3.currentRow())
      nRows = nRows + 1

    del query3
    if ( nRows != 12 ):
      print nRows
      raise RuntimeError( "Unexpected number of rows" )

    print  " Query 4:"
    query4 = workingSchema.newQuery()
    query4.addToTableList( "T2" )
    rhs4 = query4.applySetOperation( coral.query_Intersect )
    rhs4.addToTableList( "T4" )
    nRows = 0
    cursor4 = query4.execute()
    while ( cursor4.next() ):
      print str(cursor4.currentRow())
      nRows = nRows + 1

    del query4
    if ( nRows != 13 ):
      print nRows
      raise RuntimeError( "Unexpected number of rows" )


    print " Query 5:"
    query5 = workingSchema.newQuery()

    sub = query5.defineSubQuery( "A" )
    sub.addToTableList( "T2" )
    sub.setCondition( "id>10", coral.AttributeList() )
    rhs5 = sub.applySetOperation( coral.query_Intersect )
    rhs5.addToTableList( "T4" )

    query5.addToTableList( "A", "B" )
    query5.addToOrderList( "ty-tx" )
    query5.limitReturnedRows( 4 )
    nRows = 0
    cursor5 = query5.execute()
    while ( cursor5.next() ):
      print str(cursor5.currentRow())
      nRows = nRows + 1

    del query5
    if ( nRows != 4 ):
      print nRows
      raise RuntimeError( "Unexpected number of rows" )


    session.transaction().commit()
    del session



    return True

  except Exception, e:
   raise Exception("Error in Run method: " + str(e)) 
   return False

 def __del__( self ):
  print "SetOperations Destructor" 
