#!/bin/python
import subprocess
import os
import hashlib


def sendSignal(query_str):
    print("log: sendSignal")

# generate signal hash
    if query_str == None:
        print("error: target signal is not specified.")
        return
    signal_hash = hashlib.sha512(query_str).hexdigest()
    print("log: generate hash : " + signal_hash)
    
# check whether requested signal exists in lirc-DB
    lirc_pipe = subprocess.Popen(['irsend', 'LIST', 'jremocon', signal_hash],
                                 stdout=subprocess.PIPE,
                                 stderr=subprocess.PIPE)
    lirc_stdout, lirc_stderr = lirc_pipe.communicate()
    lirc_out = (lirc_stdout + lirc_stderr).decode('utf-8')

    signal_exists = None
    if lirc_pipe.returncode == 0:
        signal_exists = True
    elif 'irsend: unknown command' in lirc_out:
        # target signal isn't found on lirc-DB
        signal_exists = False
    elif lirc_out == "":
        print("error: irsend returns no response. lirc-DB may be broken.")
        return
    elif 'irsend: could not connect to socket' in lirc_out:
        print('error: lircd may not run.')
        return
    else:
        print('error: irsend returns unexpected error:')
        print(lirc_out)
        return

# add the signal to lirc-DB, if it isn't found on DB
    if signal_exists == False:
        print("log: target signal isn't found on DB. add it to DB")

# send signal with lirc:irsend

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

def restartLirc(query_str):
    print("restartLirc")
    return

def showHelp(query_str):
    print("showHelp")
    return


# cgi entry point ##############################################################
path_functions = {'/'              : showHelp,
                  '/help'          : showHelp,
                  '/send'          : sendSignal,
                  '/lirc/generate' : generateSignalString,
                  '/lirc/clear'    : clearCache,
                  '/lirc/restart'  : restartLirc}

req_path = os.environ.get('PATH_INFO')
if req_path == None: req_path = '/'
query_str = os.environ.get('QUERY_STRING')
if query_str != None: query_str = query_str.encode('utf-8')

print("Content-type: text/plain")
print("")

if req_path in path_functions:
    path_functions[req_path](query_str)
else:
    print("invalid API is called : " + req_path)
