#!/bin/bash

i=0
for entry in tests/*
do
    let "tmp = i % 2"
    if [ $tmp = 0 ]; then
        echo "$entry"
        python3 process_packages.py <$entry >$entry.txt
        tr -d '\r' <$entry.a >$entry.a.txt
        f1=`md5sum $entry.a.txt | sed -r "s/([0-9a-z]+) .*/\1/g"`
        f2=`md5sum $entry.txt | sed -r "s/([0-9a-z]+) .*/\1/g"`
        echo $f1 $f2
        if [ "$f1" = "$f2" ]; then
            echo "Equal!"
        else
            echo "Not equal!"
        fi
    fi
    let "i = i + 1"
done
