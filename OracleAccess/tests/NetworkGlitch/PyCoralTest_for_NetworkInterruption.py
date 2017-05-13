#!/usr/bin/env python

import os
import coral
from time import sleep
from ssh_tunnel import StoneHandler

class NetworkInterruption:
#    def __init__(self,Stone=True, StoneH =):
    def __init__(self, Stone=0, th=0):
        self.sh = Stone
        self.th = th
        
        print self.dbg("Contructor")
        
        os.environ['CORAL_DBLOOKUP_PATH']='/afs/cern.ch/sw/lcg/app/pool/db'
        os.environ['CORAL_AUTH_PATH']='/afs/cern.ch/sw/lcg/app/pool/db'
        os.environ['TNS_ADMIN']='.'
        
        if not self.sh:
            self.sh = StoneHandler()
            self.sh.activate()

    def dbg(self,msg):
        return "__NetworkInterruption n. "+str(self.th) +" ==> "+msg
        
    def netInterruption(self, time = 1, obj=""):
        print "__Interruption ==> "
        self.sh.deactivate()
        sleep(time)
        print "__Object ==> ", obj
        self.sh.activate()
        
    def connect(self):
        try:
            self.svc = coral.ConnectionService()
            self.session1 = self.svc.connect('oracle://lcg_coral_nightly_proxy/lcg_coral_nightly', accessMode = coral.access_Update )
            self.transaction = self.session1.transaction()
            self.transaction.start()
            self.schema = self.session1.nominalSchema()
            self.rowBuffer = coral.AttributeList()
            
        except Exception, e:
            print str(e)

    def commit(self):
        print self.dbg("Commit")
        self.transaction.commit()
        del self.session1
  
    def createTable(self, tableName):
        print self.dbg("Create Table")
        self.connect()
        try:

            self.schema.dropIfExistsTable( tableName )

            description = coral.TableDescription()
            description.setName( tableName)
            description.insertColumn( 'ID', 'int' )
            description.insertColumn( 'Data', 'float' )
            description.setPrimaryKey( 'ID' )
            
            tableHandle = self.schema.createTable( description )
            
            tableHandle.dataEditor().rowBuffer( self.rowBuffer );

            for i in range(20):
                self.rowBuffer["ID"].setData(i)
                self.rowBuffer["Data"].setData(i)
                tableHandle.dataEditor().insertRow( self.rowBuffer );

        except Exception, e:
            print str(e)

        self.commit()
        
    def query(self,tableName):
        print self.dbg("Query")
        self.connect()
        
        try:
            query = self.schema.tableHandle(tableName).newQuery()
            self.rowBuffer.extend("ID","int")
            self.rowBuffer.extend("Data","float")
            query.defineOutput( self.rowBuffer )
            
            counter = 0
            cursor = query.execute()
            while ( cursor.next() ):
                print self.rowBuffer[0], self.rowBuffer[1]
                
        except Exception, e:
            print str(e)

        self.commit()
        
if __name__ == '__main__':
    print "Start main"
    ni = NetworkInterruption(False)
#    ni.createTable("prova_1")
    ni.query("prova_1")
    os.system("killall ssh")
