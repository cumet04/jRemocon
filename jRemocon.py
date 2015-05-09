#!/usr/bin/python
# -*- coding: utf-8 -*-

from wsgiref.simple_server import make_server
import subprocess
import hashlib
import signal
import threading
import io

isLog = True
port_num = 8080
lircd_conf = '/etc/lirc/lircd.conf'
lircd_conf_skel = '/etc/lirc/lircd.conf.skel'

class jRemocon(object):
    
    def __init__(self):
        self.path_functions = {
            '/' : (self.showHelp,
                ('show this page.',)),
            '/help' : (self.showHelp,
                ('show this page.',)),
            '/send' : (self.sendSignal,
                ('emit specified signal as infrared signal.',
                'usage : /send?pulse={pulsewidth}&signal={signalstring}')),
            '/lirc/clear' : (self.clearCache,
                ("clear lirc's cache DB (/etc/lirc/lircd.conf) and restart lirc daemon.",)),
            '/lirc/restart' : (self.restartLirc,
                ('restart lirc daemon.',)),
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


# API functions
    def sendSignal(self, query_str):
        printLog("sendSignal")

        if query_str == None:
            return io.StringIO("error: target signal is not specified.")
        signal_hash = hashlib.sha512(query_str.encode('utf-8')).hexdigest()
        printLog("generate hash : " + signal_hash)

        # check whether requested signal exists in lirc-DB
        lirc_pipe = subprocess.Popen(['irsend', 'LIST', 'jremocon', signal_hash],
                                      stdout=subprocess.PIPE,
                                      stderr=subprocess.PIPE)
        lirc_stdout, lirc_stderr = lirc_pipe.communicate()
        lirc_out = (lirc_stdout + lirc_stderr).decode('utf-8')
 
        # error check
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
            printLog("target signal isn't found on DB. add it to DB")

            # generate lirc raw_code
            gen_command = 'signal_string -d | width_array | cut -d" " -f 2-'
            generate = subprocess.Popen(['bash', '-c', gen_command],
                        stdin=subprocess.PIPE, stdout=subprocess.PIPE)
            (stdout, stderr) = generate.communicate(query_str.encode('utf-8'))
            raw_code = stdout.decode('utf-8')

            # append the code to lircd.conf
            conf_file = open(lircd_conf, 'r+')
            new_conf = io.StringIO()
            for line in conf_file:
                new_conf.write(line)
                if 'begin raw_codes' in line:
                    new_conf.write("name " + signal_hash + "\n")
                    new_conf.write(raw_code)
            conf_file.seek(0)
            conf_file.write(new_conf.getvalue())
            conf_file.close()

            self.restartLirc(None)

        # send signal with irsend
        subprocess.Popen(['irsend', 'SEND_ONCE', 'jremocon', signal_hash])
        return io.StringIO('execute irsend')


    def clearCache(self, query_str):
        result = io.StringIO()
        printLog("clearCache")

        new_conf = io.StringIO()
        with open(lircd_conf, 'r') as conf_file:
            isSignal = False
            for line in conf_file:
                if 'end raw_codes' in line: isSignal = False
                if isSignal == False: new_conf.write(line)
                if 'begin raw_codes' in line: isSignal = True
        with open(lircd_conf, 'w') as conf_file:
            # write clean conf
            conf_file.seek(0)
            conf_file.write(new_conf.getvalue())

        self.restartLirc(None)
        print('copy skeleton to original conf.', file=result)
        return result


    def restartLirc(self, query_str):
        result = io.StringIO()
        printLog("restartLirc")

        pidof = subprocess.Popen(['pidof', 'lircd'])
        pidof.wait()
        if pidof.returncode == 0:
            subprocess.check_call(['sudo', 'pkill', 'lircd'])
        subprocess.check_call(['sudo', 'lircd'])

        print('restart lircd.', file=result)
        return result


    def showHelp(self, query_str):
        printLog("showHelp")
        result = io.StringIO()
        for api in self.path_functions:
            print('- ' + api, file=result)
            for line in self.path_functions[api][1]:
                print('  ' + line, file=result)
            print('', file=result)
        return result


# entry point ------------------------------------------------------------------

def printLog(message):
    if isLog: print("log: " + message)

application = jRemocon()

if __name__ == '__main__':
    server = make_server('', port_num, application)
    signal.signal(signal.SIGINT, lambda n,f : server.shutdown())
    t = threading.Thread(target=server.serve_forever)
    t.start()
