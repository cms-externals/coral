import os
os.environ['CORAL_DBLOOKUP_PATH']='/afs/cern.ch/sw/lcg/app/pool/db'
os.environ['CORAL_AUTH_PATH']='/afs/cern.ch/sw/lcg/app/pool/db'
import coral
import pickle
import math


class DmlOperations:
 "A Simple Test for DmlOperations"
 m_connectionString = 0
 m_svc = 0
 def __init__( self, connectionString ):
  print("DmlOperations Constructor") 
  try:
   print("In DmlOperations" , " connectionString = " , connectionString)
   self.m_svc = coral.ConnectionService()
   #self.loadServices()
   self.m_connectionString = connectionString
  except Exception as e:
   raise Exception("Error in Initializer: " + str(e)) 

 def atomicOperations(self):
  "AtomicOperations method of DmlOperations"
  try:
    print("In AtomicOperations")
    session = self.m_svc.connect(self.m_connectionString, coral.access_Update)

    session.transaction().start()

    editor1 = session.nominalSchema().tableHandle( "T1" ).dataEditor()
    rowBuffer1 = coral.AttributeList()
    rowBuffer1.extend( "id","long" )
    rowBuffer1.extend( "t", "long long" )
    rowBuffer1.extend( "X", "float" )
    rowBuffer1.extend( "Y", "double" )
    rowBuffer1.extend( "Z", "double" )
    rowBuffer1.extend( "Comment", "string" )
    rowBuffer1.extend( "Data", "blob" )

    for i in range( 0,10 ):
      id = i + 1
      rowBuffer1[ "id" ].setData(id)
      t = 1
      t <<= 4*i
      rowBuffer1[ "t" ].setData(t)
      x = 1 + (i+1)*0.001
      rowBuffer1[ "X" ].setData(x)
      y = 2 + (i+1)*0.0001
      rowBuffer1[ "Y" ].setData(y)
      z = i*1.1
      rowBuffer1[ "Z" ].setData(z)
      os = "Row " + str( i + 1)
      comment = str(os)
      rowBuffer1[ "Comment" ].setData(comment)

      blobSize = 1000 * ( i + 1 );
      #blob.resize( blobSize )
      blob = coral.Blob()
      p = []
      for j in range( 0, blobSize ):
       p.append(( i + j )%256)

      pickle.dump(p,blob,True)
      rowBuffer1[ "Data" ].setData(blob)
      if ( i%4 == 2 ):
        rowBuffer1[ "Z" ].setData( None )

      #Insert the row
      editor1.insertRow( rowBuffer1 )

    # Delete some rows.
    deleteCondition = "Z > :z"
    deleteData = coral.AttributeList()
    deleteData.extend( "z", "float" )
    deleteData[0].setData(7)
    rowsDeleted = editor1.deleteRows( deleteCondition, deleteData )
    print("Deleted " , rowsDeleted , " rows.")
    if ( rowsDeleted != 3 ):
      raise Exception( "Unexpected number of rows deleted" )

    #Update some rows.
    updateAction = "t = t + :offset"
    updateCondition = "X < :x"
    updateData = coral.AttributeList()
    updateData.extend("offset","int")
    updateData.extend("x","float")
    updateData[0].setData(111)
    updateData[1].setData(1.003)
    rowsUpdated = editor1.updateRows( updateAction,updateCondition,updateData )
    print("Updated ", rowsUpdated , " rows.")
    if ( rowsUpdated != 2 ):
      raise Exception( "Unexpected number of rows updated" )

    session.transaction().commit()

    del session

    return True

  except Exception as e:
   raise Exception("Error in AtomicOperations method: " + str(e)) 
   return False

 def bulkOperations(self):
  "BulkOperations method of DmlOperations"
  try:
    print("In BulkOperations")
    session = self.m_svc.connect(self.m_connectionString, coral.access_Update)

    session.transaction().start()

    # Fill in table T2
    editor2 = session.nominalSchema().tableHandle( "T2" ).dataEditor()
    rowBuffer2 = coral.AttributeList()
    rowBuffer2.extend( "id","long" )
    rowBuffer2.extend( "tx","float" )
    rowBuffer2.extend( "ty","double" )

    bulkInserter = editor2.bulkInsert( rowBuffer2, 10 )

    for i in range( 0, 35 ):
      id = i
      rowBuffer2[0].setData(id)
      tx = i + 100.1
      rowBuffer2[1].setData(tx)
      ty = i + 200.234
      rowBuffer2[2].setData(ty)
      bulkInserter.processNextIteration()

    bulkInserter.flush()
    del bulkInserter

    # Bulk delete some rows
    deleteCondition = "tx < :tx AND id > :id "
    deleteData = coral.AttributeList()
    deleteData.extend( "tx","float" )
    deleteData.extend( "id","long" )
    bulkDeleter = editor2.bulkDeleteRows( deleteCondition, deleteData, 3 )
    for i in range( 0, 5 ):
      deleteData[0].setData(104 + 4*i)
      deleteData[1].setData(1 + 4*i)
      bulkDeleter.processNextIteration()

    bulkDeleter.flush()
    del bulkDeleter

    # Bulk update some rows
    updateAction = "tx = :offset"
    updateCondition = "ty > :minValue AND ty < :maxMalue"
    updateData = coral.AttributeList()
    updateData.extend( "offset","float" )
    updateData.extend( "minValue","float" )
    updateData.extend( "maxMalue","float" )

    bulkUpdater = editor2.bulkUpdateRows( updateAction, updateCondition, updateData, 3 )
    for i in range( 0, 5 ):
      offset = i
      updateData[0].setData(offset)
      minValue = 200 + i*3
      updateData[1].setData(minValue)
      maxValue = 202 + i*3
      updateData[2].setData(maxValue)
      bulkUpdater.processNextIteration()

    bulkUpdater.flush()
    del bulkUpdater

    # Fill-in table T3
    editor3 = session.nominalSchema().tableHandle( "T3" ).dataEditor()
    rowBuffer3 = coral.AttributeList()
    rowBuffer3.extend( "id","long" )
    rowBuffer3.extend( "fk1","long" )
    rowBuffer3.extend( "fk2","long" )
    rowBuffer3.extend( "Qn","float" )

    bulkInserter = editor3.bulkInsert( rowBuffer3, 3 )
    for i in range( 0, 6 ):
      id3 = i+1
      rowBuffer3[0].setData(id3)
      fk1 = i+1
      rowBuffer3[1].setData(fk1)
      fk2 = i*4 + i%2
      rowBuffer3[2].setData(fk2)
      q = ( i + 1 ) * 0.3
      rowBuffer3[3].setData(q)
      bulkInserter.processNextIteration()

    bulkInserter.flush()
    del bulkInserter


    session.transaction().commit()
    del session


    return True

  except Exception as e:
   raise Exception("Error in BulkOperations method: " + str(e)) 
   return False

 def __del__( self ):
  print("DmlOperations Destructor") 
