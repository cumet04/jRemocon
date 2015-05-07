#!/bin/bash 

conf_file="/etc/lirc/lircd.conf"

# generate raw_code
raw_code=`echo "$1" | signal_string -d | width_array | cut -d" " -f 2-`

# edit lircd.conf
insert_line=`cat $conf_file | grep -n "end raw_codes" | sed 's/:.*$//'`
sed -i -e "${insert_line}i ${raw_code}" $conf_file
sed -i -e "${insert_line}i name ${lirc_opname}" $conf_file
