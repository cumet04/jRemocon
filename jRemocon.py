#!/usr/bin/python
# -*- coding: utf-8 -*-

from wsgiref.simple_server import make_server
import datetime
import subprocess
import json
import sys
import hashlib
import signal
import threading
import io

class jRemocon(object):
    
    def __init__(self):
        self.isLog = True
        self.path_functions = {
            '/' : (self.showHelp,
                ('show this help page.',)),
            '/help' : (self.showHelp,
                ('show this help page.',)),
            '/send' : (self.sendSignal,
                ('emit specified signal as infrared signal.',
                'usage : /send?pulse={pulsewidth}&signal={signalstring}')),
            '/lirc/clear' : (self.clearCache,
                ('',)),
            '/lirc/restart' : (self.restartLirc,
                ('',))
            }

    def __call__(self, environ, start_response):
        path = environ['PATH_INFO']
        query = environ['QUERY_STRING']
        # headers = [('Content-type', 'application/json; charset=utf-8')]
        headers = [('Content-type', 'text/plain; charset=utf-8')]

        if path in self.path_functions:
            start_response('200 OK', headers)
            result = self.path_functions[path][0](query)
            return [result.getvalue().encode('utf-8')]
        else:
            start_response('404 Not found', headers)
            return ['404 Not found'.encode("utf-8")]

    def printLog(self, message):
        if self.isLog: print("log: " + message)

# API functions
    def sendSignal(self, query_str):
        self.printLog("sendSignal")

        if query_str == None:
            return io.StringIO("error: target signal is not specified.")
        signal_hash = hashlib.sha512(query_str.encode('utf-8')).hexdigest()
        self.printLog("generate hash : " + signal_hash)

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
            return io.StringIO(
                "error: irsend returns no response. lirc-DB may be broken.")
        elif 'irsend: could not connect to socket' in lirc_out:
            return io.StringIO('error: lircd may not run.')
        else:
            result = io.StringIO()
            print('error: irsend returns unexpected error:', file=result)
            print(lirc_out, file=result)
            return result

        # add signal to DB
        if signal_exists == False:
            self.printLog("target signal isn't found on DB. add it to DB")
            subprocess.Popen(['bash', 'jRemocon_append.sh', query_str])

        # send signal with irsend
        subprocess.Popen(['irsend', 'SEND_ONCE', 'jremocon', signal_hash])
        return io.StringIO('execute irsend')

    def clearCache(self, query_str):
        result = io.StringIO()
        self.printLog("clearCache")
        shutil.copy('/etc/lirc/lircd.conf.skel', '/etc/lirc/lircd.conf')
        print('copy skeleton to original conf.', file=result)
        return result

    def restartLirc(self, query_str):
        result = io.StringIO()
        self.printLog("restartLirc")
        subprocess.Popen(['bash', 'jRemocon_restartLirc.sh'])
        print('run restart script.', file=result)
        return result

    def showHelp(self, query_str):
        self.printLog("showHelp")
        result = io.StringIO()
        for api in self.path_functions:
            print('- ' + api, file=result)
            for line in self.path_functions[api][1]:
                print('  ' + line + '\n', file=result)
        return result


# entry point ------------------------------------------------------------------

application = jRemocon()

if __name__ == '__main__':
    server = make_server('', 8080, application)
    signal.signal(signal.SIGINT, lambda n,f : server.shutdown())
    t = threading.Thread(target=server.serve_forever)
    t.start()
