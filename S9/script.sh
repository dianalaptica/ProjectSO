#!/bin/sh

if ! test $# -eq 1
then 
    echo "bash script <c>"
    exit 1
fi

isalphanum=`echo $1 | grep "^[a-zA-Z0-9]$"`
if test -z "$isalphanum"
then
    echo "argument nu este alfanumeric"
    exit 1
fi

count=0
while read line
do
    match=`echo $line | grep -E  "^[A-Z][a-zA-Z0-9 ,]*[.!?]$" | grep -E -v ", ?si" | grep $1`
    if ! test -z "$match"
    then
        count=`expr $count + 1`
    fi
done

echo "$count"

exit 0
