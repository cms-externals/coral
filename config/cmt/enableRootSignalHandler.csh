source setup.csh
setenv LD_LIBRARY_PATH_OLD $LD_LIBRARY_PATH
setenv ROOTSYS `cmt -use=ROOT:'v*':LCG_Interfaces show set_value ROOTSYS`
setenv LD_LIBRARY_PATH $ROOTSYS/lib:$LD_LIBRARY_PATH
setenv LD_PRELOAD libCore.so:libdl.so

