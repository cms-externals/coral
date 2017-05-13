#! /bin/bash
if [ "$1" == "" ]; then
  echo "Usage: $0 fileName"
  exit 1
fi

date1a=`date +%Y-%b-%d`
date1b=`date -dyesterday +%Y-%b-%d`
date2a=`date +%Y-%m-%d`
date2b=`date -dyesterday +%Y-%m-%d`
# See http://www.regular-expressions.info/quickstart.html
cat $1 \
  | sed "s/$date1a ..:..:../YYYY-MMM-DD hh:mm:ss/" \
  | sed "s/$date1b ..:..:../YYYY-MMM-DD hh:mm:ss/" \
  | sed "s/$date2a ..:..:..,.../YYYY-MMM-DD hh:mm:ss,nnn/" \
  | sed "s/$date2a ..:..:..,../YYYY-MMM-DD hh:mm:ss,nnn/" \
  | sed "s/$date2a ..:..:..\..../YYYY-MMM-DD hh:mm:ss\.nnn/" \
  | sed "s/$date2a ..:..:..\.../YYYY-MMM-DD hh:mm:ss\.nnn/" \
  | sed "s/$date2b ..:..:..,.../YYYY-MMM-DD hh:mm:ss,nnn/" \
  | sed "s/$date2b ..:..:..,../YYYY-MMM-DD hh:mm:ss,nnn/" \
  | sed "s/$date2b ..:..:..\..../YYYY-MMM-DD hh:mm:ss\.nnn/" \
  | sed "s/$date2b ..:..:..\.../YYYY-MMM-DD hh:mm:ss\.nnn/" \
  | sed -r 's/from port ([0-9])*/from port nnnnn/g' \
  | sed -r 's/from 127.0.0.1:([0-9])*/from 127.0.0.1:nnnnn/g' \
  | sed -r 's/from \( 127.0.0.1, ([0-9])* \)/from ( 127.0.0.1, nnnnn )/g' \
  | sed -r 's/thread 0x([a-f0-9])*/thread 0xNNNNNNNN/g' \
  | sed -r 's/ID=[a-f0-9]{8}-([a-f0-9]{4}-){3}[a-f0-9]{12}/ID=NNNNNN-NNNN-NNNN-NNNN-NNNNNNNNNNNN/g'\
  | sed -r 's/\[0x[a-f0-9]{8}\]\+([a-f0-9])*/[0xNNNNNNNN]+NNNNNNNN/g' \
  | sed -r 's/msgSvc 0x[a-f0-9]{8}/msgSvc 0xNNNNNNNN/g' \
  | sed -r 's/service 0x[a-f0-9]{8}/service 0xNNNNNNNN/g' \
  | sed -r 's/location <0x[a-f0-9]{8}/location <0xNNNNNNNN/g' \
  | sed -r 's/\$Id([^\$])*\$/$[CvsId]$/' \
  | sed -r 's/\$Name([^\$])*\$/$[CvsName]$/'
