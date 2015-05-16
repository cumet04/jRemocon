#!/usr/bin/python
# -*- coding: utf-8 -*-

from wsgiref.simple_server import make_server
from cgi import parse_qs
from urllib.request import urlopen
import mysql.connector
import datetime
import textwrap
import io
import os
import json
import sys
import signal
import threading


class IRProxy(object):
    
    def __init__(self):
        self.path_functions = {
                '': self.showHelp,
                '/': self.showHelp,
                '/help': self.showHelp,
                '/request': self.requestExec,
                '/db/register': self.registerSignal,
                '/db/list': self.listSignalDB
            }

    def __call__(self, environ, start_response):
        path = environ['PATH_INFO']
        query = parse_qs(environ['QUERY_STRING'])
        headers = [('Content-type', 'text/plain; charset=utf-8')]

        method = None
        # eliminate path prefix from PATH_INFO.
        if path.startswith('/' + conf.url_prefix):
            method = path.partition(conf.url_prefix)[2]

        # execute API function if the requested API exists.
        if method in self.path_functions:
            start_response('200 OK', headers)
            result = self.path_functions[method](query)
            return [result.getvalue().encode('utf-8')]
        else:
            start_response('404 Not found', headers)
            return ['404 Not found'.encode("utf-8")]


# API functions ----------------------------------------------------------------
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

    def requestExec(self, query_param):
        """
        request appliance control to jRemocon.
        usage: /request?ip={jRemocon IP}&deviceid={target id}&operation={operation}
        """
        # parameter exist check
        if not 'ip' in query_param:
            return io.StringIO('error: parameter not found : ip')
        if not 'deviceid' in query_param:
            return io.StringIO('error: parameter not found : deviceid')
        if not 'operation' in query_param:
            return io.StringIO('error: parameter not found : operation')
        ip = query_param['ip'][0]
        device = query_param['deviceid'][0]
        operation = query_param['operation'][0]

        # query signal
#TODO: error handling
        connect = mysql.connector.connect(
                user=conf.db_user, password=conf.db_pass,
                host=conf.db_host, database=conf.db_name,
                charset=conf.db_charset)
        sql_cursor = connect.cursor()
        sql_query = "select SignalData from DeviceOperation " + \
                    "where DeviceClassID=%s " + \
                    "and Operation=%s " + \
                    "and Protocol='IR'"
        sql_cursor.execute(sql_query, (device, operation))
        signal = sql_cursor.fetchone()
        if signal is None:
            return io.StringIO('error: signal not found on DB')
        signal = signal[0]

        # send signal to jRemocon
        request_url = conf.jRemocon_uri + signal
#TODO: error handling
        result = urlopen(request_url).read().decode('utf-8')
        return io.StringIO("sent signal to jRemocon.")


    def registerSignal(self, query_param):
        """
        not implemented.
        """
        return io.StringIO("registerSignal")


    def listSignalDB(self, query_param):
        """
        list all signal data on SignalDB.
        """
        result = io.StringIO()
#TODO: error handling
        connect = mysql.connector.connect(
                user=conf.db_user, password=conf.db_pass,
                host=conf.db_host, database=conf.db_name,
                charset=conf.db_charset)
        sql_cursor = connect.cursor()
        sql_query= "select * from DeviceOperation"
        sql_cursor.execute(sql_query)
        signals = sql_cursor.fetchall()

        for item in signals:
            print(item, file=result)
        return result


# entry point ------------------------------------------------------------------

def printLog(message):
    if conf.isLog: print("log: " + message)

class Config(object):
    def loadparam(self, conf_dict, param_name):
        if param_name in conf_dict:
            self.__dict__[param_name] = conf_dict[param_name]
            return True
        else:
            return False

script_path = os.path.abspath(os.path.dirname(__file__))
conf_name = script_path + '/conf/IRProxy.conf'
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
           conf.loadparam(conf_dict, 'db_host') and \
           conf.loadparam(conf_dict, 'db_user') and \
           conf.loadparam(conf_dict, 'db_pass') and \
           conf.loadparam(conf_dict, 'db_name') and \
           conf.loadparam(conf_dict, 'db_charset') and \
           conf.loadparam(conf_dict, 'jRemocon_uri'): pass
        else:
            print("error: load config file failed; lack of parameter.")
            quit(1)

    # start server
    application = IRProxy()
    server = make_server('', conf.port_num, application)
    signal.signal(signal.SIGINT, lambda n,f : server.shutdown())
    t = threading.Thread(target=server.serve_forever)
    t.start()
