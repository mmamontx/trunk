#!/bin/bash

i=0
for entry in tests/*
do
    echo "$entry"
    let "tmp = i % 2"
    #if [ $tmp = 1 ]; then
    if [ $tmp = 0 ]; then
        #python3 check_brackets.py <$entry >$entry.txt
        time python3 evacuation.py <$entry >$entry.txt
        #tr -d '\r' <$entry >$entry-tr.txt
        #f1=`md5sum $entry-tr.txt | sed -r "s/([0-9a-z]+) .*/\1/g"`
        f1=`md5sum $entry.txt | sed -r "s/([0-9a-z]+) .*/\1/g"`
        f2=`md5sum $entry.a | sed -r "s/([0-9a-z]+) .*/\1/g"`
        echo $f1 $f2
        if [ "$f1" = "$f2" ]; then
            echo "Equal!"
        else
            echo "Not equal!"
        fi
    fi
    let "i = i + 1"
done
