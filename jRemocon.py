#!/usr/bin/python
# -*- coding: utf-8 -*-

from wsgiref.simple_server import make_server
import subprocess
import hashlib
import json
import signal
import os
import sys
import threading
import io


class jRemocon(object):
    
    def __init__(self):
#TODO: use alternative instead of dict
        self.path_functions = {
            '' : self.showHelp,
            '/' : self.showHelp,
            '/help' : self.showHelp,
            '/send' : self.sendSignal,
            '/lirc/clear' : self.clearCache,
            '/lirc/restart' : self.restartLirc
            }

    def __call__(self, environ, start_response):
        path = environ['PATH_INFO']
        query = parse_qs(environ['QUERY_STRING'])
        headers = [('Content-type', 'application/xml; charset=UTF-8')]

        method = None
        if path.startswith('/' + url_prefix):
            method = path.partition(url_prefix)[2]

        result = None
        if method in self.path_functions:
            start_response('200 OK', headers)
            result = self.path_functions[method][0](query)
            result = result.getvalue().encode('utf-8')
        else:
            start_response('404 Not found', headers)
            result = '404 Not found'.encode("utf-8")

        # generate response
        # ACD周りのサービスがxmlでのレスポンスを要求するため強引に対応
        xml_head = '<ns:jRemoconResponse xmlns:ns="http://jRemocon"><ns:return>'
        xml_tail = '</ns:return></ns:jRemoconResponse>'
        response = xml_head + result + xml_tail
        return [response]


# API functions
    def sendSignal(self, query_param):
        """
        emit specified signal as infrared signal.
        usage : /send?pulse={pulsewidth}&signal={signalstring}
        """
        printLog("sendSignal")

        if query_str is None:
            return io.StringIO("error: target signal is not specified.")
        query_str = "pulse=%d&signal=%s".format(
                        query_param['pulse'], query['signal'])
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
        return io.StringIO('is;ok')


    def clearCache(self, query_param):
        """
        clear lirc's cache DB (/etc/lirc/lircd.conf) and restart lirc daemon.
        """
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


    def restartLirc(self, query_param):
        """
        restart lirc daemon.
        """
        result = io.StringIO()
        printLog("restartLirc")

        pidof = subprocess.Popen(['pidof', 'lircd'])
        pidof.wait()
        if pidof.returncode == 0:
            subprocess.check_call(['sudo', 'pkill', 'lircd'])
        subprocess.check_call(['sudo', 'lircd'])

        print('restart lircd.', file=result)
        return result


    def showHelp(self, query_param):
        """
        show this help message.
        """
        printLog("showHelp")
        result = io.StringIO()
        for api in self.path_functions:
            print('- ' + api, file=result, end="")
            print(textwrap.dedent(self.path_functions[api].__doc__), file=result)
        return result


# entry point ------------------------------------------------------------------

def printLog(message):
    if isLog: print("log: " + message)


class Config(object):
    def loadparam(self, conf_dict, param_name):
        if param_name in conf_dict:
            self.__dict__[param_name] = conf_dict[param_name]
            return True
        else:
            return False

script_path = os.path.abspath(os.path.dirname(__file__))
conf_name = script_path + '/conf/jRemocon.conf'
conf = Config()

if __name__ == '__main__':
    # load and check config
    if len(sys.argv) == 2:
        conf_name = sys.argv[1]
    with open(conf_name, 'r') as conf_file:
        conf_dict = json.loads(conf_file.read())
        if conf.loadparam(conf_dict, 'isLog') and \
           conf.loadparam(conf_dict, 'url_prefix') and \
           conf.loadparam(conf_dict, 'port_num') and \
           conf.loadparam(conf_dict, 'lircd_conf'): pass
        else:
            print("error: load config file failed; lack of parameter.")
            quit(1)

    # start server
    application = jRemocon()
    server = make_server('', port_num, application)
    signal.signal(signal.SIGINT, lambda n,f : server.shutdown())
    t = threading.Thread(target=server.serve_forever)
    t.start()
