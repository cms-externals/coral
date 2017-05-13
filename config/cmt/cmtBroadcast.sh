#! /bin/bash
#
# Usage:
# cmtBroadcast [-select="pkg1 pkg2 ..."] cmd
#
# Examples:
# cmtBroadcast make                    -> lib, utilitlies
# cmtBroadcast make all                -> lib, utilities
# cmtBroadcast make tests              -> tests
# cmtBroadcast make examples           -> examples
# cmtBroadcast make all tests          -> lib, utilities, tests
# cmtBroadcast make all tests examples -> lib, utilities, tests, examples
#
CMTPATH=""
if [ "$CMTUSERCONTEXT" = "" ]; then
  echo "CMTUSERCONTEXT is not set"
  cmt broadcast $*
else
  select=""
  if [ "${1:0:8}" == "-select=" ]; then
    select="$1"
    shift
  fi
  echo "CMTUSERCONTEXT is $CMTUSERCONTEXT"
  theCmtUserContext=$CMTUSERCONTEXT
  CMTUSERCONTEXT=""
  cmt broadcast "$select" "CMTUSERCONTEXT=$theCmtUserContext" $*
fi
