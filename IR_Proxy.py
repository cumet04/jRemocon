#!/usr/bin/python
# -*- coding: utf-8 -*-

from wsgiref.simple_server import make_server
from cgi import parse_qs
from urllib.request import urlopen
import mysql.connector
import datetime
import textwrap
import io
import json
import sys
import signal
import threading

isLog = True
url_prefix = 'IRProxy'
port_num = 8080
# db_host = 'localhost'
db_host = '192.168.100.182'
db_user = 'inomoto'
db_pass = 'inomoto'
db_name = 'testdb'
db_charset = 'utf8'
jRemocon_uri = 'http://192.168.0.34:8080/jRemocon/send?{0}'

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
        if path.startswith('/' + url_prefix):
            method = path.partition(url_prefix)[2]

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
#             for line in self.path_functions[api][1]:
#                 print('  ' + line, file=result)
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
        connect = mysql.connector.connect(user=db_user, password=db_pass,
                host=db_host, database=db_name, charset=db_charset)
        sql_cursor = connect.cursor()
        sql_query = "select SignalData from DeviceOperation " + \
                    "where DeviceClassID=%s " + \
                    "and Operation=%s " + \
                    "and Protocol='IR'"
        sql_cursor.execute(sql_query, (device, operation))
        signal = sql_cursor.fetchone()
        if signal == None:
            return io.StringIO('error: signal not found on DB')
        signal = signal[0]

        # send signal to jRemocon
        request_url = jRemocon_uri.format(signal)
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
        connect = mysql.connector.connect(user=db_user, password=db_pass,
                host=db_host, database=db_name, charset=db_charset)
        sql_cursor = connect.cursor()
        # sql_query= "select DeviceClassID,Operation,Description " + \
        sql_query= "select * " + \
                   "from DeviceOperation"
        sql_cursor.execute(sql_query)
        signals = sql_cursor.fetchall()

        for item in signals:
            print(item, file=result)
        return result


# entry point ------------------------------------------------------------------

def printLog(message):
    if isLog: print("log: " + message)


if __name__ == '__main__':
    application = IRProxy()
    server = make_server('', port_num, application)
    signal.signal(signal.SIGINT, lambda n,f : server.shutdown())
    t = threading.Thread(target=server.serve_forever)
    t.start()
