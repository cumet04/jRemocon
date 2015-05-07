from wsgiref.simple_server import make_server
import datetime
import json
import sys
import signal
import threading

class IRProxy(object):
    
    def __init__(self):
        self.path_functions = {
                '/': (self.showHelp,
                    ('show this message.',)),
                '/help': (self.showHelp,
                    ('show this message.',)),
                '/request': (self.requestExec,
                    ('',)),
                '/register': (self.registerSignal,
                    ('',)),
                '/generate': (self.generateSignal,
                    ('',))
            }

    def __call__(self, environ, start_response):
        path = environ['PATH_INFO']
        query = environ['QUERY_STRING']
        # headers = [('Content-type', 'application/json; charset=utf-8')]
        headers = [('Content-type', 'text/plain; charset=utf-8')]

        if path in self.path_functions:
            start_response('200 OK', headers)
            result = self.path_functions[path][0](query, environ)
            return [result.encode('utf-8')]
        else:
            start_response('404 Not found', headers)
            return ['404 Not found'.encode("utf-8")]

# API functions ----------------------------------------------------------------
    def showHelp(self, query_string, environ):
#TODO: impl
        return ""
    def requestExec(self, query_string, environ):
        return ""
    def registerSignal(self, query_string, environ):
        return ""
    def generateSignal(self, query_string, environ):
        return ""

# entry point ------------------------------------------------------------------

application = IRProxy()

if __name__ == '__main__':
    print("test")
    # server = make_server('', 80, application)
    # signal.signal(signal.SIGINT, lambda n,f : server.shutdown())
    # t = threading.Thread(target=server.serve_forever)
    # t.start()
