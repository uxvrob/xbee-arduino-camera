# -*- coding: utf-8 -*-
"""
Created on Wed Aug 16 18:55:44 2017

@author: Robbie
"""


from __future__ import print_function

import sys
sys.path.insert(0, '..')

import logging
import serial
import serial.threaded
import threading

try:
    import queue
except ImportError:
    import Queue as queue


class AVException(Exception):
    pass

class AVFrameReader (serial.threaded.FramedPacket):
    START = b'AV+['
    STOP = b']+AV'
    
    def __init__(self):
        super(AVFrameReader, self).__init__()
        #self.ENCODING = 'ASCII'
        self.timeout=5
        self.alive = True
        self.responses = queue.Queue()
        self.events = queue.Queue()
        self._event_thread = threading.Thread(target=self._run_event)
        self._event_thread.daemon = True
        self._event_thread.name = 'av-event'
        self._event_thread.start()
        self.lock = threading.Lock()
        self.trans_lock = False

class AVLineReader(serial.threaded.LineReader):
    
    TERMINATOR = b'\r\n'
    
    def __init__(self):
        super(AVLineReader, self).__init__()
        #self.ENCODING = 'ASCII'
        self.timeout=5
        self.alive = True
        self.responses = queue.Queue()
        self.events = queue.Queue()
        self._event_thread = threading.Thread(target=self._run_event)
        self._event_thread.daemon = True
        self._event_thread.name = 'av-event'
        self._event_thread.start()
        self.lock = threading.Lock()
        self.trans_lock = False
    
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
        #print('Line recevied: %r' % (line))
        
        """
        if self.trans_lock == False:
            if line.startswith('+'):
                self.events.put(line)
            else:
                self.responses.put(line)
        else:
            self.responses.put(line.encode())
            
        """
        
        if line.startswith('+AV'):
            self.events.put(line)
        elif line.startswith('AV+'):
            self.events.put(line)
        else:
            self.responses.put(line)

    def handle_event(self, event):
        """
        Spontaneous message received.
        """
        print('event received:', event)
    
    
    def command(self, command, response='OK'):
        """
        Set an AT command and wait for the response.
        """
        with self.lock:  # ensure that just one thread is sending commands at once
            self.write_line(command)
            #print("Command sent: {}".format(command))            
            lines = []
            while True:
                try:
                    line = self.responses.get(timeout=self.timeout)
                    #print("%s -> %r" % (command, line))
                    if line == response:
                        return lines
                    else:
                        lines.append(line)
                except queue.Empty:
                    raise AVException('AV command timeout ({!r})'.format(command))
                    


                    
if __name__ == '__main__':
    import time

    class PAN1322(AVLineReader):
        """
        Example communication with PAN1322 BT module.

        Some commands do not respond with OK but with a '+...' line. This is
        implemented via command_with_event_response and handle_event, because
        '+...' lines are also used for real events.
        """

        def __init__(self):
            super(PAN1322, self).__init__()
            self.event_responses = queue.Queue()
            self.img_queue = queue.Queue()
            self._awaiting_response_for = None
            self.timeout=20

        def connection_made(self, transport):
            super(PAN1322, self).connection_made(transport)
            # our adapter enables the module with RTS=low
            self.transport.serial.rts = False
            time.sleep(0.3)
            self.transport.serial.reset_input_buffer()

        def handle_event(self, event):
            """Handle events and command responses starting with '+...'"""
            #print("event: %s" % (event))
            if event.startswith('+RRBDRES') and self._awaiting_response_for.startswith('AV+JRBD'):
                
                rev = event[9:9 + 12]
                mac = ':'.join('{:02X}'.format(ord(x)) for x in rev[::-1])
                self.event_responses.put(mac)
                
            elif event.startswith('+AV+CTRANS') and self._awaiting_response_for.startswith('AV+CGETS'):
                img_size = int(event[11:])
                
                expected_lines = int(img_size/64);
                
                print ("Expected Lines: {}".format(expected_lines))
                
                # Open image file 
                filename = 'IMAGE01.jpg'
    
                # Increment Image file name by 1
                for i in range(0,100):
                    d = list(filename)
                            
                    d[5] = str(int(i/10))
                    d[6] = str(int(i%10))
                    
                    #serialLog.insert(0.0,"d[5]: {0}, d[6]: {1}".format(d[5],d[6]))
                    filename = ''.join(d)
                                      
                    # Check if file exists, if not break loop and         
                    try:
                        #serialLog.insert(0.0, "Trying to open file: {0}\n".format(filename))
                        f = open(filename)
                        f.close()
                    except IOError:
                        #serialLog.insert(0.0, "IO Error on: {0}\n".format(filename))
                        break;
                counter=0
                d_img_size = img_size
                with open(filename,'wb') as f:
                    self.trans_lock = True
                    while d_img_size > 0:
                        try:
                            line = self.responses.get(timeout=self.timeout)
                            s = line
                            #print("%s -> %r" % (command, line))
                            if d_img_size == 0:
                                f.flush()
                                f.close()
                                break
                            
                            #if line.startswith('AV++'):
                                #s = ''.join(e.strip('\r\n') for e in line[4:])
                            s = line
                            
                            if counter == 100:
                                counter = 0
                                #print ('line:{} line[3:]:{} line[4:]:{} '.format(line,line[3:],line[4:]))
                                print ('s_len: {} f_len: {} img_size: {} d_img_size: {} expected_lines: {}'.format(len(s),f.tell(), img_size, d_img_size, expected_lines))
                            #f.write(''.join(format(x,'b') for x in bytearray(s.encode())))
                            f.write(s.encode())
                            counter = counter + 1
                            d_img_size = d_img_size - len(s)
                            expected_lines = expected_lines-1
                        except queue.Empty:
                            f.close()
                            #return ('Recv size: {} File size: {}'.format(img_size,f.tell()))
                        except:
                            f.close()
                            logging.critical('Unexpected error - Event: {!r} Error: {}'.format(event[:5],sys.exc_info()[0]))
                            #return
                    self.trans_lock=False
                    self.event_responses.put('Recv size: {} File size: {}'.format(img_size,f.tell()))
                    f.flush()
                    f.close()
                
            elif event.startswith('+AV+CTRANF'):
                self.event_responses.put("TRANS_COMP")
            else:
                logging.warning('unhandled event: {!r}, awaiting_response: {}'.format(event[:5],self._awaiting_response_for))

        def command_with_event_response(self, command):
            """Send a command that responds with '+...' line"""
            with self.lock:  # ensure that just one thread is sending commands at once
                self._awaiting_response_for = command
                #self.transport.write(command)
                self.transport.write(('%s\r\n' % command).encode(self.ENCODING, self.UNICODE_HANDLING))
                response = self.event_responses.get(timeout=self.timeout)
                self._awaiting_response_for = None
                return response
        

        # - - - example commands

        def reset(self):
            self.command("AV+JRES", response='ROK')      # SW-Reset BT module

        def get_mac_address(self):
            # requests hardware / calibration info as event
            return self.command_with_event_response("AV+JRBD")
        
        def getSnapshot(self):
            return self.command_with_event_response("AV+CGETS")
        
    
    timeout = 20
    #ser = serial.serial_for_url('spy://COM3', baudrate=115200, timeout=timeout)
    ser = serial.Serial('COM3', baudrate=115200, timeout=timeout)
    time.sleep(2)
    with serial.threaded.ReaderThread(ser, PAN1322) as bt_module:
        bt_module.timeout=timeout
        #bt_module.reset()
        #bt_module.getSnapshot()
        print ("Snapshot received: {}".format(bt_module.getSnapshot()))