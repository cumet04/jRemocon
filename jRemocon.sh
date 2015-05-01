#!/bin/sh 

conf_file="/etc/lirc/lircd.conf"

echo "Content-type: text/plain"
echo ""


lirc_opname=`echo $QUERY_STRING | sha512sum | sed 's/\ //g'`

if irsend LIST "jremocon" "$lirc_opname" &> /dev/null ; then
    irsend SEND_ONCE "jremocon" "$lirc_opname" && echo "send ok"
    exit
fi

# if signal isn't found in lircd.conf ####################
echo "info : signal isn't found on DB. genarate data."

# generate raw_code
raw_code=`echo "${QUERY_STRING}" | signal_string -d | width_array | cut -d" " -f 2-`

# edit lircd.conf
insert_line=`cat $conf_file | grep -n "end raw_codes" | sed 's/:.*$//'`
sudo sed -i -e "${insert_line}i ${raw_code}" $conf_file
sudo sed -i -e "${insert_line}i name ${lirc_opname}" $conf_file

# restart service lircd
sudo systemctl restart lircd
sleep 1

# send signal
irsend SEND_ONCE "jremocon" "$lirc_opname" && echo "send ok"
