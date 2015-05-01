#!/bin/python
import os


def sendSignal(query_str):
    print("sendSignal")
    return
    # generate signal hash
    
    # check whether requested signal exists in lirc-DB
    
    # add the signal to lirc-DB
    
    # send signal with lirc:irsend


# lirc_opname=`echo $QUERY_STRING | sha512sum | sed 's/\ //g'`
# 
# if irsend LIST "jremocon" "$lirc_opname" &> /dev/null ; then
#     irsend SEND_ONCE "jremocon" "$lirc_opname" && echo "send ok"
#     exit
# fi
# 
# # if signal isn't found in lircd.conf ####################
# echo "info : signal isn't found on DB. genarate data."
# 
# # generate raw_code
# raw_code=`echo "${QUERY_STRING}" | \
#           /srv/http/signal_string -d | \
#           /srv/http/width_array`
# # remove pulse_width
# raw_code=`echo $raw_code | sed 's/^[0-9]* //'`
# 
# # edit lircd.conf
# insert_line=`cat $conf_file | grep -n "end raw_codes" | sed 's/:.*$//'`
# sudo sed -i -e "${insert_line}i ${raw_code}" $conf_file
# sudo sed -i -e "${insert_line}i name ${lirc_opname}" $conf_file
# 
# # restart service lircd
# sudo systemctl restart lircd
# sleep 1
# 
# # send signal
# irsend SEND_ONCE "jremocon" "$lirc_opname" && echo "send ok"


def generateSignalString(query_str):
    print("generateSignalString")
    return


def clearCache(query_str):
    print("clearCache")
    return


def showHelp(query_str):
    print("showHelp")
    return


# cgi entry point ##############################################################
path_functions = {'/'         : showHelp,
                  '/send'     : sendSignal,
                  '/generate' : generateSignalString,
                  '/clear'    : clearCache}

req_path = os.environ.get('PATH_INFO')
query_str = os.environ.get('QUERY_STRING')
if req_path == None: req_path = '/'

print("Content-type: text/plain")
print("")

if req_path in path_functions:
    path_functions[req_path](query_str)
else:
    print("invalid API is called : " + req_path)
