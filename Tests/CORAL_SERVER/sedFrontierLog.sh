#! /bin/bash
if [ "$1" == "" ]; then
  echo "Usage: $0 fileName"
  exit 1
fi
date1=`date +'%a %b %d'`
date2=`date +'%a, %d %b %Y'`
cat $1 \
  | sed "s/$date1 ..:..:.. ..../www MMM DD hh:mm:ss YYYY/" \
  | sed "s/$date2 ..:..:../www, DD MMM YYYY hh:mm:ss/" \
  | sed "s/client 2.7.14 ..... avalassi(14546) Andrea Valassi/client 2.7.14 xxxxx USER/"
