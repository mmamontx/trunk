i=0
for filename in tests/*; do
    d=$((i%2))
    if [ $d -eq 0 ]; then
        echo "cat" $filename
        cat $filename
        o=`python3 stock_charts.py < $filename`
    else
        eo=`cat $filename`
        echo
        echo $o $eo
        if [ $o != $eo ]; then
            exit -1
        fi
        echo
    fi
    i=$((i+1))
done
