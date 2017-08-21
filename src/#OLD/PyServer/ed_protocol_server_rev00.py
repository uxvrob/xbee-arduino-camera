#! /usr/bin/env python

"""
 @file ed_protocol_server.py
 @desc This script creates a background threaded process for reading serial data
 
"""

#%%
from __future__ import print_function

import sys
sys.path.insert(0, '..')

import logging
import serial
import serial.threaded
from serial import SerialException
import threading

serPort = 'COM3'

#%%

try:
    import queue
except ImportError:
    import Queue as queue


class EDException(Exception):
    pass


class EDProtocol(serial.threaded.LineReader):

    TERMINATOR = b'\r\n'

    def __init__(self):
        super(EDProtocol, self).__init__()
        self.alive = True
        self.responses = queue.Queue()
        self.events = queue.Queue()
        self._event_thread = threading.Thread(target=self._run_event)
        self._event_thread.daemon = True
        self._event_thread.name = 'ed-event'
        self._event_thread.start()
        self.lock = threading.Lock()

    def stop(self):
        """
        Stop the event processing thread, abort pending commands, if any.
        """
        self.alive = False
        self.events.put(None)
        self.responses.put('<exit>')

    def _run_event(self):
        """
        Process events in a separate thread so that input thread is not
        blocked.
        """
        while self.alive:
            try:
                self.handle_event(self.events.get())
            except:
                logging.exception('_run_event')

    def handle_line(self, line):
        """
        Handle input from serial port, check for events.
        """
        if line.startswith('ED+'):
            self.events.put(line)
        else:
            self.responses.put(line)

    def handle_event(self, event):
        """
        Spontaneous message received.
        """
        print('event received:', event)

    def command(self, command, response='OK', timeout=5):
        """
        Send an ED command and wait for the response.
        """
        with self.lock:  # ensure that just one thread is sending commands at once
            self.write_line(command)
            lines = []
            while True:
                try:
                    line = self.responses.get(timeout=timeout)
                    #~ print("%s -> %r" % (command, line))
                    if line == response:
                        return lines
                    else:
                        lines.append(line)
                except queue.Empty:
                    raise EDException('ED command timeout ({!r})'.format(command))


# test
if __name__ == '__main__':
    import time

    class Node(EDProtocol):
        """
        Example communication with PAN1322 BT module.

        Some commands do not respond with OK but with a '+...' line. This is
        implemented via command_with_event_response and handle_event, because
        '+...' lines are also used for real events.
        """

        def __init__(self):
            super(Node, self).__init__()
            self.event_responses = queue.Queue()
            self._awaiting_response_for = None

        def connection_made(self, transport):
            super(Node, self).connection_made(transport)
            # our adapter enables the module with RTS=low
            self.transport.serial.rts = False
            time.sleep(0.3)
            self.transport.serial.reset_input_buffer()

        def handle_event(self, event):
            
            """Handle events and command responses starting with '+...'"""
            """and self._awaiting_response_for.startswith('AT+JRBD'):"""
            if event.startswith('+ED+CTRANS'): 
                print ("Snapshot event received")
                
                img_size = event[10:10 + 3]
                
                print ("Image size: {0}".format(img_size))
                
                """
                img_file = open('image000.jpg', 'w')
                img_size = s[1]
                
                if self.transport.serial.readline() === '>>>':
                    try:
                        image_file.write(self.transport.serial.read(img_size))
                    except SerialException as e:
                        print "Exception: Image read/write error {0}".format(e)
                img_file.close()
                """
                
            else:
                logging.warning('unhandled event: {!r}'.format(event))
        

        def command_with_event_response(self, command):
            """Send a command that responds with '+...' line"""
            with self.lock:  # ensure that just one thread is sending commands at once
                self._awaiting_response_for = command
                self.transport.write(b'{}\r\n'.format(command.encode(self.ENCODING, self.UNICODE_HANDLING)))
                response = self.event_responses.get()
                self._awaiting_response_for = None
                return response

        def getSnapshot(self):
            return self.command_with_event_response("ED+CGETS")

    
    ser = serial.Serial(serPort, baudrate=115200, timeout=1)
    with serial.threaded.ReaderThread(ser, Node) as ed_module:
        print ("Listening for messages...")
        #print("Getting snapshot", ed_module.getSnapshot())