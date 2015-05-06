#!/bin/python

import subprocess
import os
import hashlib

class jRemoconCGI(object):
    def __init__(self):
        self.isLog = True
# TODO: If it uses dict, help message's order is not guaranteed. In addition,
# '/' and 'help' are same in its context but each has its dict-item.
# To solve these problems, another approach is needed.
        self.path_functions = {
            '/' : (self.showHelp,
                ('show this help page.',)),
            '/help' : (self.showHelp,
                ('show this help page.',)),
            '/send' : (self.sendSignal,
                ('emit specified signal as infrared signal.',
                'usage : /send?pulse={pulsewidth}&signal={signalstring}')),
            '/lirc/generate' : (self.generateSignalString,
                ('',)),
            '/lirc/clear' : (self.clearCache,
                ('',)),
            '/lirc/restart' : (self.restartLirc,
                ('',))
            }

    def printLog(self, message):
        if self.isLog: print("log: " + message)

# API functions
    def sendSignal(self, query_str):
        # TODO: share isLog with shell script
        self.printLog("sendSignal")
        subprocess.Popen(['bash', 'jRemocon_send.sh'])


    def generateSignalString(self, query_str):
        self.printLog("generateSignalString")
        return


    def clearCache(self, query_str):
        self.printLog("clearCache")
#スケルトンを用意してmv
        return

    def restartLirc(self, query_str):
        self.printLog("restartLirc")
        return

    def showHelp(self, query_str):
        self.printLog("showHelp")
        for api in self.path_functions:
            print('- ' + api)
            for line in self.path_functions[api][1]:
                print('  ' + line)
            print('')
        return


# cgi entry point ##############################################################

req_path = os.environ.get('PATH_INFO')
if req_path == None: req_path = '/'
query_str = os.environ.get('QUERY_STRING')
if query_str != None: query_str = query_str.encode('utf-8')

print("Content-type: text/plain")
print("")

cgi = jRemoconCGI()

if req_path in cgi.path_functions:
    cgi.path_functions[req_path][0](query_str)
else:
    print("invalid API is called : " + req_path)
