#!/usr/bin/env python
import os, sys
import coral
import time

MSGHDR = "+++ CORALSERVER TEST +++"

#----------------------------------------------------------------------------

def usage():
    print 'Usage: testCoralServer.py ["server"|"proxy"|"local"|"Fac"|"StbFac" ["1ses"|"2ses"|"2sesAll" ["defOut"|"noDefOut"] ] ]'
    print 'Default: testCoralServer.py Fac 1ses defOut'
    sys.exit(1)
    
#----------------------------------------------------------------------------

user='lcgnight'
xmlPrivate='/afs/cern.ch/sw/lcg/app/pool/db'

# No real need for a different setup for user avalassi 
# Keep the same setup as for user lcgnight (test fix for bug #87175)
###if 'USER' in os.environ and os.environ['USER'] == 'avalassi':
###    user=os.environ['USER']
###    xmlPrivate='/afs/cern.ch/user/'+user[0]+'/'+user+'/private'
        
# Analyse the command line arguments
server="Fac"    # Allowed values: "server", "proxy", "local", "Fac" (default), "StbFac"
sessions="1ses" # Allowed values: "1ses" (default), "2ses", "2sesAll"
defOut="defOut" # Allowed values: "defOut" (default), "noDefOut" 
if len(sys.argv)>4 : usage()
if len(sys.argv)>=4 : defOut=sys.argv[3]
if len(sys.argv)>=3 : sessions=sys.argv[2]
if len(sys.argv)>=2 : server=sys.argv[1]

if server=="local": local=True
elif server=="server" or server=="proxy" : local=False
elif server=="Fac" or server=="StbFac" : local=False
else: usage()
    
if sessions=="1ses": twoSessions=False
elif sessions=="2ses": twoSessions=True
elif sessions=="2sesAll": twoSessions=True
else: usage()
fetchAllRows=False
if sessions=="2sesAll": fetchAllRows=True

if defOut=="defOut": doDefOut=True
elif defOut=="noDefOut": doDefOut=False
else: usage()

try:

    # Configure data access - from the server, a server proxy or locally
    if local:
        os.environ['CORAL_DBLOOKUP_PATH']=xmlPrivate
        os.environ['CORAL_AUTH_PATH']=xmlPrivate
        ###os.environ['CORAL_DBLOOKUP_PATH']=xmlPrivate+"/CoralServer"
        ###os.environ['CORAL_AUTH_PATH']=xmlPrivate+"/CoralServer"
        serverUrl=''
    else:
        if server=="server":
            HOST='localhost'   # The coral server host
            PORT=40007         # The coral server port
            ###HOST='coralsrv01'  # The host for CORAL-preview nightlies
            ###PORT=40007         # The port for CORAL-preview nightlies
            ###HOST='coralsrv01'  # The host for CORAL_2_3-patches nightlies
            ###PORT=40009         # The host for CORAL_2_3-patches nightlies
            suffix=""
        elif server=="proxy":
            HOST='localhost'   # The coral proxy host
            PORT=40008         # The coral proxy port
            suffix=""
        elif server=="Fac":
            os.environ['CORAL_DBLOOKUP_PATH']=xmlPrivate
            os.environ['CORAL_AUTH_PATH']=xmlPrivate
            ###os.environ['CORAL_DBLOOKUP_PATH']=xmlPrivate+"/CoralServer"
            ###os.environ['CORAL_AUTH_PATH']=xmlPrivate+"/CoralServer"
            HOST="SFac"
            PORT=""
            suffix="_TEST"
        elif server=="StbFac":
            os.environ['CORAL_DBLOOKUP_PATH']=xmlPrivate
            os.environ['CORAL_AUTH_PATH']=xmlPrivate
            ###os.environ['CORAL_DBLOOKUP_PATH']=xmlPrivate+"/CoralServer"
            ###os.environ['CORAL_AUTH_PATH']=xmlPrivate+"/CoralServer"
            HOST="CStb+SStb+SFac"
            PORT=""
            suffix="_TEST"
        else:
            print MSGHDR, "PANIC! Unknown server type",server
            sys.exit(1)
        serverUrl = 'coral'+suffix+'://'+str(HOST)
        if PORT != "" : serverUrl += ':'+str(PORT)
        serverUrl += '&'
        ###BUFSIZE=1024       # The buffer size

    # Get the PyCoral connection service
    svc=coral.ConnectionService()
    print MSGHDR, "Got the PyCoral connection service"###,svc

    # Set the connection service configuration parameters
    svc.configuration().setConnectionRetrialPeriod(1)
    svc.configuration().setConnectionRetrialTimeOut(3)
    svc.configuration().disablePoolAutomaticCleanUp(); 
    svc.configuration().setConnectionTimeOut(0)
    print MSGHDR, "Connection service has been configured"
    ###svc.configuration().disableConnectionSharing()
    ###print MSGHDR, "Connection sharing has been disabled"

    # Define the logicalservicename prefix
    prefixUrl='COOL-'
    localUrl=prefixUrl+'Oracle-'+user
    ###localUrl='oracle://lcg_cool_nightly/lcg_cool_nightly' # nightlies
    coralUrl=serverUrl+localUrl
    ###coralUrl='sqlite_file:/tmp/sqliteTest.db'
    ###coralUrl=serverUrl+'pippo'
    ###coralUrl=localUrl

    # Get a R/O session
    print MSGHDR, "Connect R/O to:",coralUrl
    session=svc.connect(coralUrl,coral.access_ReadOnly)
    print MSGHDR, "Got a R/O session"###,session
    
    # Get the nominal schema
    schema=session.nominalSchema()
    print MSGHDR, "Got a schema"###,schema
    
    # List tables in the schema
    readOnly=True
    session.transaction().start(readOnly)
    tables=schema.listTables()
    ###print MSGHDR, "Tables in the schema:",tables
    print MSGHDR, "Fetched tables in the schema"
    session.transaction().commit()
    
    # Define the table name for all tests
    tableName="CORALSERVER_TEST"

    #------------------------------------------------------------------------
    # *** CREATE TABLE *** (only for local connection)
    #------------------------------------------------------------------------

    if local:
    
        # Reconnect in r/w mode
        print MSGHDR, "Connect R/W to:",coralUrl
        session=svc.connect(coralUrl,coral.access_Update)
        print MSGHDR, "Got a R/W session"###,session
        schema=session.nominalSchema()
        print MSGHDR, "Got a schema"###,schema

        # Start a r/w transaction
        print MSGHDR, "Create table",tableName
        readOnly=False
        session.transaction().start(readOnly)
    
        # Drop the table if it already exists
        schema.dropIfExistsTable( tableName )
    
        # Create the table
        tableDesc=coral.TableDescription();
        tableDesc.setName(tableName)
        tableDesc.insertColumn("ID","int")
        tableDesc.setPrimaryKey("ID")
        tableDesc.insertColumn("x","float")
        table=schema.createTable(tableDesc)
        
        # Commit the transaction
        session.transaction().commit()
        
        # Sleep to avoid ORA-01466
        time.sleep(1)
        
    #------------------------------------------------------------------------
    # *** FILL TABLE ***
    #------------------------------------------------------------------------

    if local: # Insertion not yet implemented in the C++ client
        
        print MSGHDR, "Fill table",tableName
        
        # Start a r/w transaction
        readOnly=False
        session.transaction().start(readOnly)
        
        # Get the table
        table=schema.tableHandle(tableName)
        print MSGHDR, "Got the table"###,table
        
        # Fill the table
        editor = table.dataEditor()
        print MSGHDR, "Got the table data editor"###,tde
        editor.deleteRows("",coral.AttributeList())
        rowBuffer = coral.AttributeList()
        editor.rowBuffer( rowBuffer )
        print MSGHDR, "Formatted buffer:",rowBuffer
        if False : # This would work too but does not use bulk operations
            for i in range (0,20):
                rowBuffer["ID"].setData( i )
                rowBuffer["x"].setData( i + 0.1*i )
                editor.insertRow( rowBuffer )
        else:
            bulkInserter = editor.bulkInsert( rowBuffer, 3 )
            for i in range (0,20):
                rowBuffer["ID"].setData( i )
                rowBuffer["x"].setData( i + 0.1*i )
                bulkInserter.processNextIteration()
            bulkInserter.flush()
            ###bulkInserter = 0 # Workaround for bug #61090
            
        # Format the output buffer
        outBuffer = coral.AttributeList()
        editor.rowBuffer( outBuffer )
        print MSGHDR, "Formatted the output buffer"

        # Commit the transaction
        session.transaction().commit()

        # reconnect in r/o mode
        print MSGHDR, "Connect R/O to:",coralUrl
        session=svc.connect(coralUrl,coral.access_ReadOnly)
        print MSGHDR, "Got a R/O session"###,session
        schema=session.nominalSchema()
        print MSGHDR, "Got a schema"###,schema

    #------------------------------------------------------------------------
    # *** READ-BACK FROM TABLE (define output) ***
    #------------------------------------------------------------------------

    if not local: # For local (R/W) connections, use dataEditor().rowBuffer()
        # FIXME: dataEditor() should throw in a RO session in CoralAccess
        outBuffer = coral.AttributeList()
        outBuffer.extend("ID","int")
        outBuffer.extend("x","float")
        print MSGHDR, "Formatted the output buffer"
        
    #------------------------------------------------------------------------
    # *** READ-BACK FROM TABLE (one session) ***
    #------------------------------------------------------------------------

    if not twoSessions:

        # Repeat twice (test caching)
        iQuery=0
        while iQuery<2:

            iQuery=iQuery+1
            if iQuery==1: suffix="st"
            elif iQuery==2: suffix="nd"
            elif iQuery==3: suffix="rd"
            else: suffix="th"
            print MSGHDR, "Execute test query for the "+iQuery.__str__()+suffix+" time"
            
            # Start a r/o transaction
            readOnly=True
            session.transaction().start(readOnly)
    
            # Get a query
            query = schema.newQuery()
            print MSGHDR, "Got a query"###,query
            query.addToTableList(tableName)
            query.setRowCacheSize(3)
            if doDefOut:
                query.defineOutput(outBuffer)
                print MSGHDR, "Defined the output"
            else:
                print MSGHDR, "Did NOT define the output"
            
            # Get a cursor
            cursor=query.execute()
            print MSGHDR, "Got a cursor"###,cursor
    
            # Iterate over the cursor
            nrows = 0
            while cursor.next() :
                nrows = nrows + 1
                row = cursor.currentRow()
                print MSGHDR, "Current row:",row
            print MSGHDR, "Retrieved",nrows,"rows"
            if nrows == 20: print "OK"
            else: print "ERROR"
            
            # Release the cursor and the query
            print MSGHDR, "Release the cursor"
            cursor=None
            print MSGHDR, "Release the query"
            query=None
            
            # Commit the transaction
            session.transaction().commit()
    
    #------------------------------------------------------------------------
    # *** READ-BACK FROM TABLE (two sessions) ***
    #------------------------------------------------------------------------

    else:

        # Get a 2nd session
        print MSGHDR, "------------------------------------------------------"
        session2=svc.connect(coralUrl,coral.access_ReadOnly)
        print MSGHDR, "Got a 2nd session"###,session2
        
        # Get the nominal schema
        schema2=session2.nominalSchema()
        print MSGHDR, "Got a schema"###,schema2
        print MSGHDR, "------------------------------------------------------"
        
        # Start a r/o transaction
        readOnly=True
        session.transaction().start(readOnly)
        session2.transaction().start(readOnly)
            
        # Repeat twice (test caching)
        iQuery=0
        while iQuery<2:

            iQuery=iQuery+1
            if iQuery==1: suffix="st"
            elif iQuery==2: suffix="nd"
            elif iQuery==3: suffix="rd"
            else: suffix="th"
            print MSGHDR, "Execute test query for the "+iQuery.__str__()+suffix+" time"
            
            # Get a query
            query=schema.newQuery()
            print MSGHDR, "Got a query"###,query
            query.addToTableList(tableName)
            if not fetchAllRows: query.setRowCacheSize(3)
            else: query.setRowCacheSize(0)
            if doDefOut:
                query.defineOutput(outBuffer)
                print MSGHDR, "Defined the output"
            else:
                print MSGHDR, "Did NOT define the output"

            # Get a 2ndquery - exactly the same as the first one!
            query2=schema2.newQuery()
            print MSGHDR, "Got a 2nd query"###,query2
            query2.addToTableList(tableName)
            if not fetchAllRows: query2.setRowCacheSize(3)
            else: query2.setRowCacheSize(0)
            if doDefOut:
                query2.defineOutput(outBuffer)
                print MSGHDR, "Defined the output"
            else:
                print MSGHDR, "Did NOT define the output"

            # Get two cursors
            cursor=query.execute()
            print MSGHDR, "Got a cursor"###,cursor
            cursor2=query2.execute()
            print MSGHDR, "Got a 2nd cursor"###,cursor2
            
            # Iterate over the cursor
            nrows = 0
            nrows2 = 0
            hasNext=True
            hasNext2=True
            while hasNext or hasNext2:
                if hasNext and nrows <= nrows2:            
                    hasNext=cursor.next()
                    if hasNext:
                        nrows = nrows + 1
                        row = cursor.currentRow()
                        print MSGHDR, "Cursor #1: current row:",row
                else:
                    hasNext2=cursor2.next()
                    if hasNext2:
                        nrows2 = nrows2 + 1
                        row = cursor2.currentRow()
                        print MSGHDR, "Cursor #2: current row:",row
            print MSGHDR, "Cursor #1 - retrieved",nrows,"rows"
            print MSGHDR, "Cursor #2 - retrieved",nrows2,"rows"
            if nrows == 20: print "OK"
            else: print "ERROR"
            if nrows2 == 20: print "OK"
            else: print "ERROR"

        # Commit the transactions
        session.transaction().commit()
        session2.transaction().commit()
            
#----------------------------------------------------------------------------

except Exception, inst:

    print MSGHDR, "ERROR! Exception caught:", inst

except:

    print MSGHDR, "ERROR! Unknown exception caught:", sys.exc_info()[0]

#----------------------------------------------------------------------------

# Sleep a while to keep the connection(s) open
print MSGHDR, "Sleep a bit"
time.sleep(3)
print MSGHDR, "Slept a bit"
        
print MSGHDR, "Exiting"
