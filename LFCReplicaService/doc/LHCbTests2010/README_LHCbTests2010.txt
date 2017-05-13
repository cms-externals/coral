-------------------------------------------------------------------------------
Preliminary requirements: Grid certificates (AndreaV - April 2010)
-------------------------------------------------------------------------------

Download a certificate for your browser from ca.cern.ch, eg for IE: 
  https://ca.cern.ch/ca/Certificates/certrequest.aspx

Follow the instructions on:
  https://ca.cern.ch/ca/Help/?kbid=024010
Export the certificate, then build the usercert.pem and userkey.pm files:
  openssl pkcs12 -in myCert.p12 -clcerts -nokeys -out ~/.globus/usercert.pem
  openssl pkcs12 -in myCert.p12 -nocerts -out ~/.globus/userkey.pem
  chmod go-rw ~/.globus/userkey.pem
  rm myCert.p12

You should now be able to execute grid-proxy-init successfully
(after setting uo the environment as specified below).

-------------------------------------------------------------------------------
Setup for tests of R/O functionality in LHCb (AndreaV - April 2010)
-------------------------------------------------------------------------------

This logfile explains the setup used for R/O tests of LFCReplicaSvc 
for LHCb. It reuses and complements the info provided by AlexK on
https://savannah.cern.ch/task/?9774#comment22

For CORAL237 the full list of commands needed are
  tcsh
  cd /afs/cern.ch/sw/lcg/app/releases/CORAL/CORAL_2_3_7/src/config/cmt/
  setenv CMTCONFIG slc4_ia32_gcc34_dbg
  source CMT_env.csh
  source setup.csh
  setenv LFC_HOST lfc-lhcb-ro.cern.ch
  setenv LD_LIBRARY_PATH /afs/cern.ch/sw/lcg/external/Grid/voms-api-cpp/1.8.9-2/slc4_ia32_gcc34/lib:$LD_LIBRARY_PATH
  grid-proxy-init
  setenv X509_CERT_DIR /afs/cern.ch/project/gd/LCG-share/current/external/etc/grid-security/certificates/
  setenv CORAL_LFC_BASEDIR /grid/lhcb/database
  coral_replica_manager -ls -l CondDB
This successfully prints out 21 replicas. 

A similar setup is needed for the current HEAD of CORAL/LCGCMT.
  tcsh
  cd ~/myLCG/CORAL_HEAD/src/config/cmt
  setenv CMTCONFIG slc4_ia32_gcc34_dbg
  source CMT_env.csh
  source setup.csh
  setenv LFC_HOST lfc-lhcb-ro.cern.ch
  setenv LD_LIBRARY_PATH /afs/cern.ch/sw/lcg/external/Grid/voms-api-cpp/1.8.9-2/slc4_ia32_gcc34/lib:$LD_LIBRARY_PATH
  grid-proxy-init
  setenv X509_CERT_DIR /afs/cern.ch/project/gd/LCG-share/current/external/etc/grid-security/certificates/
  setenv CORAL_LFC_BASEDIR /grid/lhcb/database
  coral_replica_manager -ls -l CondDB
This successfully prints out 21 replicas. 

Note that grid-proxy-init is used instead of voms-proxy-init. 
My certificate does not have a VOMS extension (I checked from
another window using voms-proxy-info, after setting that up too).
I am not sure how the LFC server recognizes me as a member of LHCb.
I guess that it is up to the LFC server to contact VOMS for that?

Before converging on the recipe above, I had to solve several issues:

1. LFC_HOST must be defined
+++++++++++++++++++++++++++

The following
  tcsh
  cd /afs/cern.ch/sw/lcg/app/releases/CORAL/CORAL_2_3_7/src/config/cmt/
  setenv CMTCONFIG slc4_ia32_gcc34_dbg
  source CMT_env.csh
  source setup.csh
  coral_replica_manager -ls -l CondDB

fails with
  CORAL/Services/LFCReplicaService Error Access to LFC server "" failed. Aborting transaction.
  ERROR: LFC host has not been defined ( CORAL : "LFCReplicaService::listReplicas" from "CORAL/Services/LFCReplicaService" )

A different error message is printed for the HEAD version,
  ERROR: LFC host has not been defined

2. The vomsapicpp library must be added to LD_LIBRARY_PATH
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

The following
  tcsh
  cd /afs/cern.ch/sw/lcg/app/releases/CORAL/CORAL_2_3_7/src/config/cmt/
  setenv CMTCONFIG slc4_ia32_gcc34_dbg
  source CMT_env.csh
  source setup.csh
  setenv LFC_HOST lfc-lhcb-ro.cern.ch
  coral_replica_manager -ls -l CondDB

fails with the cryptic message
  send2nsd: NS002 - send error : Could not load a security plugin
  CORAL/Services/LFCReplicaService Error Access to LFC server "lfc-lhcb-ro.cern.ch" failed. Aborting transaction.
  ERROR: Could not start the transaction. Function "lfc_starttrans" returned error code=2707 label=UNRESOLVED ( CORAL : "LFCReplicaService::listReplicas" from "CORAL/Services/LFCReplicaService" )

A different error message is printed for the HEAD version,
  send2nsd: NS002 - send error : Could not load a security plugin
  send2nsd: NS002 - send error : Could not load a security plugin
  send2nsd: NS002 - send error : Could not load a security plugin
  ERROR: Can't create LFC session

I tried to have a look at several links:
  https://twiki.cern.ch/twiki/bin/view/LCG/CouldNotLoadSecurityMechanism
  https://savannah.cern.ch/bugs/?44044
but these were not useful (and CGSI_TRACE did not print out anything).

Alex suggested to use
  source /afs/cern.ch/project/gd/LCG-share/sl4/etc/profile.d/grid-env.csh
and this indeed worked better, but added too many libraries.
I was also able to run the command from the LHCb setup as andreav after
  SetupProject LHCbDirac
but this also added too many libraries.

I removed one library at a time and I realised that this was enough:
  setenv LD_LIBRARY_PATH /afs/cern.ch/project/gd/LCG-share/3.1.38-0/glite/lib:$LD_LIBRARY_PATH
but again there are too many libraries in that directory.
I identified the correct missing library using strace
  strace coral_replica_manager -ls -l CondDB | & grep glite | grep -v ENOENT
which prints out
  stat64("/afs/cern.ch/project/gd/LCG-share/3.1.38-0/glite/lib", {st_mode=S_IFDIR|0755, st_size=30720, ...}) = 0
  open("/afs/cern.ch/project/gd/LCG-share/3.1.38-0/glite/lib/libvomsapi_gcc32dbgpthr.so.0", O_RDONLY) = 4
I then saw that the library is also available as /afs/cern.ch/sw/lcg/external/Grid/voms-api-cpp/1.8.9-2/slc4_ia32_gcc34/lib/libvomsapi_gcc32dbgpthr.so.0

The library can be added manually for the moment.

Eventually, it would be better to add to LCGCMT package lcgdmcommon a 
dependency on vomsapicpp, because:
ldd /afs/cern.ch/sw/lcg/external/Grid/lcg-dm-common/1.7.2-1sec/slc4_ia32_gcc34/lib/libCsec_plugin_GSI_thread.so | grep voms
        libvomsapi_gcc32dbgpthr.so.0 => /afs/cern.ch/sw/lcg/external/Grid/voms-api-cpp/1.8.9-2/slc4_ia32_gcc34/lib/libvomsapi_gcc32dbgpthr.so.0 (0xf7f72000)

In fact, I found via strace that libvomsapi_gcc32dbgpthr.so.0 is loaded
after libCsec_plugin_GSI_thread.so, which itself is loaded after liblfc.so.
While 'ldd liblfc.so' does not return libCsec_plugin_GSI_thread.so, I think
that liblfc.so loads at runtime libCsec_plugin_GSI_thread.so:
hexdump -e '80/1 "%1_p" "\n"' /afs/cern.ch/sw/lcg/external/Grid/LFC/1.7.2-1sec/slc4_ia32_gcc34/lib/liblfc.so | grep libCsec_plugin 
returns:
"n linked list. Will try to load it..libCsec_plugin_%s..so._thread.so...."

3. A valid certificate must be obtained and X509_CERT_DIR must be defined
+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

The following
  tcsh
  cd /afs/cern.ch/sw/lcg/app/releases/CORAL/CORAL_2_3_7/src/config/cmt/
  setenv CMTCONFIG slc4_ia32_gcc34_dbg
  source CMT_env.csh
  source setup.csh
  setenv LFC_HOST lfc-lhcb-ro.cern.ch
  setenv LD_LIBRARY_PATH /afs/cern.ch/sw/lcg/external/Grid/voms-api-cpp/1.8.9-2/slc4_ia32_gcc34/lib:$LD_LIBRARY_PATH
  coral_replica_manager -ls -l CondDB

fails with
  send2nsd: NS002 - send error : client_establish_context: Could not find or use a credential
  CORAL/Services/LFCReplicaService Error Access to LFC server "lfc-lhcb-ro.cern.ch" failed. Aborting transaction.
  ERROR: Could not start the transaction. Function "lfc_starttrans" returned error code=2701 label=UNRESOLVED ( CORAL : "LFCReplicaService::listReplicas" from "CORAL/Services/LFCReplicaService" )

A different error message is printed for the HEAD version,
  send2nsd: NS002 - send error : client_establish_context: Could not find or use a credential
  send2nsd: NS002 - send error : client_establish_context: Could not find or use a credential
  send2nsd: NS002 - send error : client_establish_context: Could not find or use a credential
  ERROR: Can't create LFC session

Trying to ONLY get a certificate OR define X509_CERT_DIR is not enough:
  grid-proxy-destroy
  setenv X509_CERT_DIR /afs/cern.ch/project/gd/LCG-share/current/external/etc/grid-security/certificates/
  coral_replica_manager -ls -l CondDB
and also 
  grid-proxy-init
  unsetenv X509_CERT_DIR
  coral_replica_manager -ls -l CondDB
both fail with the same error messages printed above.

4. The correct CORAL_LFC_BASEDIR must be defined
++++++++++++++++++++++++++++++++++++++++++++++++

The following
  tcsh
  cd /afs/cern.ch/sw/lcg/app/releases/CORAL/CORAL_2_3_7/src/config/cmt/
  setenv CMTCONFIG slc4_ia32_gcc34_dbg
  source CMT_env.csh
  source setup.csh
  setenv LFC_HOST lfc-lhcb-ro.cern.ch
  setenv LD_LIBRARY_PATH /afs/cern.ch/sw/lcg/external/Grid/voms-api-cpp/1.8.9-2/slc4_ia32_gcc34/lib:$LD_LIBRARY_PATH
  setenv X509_CERT_DIR /afs/cern.ch/project/gd/LCG-share/current/external/etc/grid-security/certificates/
  grid-proxy-init
  coral_replica_manager -ls -l CondDB

fails with
  CORAL/Services/LFCReplicaService Error Logical connection string "/database/CondDB" not found. Closing transaction.
  ERROR: Logical connection string is unknown. ( CORAL : "LFCReplicaService::listReplicas" from "CORAL/Services/LFCReplicaService" )

A different error message is printed for the HEAD version,
  ERROR: LFC node was not found : CondDB ( CORAL : "LFCReplicaService::lookupNode" from "CORAL/Services/LFCReplicaService" )

The issues are solved only after setting also
  setenv CORAL_LFC_BASEDIR /grid/lhcb/database
  coral_replica_manager -ls -l CondDB
This displays 21 replicas for the connection string.

-------------------------------------------------------------------------------
Note about SetupProject in LHCb (AndreaV - April 2010)
-------------------------------------------------------------------------------

I had a chat with Marco about the LHCb setup as andreav using
  SetupProject LHCbDirac
This is a script initially developed by Marco himself, see
  https://twiki.cern.ch/twiki/bin/view/LHCb/SetupProject

I thought that this was internally calling grid-env.csh, but it does not!
The command creates a temporary cmt directory where it pulls in all
the relevant CMT dependencies and then simply sources the CMT setup!
The Grid setup largely comes via the LHCbGrid package.
==> We should have a look if we can/should move some of the LHCbGrid
    dependencies (eg vomsapicpp) and sets (eg X509_CERT_DIR) into LCGCMT.

If you want to keep the temporary cmt/requirements, you may set
the SPKEEPTEMPDIR environment variable (see /afs/cern.ch/lhcb/software/releases/LBSCRIPTS/LBSCRIPTS_v5r0/InstallArea/python/LbConfiguration/SetupProject.py).

-------------------------------------------------------------------------------
Performance comparison from R/O tests in LHCb (AndreaV - April 2010)
-------------------------------------------------------------------------------

I compared three versions of the software:
- CORAL231 (the version triggering the initial problems in April 2009)
- CORAL237 (the latest released, with a few minor improvements in CORAL232)
- CORALHEAD (including the latest patches by Alex)

For CORAL237 the full list of commands needed are
  tcsh
  cd /afs/cern.ch/sw/lcg/app/releases/CORAL/CORAL_2_3_7/src/config/cmt/
  setenv CMTCONFIG slc4_ia32_gcc34_dbg
  source CMT_env.csh
  source setup.csh
  setenv LFC_HOST lfc-lhcb-ro.cern.ch
  setenv LD_LIBRARY_PATH /afs/cern.ch/sw/lcg/external/Grid/voms-api-cpp/1.8.9-2/slc4_ia32_gcc34/lib:$LD_LIBRARY_PATH
  grid-proxy-init
  setenv X509_CERT_DIR /afs/cern.ch/project/gd/LCG-share/current/external/etc/grid-security/certificates/
  setenv CORAL_LFC_BASEDIR /grid/lhcb/database
  coral_replica_manager -ls -l CondDB
This successfully prints out 21 replicas. 

A similar setup is needed for the current HEAD of CORAL/LCGCMT.
  tcsh
  cd ~/myLCG/CORAL_HEAD/src/config/cmt
  setenv CMTCONFIG slc4_ia32_gcc34_dbg
  source CMT_env.csh
  source setup.csh
  setenv LFC_HOST lfc-lhcb-ro.cern.ch
  setenv LD_LIBRARY_PATH /afs/cern.ch/sw/lcg/external/Grid/voms-api-cpp/1.8.9-2/slc4_ia32_gcc34/lib:$LD_LIBRARY_PATH
  grid-proxy-init
  setenv X509_CERT_DIR /afs/cern.ch/project/gd/LCG-share/current/external/etc/grid-security/certificates/
  setenv CORAL_LFC_BASEDIR /grid/lhcb/database
  coral_replica_manager -ls -l CondDB
This successfully prints out 21 replicas. 

For CORAL231 the full list of commands needed are
  tcsh
  cd /afs/cern.ch/sw/lcg/app/releases/CORAL/CORAL_2_3_7/src/config/cmt/
  setenv CMTCONFIG slc4_ia32_gcc34_dbg
  source CMT_env.csh
  cd /afs/cern.ch/sw/lcg/app/releases/CORAL/CORAL_2_3_1/src/config/cmt/
  source setup.csh
  setenv LFC_HOST lfc-lhcb-ro.cern.ch
  setenv LD_LIBRARY_PATH /afs/cern.ch/sw/lcg/external/Grid/voms-api-cpp/1.8.9-2/slc4_ia32_gcc34/lib:$LD_LIBRARY_PATH
  grid-proxy-init
  setenv X509_CERT_DIR /afs/cern.ch/project/gd/LCG-share/current/external/etc/grid-security/certificates/
  setenv CORAL_LFC_BASEDIR /grid/lhcb/database
  coral_replica_manager -ls -l CondDB
This successfully prints out 21 replicas. 

The three logfiles above are in
- coral_replica_manager_lsCondDB_coral231.txt
- coral_replica_manager_lsCondDB_coral237.txt
- coral_replica_manager_lsCondDB_coralhead.txt
A few comments are in https://savannah.cern.ch/task/?9774

-------------------------------------------------------------------------------
Setup for tests of R/W functionality in LHCb (AndreaV - April 2010)
-------------------------------------------------------------------------------

My goal was to reproduce the tests I had performed in 2008:
  http://cool.cvs/cgi-bin/cool.cgi/cool/contrib/PyCoralProxy/README.lfc?rev=1.1&content-type=text/vnd.viewcvs-markup

Setup the libraries for CORAL237:
  tcsh
  cd /afs/cern.ch/sw/lcg/app/releases/CORAL/CORAL_2_3_7/src/config/cmt/
  setenv CMTCONFIG slc4_ia32_gcc34_dbg
  source CMT_env.csh
  source setup.csh
  setenv LD_LIBRARY_PATH /afs/cern.ch/sw/lcg/external/Grid/voms-api-cpp/1.8.9-2/slc4_ia32_gcc34/lib:$LD_LIBRARY_PATH
  grid-proxy-init
  setenv X509_CERT_DIR /afs/cern.ch/project/gd/LCG-share/current/external/etc/grid-security/certificates/

With respect to the RO tests, the R/W master LFC server must be used.
  setenv LFC_HOST prod-lfc-lhcb-central.cern.ch

Try to create a test directory to see if you have R/W credentials:
  lfc-mkdir /grid/lhcb/user/a/avalassi/test

Check that it has been created:
  lfc-ls /grid/lhcb/user/a/avalassi
returns
  databases
  test

Remove the test directory:
  lfc-rm -r /grid/lhcb/user/a/avalassi/test

Check that it has been removed:
  lfc-ls /grid/lhcb/user/a/avalassi
returns only
  databases

Check if some CORAL replicas have already been defined:
  lfc-ls /grid/lhcb/user/a/avalassi/databases
returns 
  Oracle-avalassi

Try to delete the replica:
  setenv CORAL_LFC_BASEDIR /grid/lhcb/user/a/avalassi/databases
  coral_replica_manager -del -l Oracle-avalassi
Check if it has been deleted:
  lfc-ls /grid/lhcb/user/a/avalassi/databases
returns nothing.

Create a CORAL replica:
  setenv CORAL_LFC_BASEDIR /grid/lhcb/user/a/avalassi/databases
  coral_replica_manager -add -l Oracle-avalassi -c oracle://lcg_cool_nightly/lcg_cool -u lcg_cool -p xxxxxxxx
Check if it has been created:
  lfc-ls /grid/lhcb/user/a/avalassi/databases
returns 
  Oracle-avalassi

Create two XML files for that replica in the local directory:
  \rm -rf /tmp/avalassi/*.xml
  cd /tmp/avalassi/
  setenv CORAL_LFC_BASEDIR /grid/lhcb/user/a/avalassi/databases
  coral_replica_manager -exp -l Oracle-avalassi
  ls -1 /tmp/avalassi/*.xml
returns 
  /tmp/avalassi/authentication.xml  
  /tmp/avalassi/dblookup.xml

Print the two files:
  more /tmp/avalassi/*.xml
returns
  ::::::::::::::
  /tmp/avalassi/authentication.xml
  ::::::::::::::
  <?xml version="1.0" ?>
  <connectionlist>
    <connection name="oracle://lcg_cool_nightly/lcg_cool" >
      <parameter name="user" value="lcg_cool" />
      <parameter name="password" value="xxxxxxxx" />
    </connection>
  </connectionlist>
  ::::::::::::::
  /tmp/avalassi/dblookup.xml
  ::::::::::::::
  <?xml version="1.0" ?>
  <servicelist>
    <logicalservice name="/grid/lhcb/user/a/avalassi/databases/Oracle-avalassi" >
      <service name="oracle://lcg_cool_nightly/lcg_cool" accessMode="update" authentication="password" />
    </logicalservice>
  </servicelist>

-------------------------------------------------------------------------------
