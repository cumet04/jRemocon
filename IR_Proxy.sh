#!/bin/sh

echo "Content-type: text/plain"
echo ""


# parse parameter
IFS=\& eval 'param_str_list=($QUERY_STRING)'
for line in ${param_str_list[@]}
do
    if   [[ $line =~ deviceid= ]];  then device=`echo $line | sed 's/.*=//g'`
    elif [[ $line =~ operation= ]]; then operation=`echo $line | sed 's/.*=//g'`
    elif [[ $line =~ ip= ]];        then ip=`echo $line | sed 's/.*=//g'`
    fi
done


# parameter check
if [ -z "$device" ]; then
    echo "error: parameter not found : deviceid"
    exit -1
fi
if [ -z "$operation" ]; then
    echo "error: parameter not found : operation"
    exit -1
fi
if [ -z "$ip" ]; then
    echo "error: parameter not found : ip"
    exit -1
fi


# query a signal corresponding to given the $device and $operation
alias sql='mysql -h localhost -u inomoto -pinomoto testdb'

query_str="select SignalData from DeviceOperation \
             where DeviceClassID='${device}' \
               and Operation='${operation}' \
               and Protocol='IR'"
signal=$(echo $query_str | sql | sed -n "2p")
if [ -z "$signal" ]; then
    echo "error: signal not found"
    exit -1
fi

# send signal to jRemocon
request_url="http://$ip/cgi-bin/jRemocon.sh?${signal}"
curl "$request_url" >&1 2>&1
