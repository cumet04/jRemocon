#!/bin/bash 

conf_file="/etc/lirc/lircd.conf"


lirc_opname=$(echo $QUERY_STRING | sha512sum | sed 's/\ //g')

ir_list=$(irsend LIST "jremocon" "$lirc_opname")
returncode=$?
if $returncode; then
    signal_exists=true
elif [[ $ir_list =~ 'irsend: unknown command' ]];then
    signal_exists=false
elif [[ $ir_list = '' ]];then
    echo 'error: irsend returns no response. lirc-DB may be broken.'
    exit 1
elif [[ $ir_list =~ 'irsend: could not connect to socket' ]];then
    echo 'irsend: could not connect to socket'
    exit 1
else
    echo 'error: irsend returns unexpected error:'
    echo $ir_list
    exit 1
fi

if ! $signal_exists ; then
    echo "info : signal isn't found on DB. genarate data."

    # generate raw_code
    raw_code=`echo "${QUERY_STRING}" | signal_string -d | width_array | cut -d" " -f 2-`

    # edit lircd.conf
    insert_line=`cat $conf_file | grep -n "end raw_codes" | sed 's/:.*$//'`
    sed -i -e "${insert_line}i ${raw_code}" $conf_file
    sed -i -e "${insert_line}i name ${lirc_opname}" $conf_file

    # restart service lircd
    sudo systemctl restart lircd
    sleep 1
fi

# send signal
irsend SEND_ONCE "jremocon" "$lirc_opname" && echo "send ok"
