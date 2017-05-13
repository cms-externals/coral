# $Id: CMT_userenv.csh,v 1.2 2012/02/07 17:18:15 avalassi Exp $

# Reset CMTPROJECTPATH to use a private copy of the latest nightlies
if ( "$CMTCONFIG" == "x86_64-slc6-clang30-opt" ) then
  if ( "$HOST" == "lxbuild101.cern.ch" ) then
    setenv CMTPROJECTPATH /home/avalassi/nightlies/dev/20120207Tue
  else
    setenv CMTPROJECTPATH /afs/cern.ch/sw/lcg/app/releases/COOL/internal/nightlies/dev/20120207Tue
  endif
endif

