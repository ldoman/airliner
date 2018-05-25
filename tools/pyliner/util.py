import json
import socket
import sys
import threading
import time
from datetime import datetime

import socketserver
from flufl.enum import Enum


class LogLevel(Enum):
    Debug = 1
    Info = 2
    Warn = 3
    Error = 4


class PeriodicExecutor(threading.Thread):
    """Executes a callback function every x-seconds.

    Optionally takes callbacks for exception and last-pass handling.
    """

    def __init__(self, callback, every=1, exception=None, finalize=None):
        """
        Args:
            callback (_abcoll.Callable): This method will be called every loop
                with no arguments.
            every (Real): Number of seconds to sleep between calls.
            exception (_abcoll.Callable): If an exception is raised this will
                be called with the exception as an argument.
            finalize (_abcoll.Callable): This method will be called sometime
                after the thread is stopped with no arguments.
        """
        super(PeriodicExecutor, self).__init__()
        self.daemon = True

        self.callback = callback
        # TODO if move to Python 3, use a default print handler.
        self.exception = exception
        self.every = every
        self.finalize = finalize
        self.running = False

    def run(self):
        """Do not call this directly. Use PeriodicExecutor.start()"""
        self.running = True
        try:
            while self.running:
                self.callback()
                time.sleep(self.every)
        except Exception as e:
            if callable(self.exception):
                self.exception(e)
            else:
                print('Exception in thread {}'.format(self.callback))
        finally:
            if callable(self.finalize):
                self.finalize()

    def stop(self):
        """Stops the thread from continuing to loop.

        Thread will not stop immediately. When the thread wakes up next it will
        see that it has been stopped, will execute the finalize method if it was
        given, and will then complete.
        """
        if self.running:
            self.running = False
        else:
            raise RuntimeError('This thread cannot be stopped now.')


class ThreadedUDPRequestHandler(socketserver.BaseRequestHandler):
    def __init__(self, callback, *args, **keys):
        self.callback = callback
        socketserver.BaseRequestHandler.__init__(self, *args, **keys)

    def handle(self):
        self.callback(self.request)


def feet(feet):
    """Convert from given feet to meters."""
    return feet * 0.3048


def get_time():
    return int(
        (datetime.utcnow() - datetime(1970, 1, 1)).total_seconds() * 10000)


def handler_factory(callback):
    """ Creates server object and sets the callback """
    return lambda *args, **kwargs: ThreadedUDPRequestHandler(callback, *args,
                                                             **kwargs)


def indent(level, iterable):
    for string in iterable:
        yield ' ' * level + str(string)


def init_socket():
    """ Creates a UDP socket object and returns it """
    return socket.socket(socket.AF_INET, socket.SOCK_DGRAM)


def query_yes_no(question, default=None):
    """Ask a yes/no question via raw_input() and return their answer.

    "question" is a string that is presented to the user.
    "default" is the presumed answer if the user just hits <Enter>.

    The "answer" return value is one of "yes" or "no".
    Adapted from: http://code.activestate.com/recipes/577058/
    """
    valid = {True: {'y', 'ye', 'yes'},
             False: {'n', 'no'},
             'takeover': {'takeover'}}
    if default is None:
        prompt = " [y/n] "
    elif default:
        prompt = " [Y/n] "
    else:
        prompt = " [y/N] "

    while True:
        sys.stdout.write(question + prompt)
        choice = raw_input().lower()
        if default is not None and choice == '':
            return default
        for answer, options in valid.items():
            if choice in options:
                return answer
        else:
            sys.stdout.write("Please respond with 'yes' or 'no' "
                             "(or 'y' or 'n').\n")


def read_json(file_path):
    """Parses the required JSON input file containing Airliner mappings.

    Returns:
        dict
    """
    try:
        with open(file_path, 'r') as airliner_map:
            return json.load(airliner_map)
    except IOError:
        print("Specified input file (%s) does not exist" % file_path)
    except Exception as e:
        print(e)


def serialize(header, payload):
    """
    Receive a CCSDS message and payload then returns the
    serialized concatenation of them.
    """
    ser = header.get_encoded()
    if payload:
        ser += payload.SerializeToString()
    return ser