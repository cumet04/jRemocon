#!/usr/bin/python3
# -*- coding: utf-8 -*-

import json
import os
import sys
import threading
import time
import subprocess
import hashlib
from flask import Flask, jsonify, request, url_for, abort, Response

from logging import getLogger,StreamHandler,DEBUG


class jRemocon(object):
    def __init__(self):
        pass


    def show_help(self):
        logger.info('run show_help')
        return jsonify(res='help')


    def send_signal(self, pulse, signal):
        logger.info('run send_signal: pulse={0}, signal={1}'.format(pulse, signal))

        # generate signal hash
        hash_seed = signal + str(pulse)
        signal_hash = hashlib.sha512(hash_seed.encode('utf-8')).hexdigest()

        # check signal existing on DB
        is_exist = self.has_signal(signal_hash)
        if is_exist == 0:
            logger.info('The signal exists on LIRC-DB.')
        elif is_exist == -1:
            # add signal to DB
            logger.info("The signal doesn't exists on LIRC-DB. add it")
            self.add_signal(pulse, signal, signal_hash)
        elif is_exist == -2:
            logger.info("irsend returns no response; LIRC-DB may be broken.")
            return jsonify(result='failed', message="irsend returns no response; LIRC-DB may be broken.")
        elif is_exist == -3:
            logger.info("lircd may not run.")
            return jsonify(result='failed', message="lircd may not run.")
        elif is_exist == -4:
            return jsonify(result='failed', message="irsend returns unexpected error.")

        # send signal
        # subprocess.Popen(['irsend', 'SEND_ONCE', 'jremocon', signal_hash])

        res_soap  = '<ns:jRemoconResponse xmlns:ns="http://jRemocon">'
        res_soap += '<ns:return>is;ok</ns:return>'
        res_soap += '</ns:jRemoconResponse>'
        return res_soap

    def add_signal(self, pulse, signal, hash):
        pass

    def has_signal(self, hash):
        # lirc_pipe = subprocess.Popen(['irsend', 'LIST', 'jremocon', hash],
        lirc_pipe = subprocess.Popen(['echo', hash],
                                     stdout=subprocess.PIPE,
                                     stderr=subprocess.PIPE)
        out, err = lirc_pipe.communicate()
        lirc_out = (out + err).decode('utf-8')

        if lirc_pipe.returncode == 0:
            return 0
        if 'irsend: unknown command' in lirc_out:
            return -1
        if lirc_out == "":
            return -2 # irsend returns no response; lirc-DB may be broken.
        if 'irsend: could not connect to socket' in lirc_out:
            return -3 # error: lircd may not run.
        # unexpected error
        logger.fatal('irsend returns unexpected error: ' + lirc_out)
        return -4


    def clear_DB(self):
        logger.info('run clear_DB')
        return jsonify(res='clear')


    def restart_lirc(self):
        logger.info('run restart_lirc')
        return jsonify(res='restart')


# REST interface ---------------------------------------------------------------

app = jRemocon()
server = Flask(__name__)


@server.route('/jRemocon/', methods=['GET'])
@server.route('/jRemocon/help', methods=['GET'])
def api_help():
    return app.show_help()


@server.route('/jRemocon/send', methods=['GET'])
def api_send():
    logger.info('get request: send')
    args = request.args

    # param check: pulse
    if 'pulse' in args.keys():
        pulse = args.get('pulse')
    else:
        return jsonify(result='error', info='parameter pulse is not found'), 400
    if pulse.isdigit():
        pulse = int(pulse)
    else:
        return jsonify(result='error', info='parameter pulse is not number'), 400

    # param check: signal
    if 'signal' in args.keys():
        signal = args.get('signal')
    else:
        return jsonify(result='error', info='parameter signal is not found'), 400

    return app.send_signal(pulse, signal)


@server.route('/jRemocon/lirc/clear', methods=['GET'])
def api_clear():
    return app.clear_DB()


@server.route('/jRemocon/lirc/restart', methods=['GET'])
def api_restart():
    return app.restart_lirc()


# entry point ------------------------------------------------------------------
logger = getLogger(__name__)
handler = StreamHandler()
handler.setLevel(DEBUG)
logger.setLevel(DEBUG)
logger.addHandler(handler)


if __name__ == "__main__":
    server.run(port=8080, host='0.0.0.0', debug=True)

