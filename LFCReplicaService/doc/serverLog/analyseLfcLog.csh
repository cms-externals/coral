#! /bin/tcsh -f

if ( "$1" == "" ) then
  echo "Usage: $0 logfile"
  echo "Example: $0 lfclhcbro_20090416.txt | sort > lfclhcbro_20090416_short.txt"
  echo "Example: $0 lfclhcbro_20100406.txt | sort > lfclhcbro_20100406_short.txt"
  exit 1
endif
set theLog=$1

set theAwkDir=`dirname $0`
setenv AWKPATH ${theAwkDir}

set theYear=2010
echo ${theLog} | grep 2009 > /dev/null
if ( $status == 0 ) set theYear=2009

awk -f analyseLfcLog.awk -vyear=$theYear ${theLog}
