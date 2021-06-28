import os
os.environ['CORAL_DBLOOKUP_PATH']='/afs/cern.ch/sw/lcg/app/pool/db'
os.environ['CORAL_AUTH_PATH']='/afs/cern.ch/sw/lcg/app/pool/db'
import coral
import time
import pickle


try:

  ## Fill Data
  svc = coral.ConnectionService()
  session = svc.connect( '/dbdev/service_1', accessMode = coral.access_Update )
  transaction = session.transaction()
  transaction.start()

  schema = session.nominalSchema()

  print("About to drop previous table")
  schema.dropIfExistsTable( "T_1" )

  print("Describing new table")
  description = coral.TableDescription()
  description.setName( "T_1" )
  description.insertColumn( "ID","int")
  description.setPrimaryKey( "ID" );
  description.insertColumn( "x","float")
  description.setNotNullConstraint( "x" )
  description.insertColumn( "Y","double")
  description.insertColumn( "Z","double")
  description.insertColumn( "data0","string" )
  description.insertColumn( "data1","string", 2, True )
  description.insertColumn( "data2", "string",321,False)

  print("About to create the tables")
  editor = schema.createTable( description ).dataEditor()

  rowBuffer = coral.AttributeList()
  editor.rowBuffer( rowBuffer )

  for i in range (0,10):
    rowBuffer["ID"].setData(i)
    rowBuffer["x"].setData(i + 0.1*i)
    rowBuffer["Y"].setData(i + 0.01*i)
    rowBuffer["Z"].setData(i + 0.001*i)
    print("The data0 is" , repr(i))
    rowBuffer["data0"].setData(repr(i))
    print("The data1 is" , repr(i))
    rowBuffer["data1"].setData(repr(i))
    data2 = "Hello" + repr(10*(i-1)+i)
    print("The data2 is" , repr(data2))
    rowBuffer["data2"].setData(data2)

    if ( i%3 == 1):
     rowBuffer["Y"].setData(None)

    editor.insertRow( rowBuffer )

  #Now try to insert a duplicate entry
  try :
    editor.insertRow( rowBuffer )  
    print("DuplicateEntryInUniqueKeyException not thrown !!!") 
  except Exception as e:
    print("DuplicateEntryInUniqueKeyException thrown !!!")  
    print(str(e))
  
  #Remove a few entries...
  inputData = coral.AttributeList()
  inputData.extend( "id","int" )
  inputData[0].setData(5)
  rowsDeleted = editor.deleteRows( "ID<:id", inputData )
  print("Removed " , rowsDeleted , " rows")

  #Udpate the entries;
  inputData.extend("increment","double")
  inputData[1].setData(1.111)
  inputData[0].setData(inputData[0].data() + 2)
  rowsUpdated = editor.updateRows( "Y   = Y+ :increment", "ID>:id", inputData )
  print("Updated " , rowsUpdated , " rows")

  transaction.commit()
  del session

  print("SUCCESS")

except Exception as e:
  print("FAILURE")
  print(str(e))
