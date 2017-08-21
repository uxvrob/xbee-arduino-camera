#%%
# -*- coding: utf-8 -*-
"""
Created on Wed Aug 16 18:55:44 2017

@author: Robbie
"""


from __future__ import print_function

import os
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

class AVFrameReader (serial.threaded.LineReader):
    
    TERMINATOR = b'<<<'
    
    ENCODING = 'ascii'
    UNICODE_HANDLING = 'ignore'
    
    def __init__(self):
        super(AVFrameReader, self).__init__()
        #self.ENCODING = 'ASCII'
        self.timeout=50
        self.alive = True
        self.responses = queue.Queue()
        self.events = queue.Queue()
        self._event_thread = threading.Thread(target=self._run_event)
        self._event_thread.daemon = True
        self._event_thread.name = 'av-event'
        self._event_thread.start()
        self.lock = threading.Lock()
        self.trans_lock = False
        self.packet_event_length = 0
        self.packet_response_length = 0
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
                
    def handle_event(self, event):
        """
        Spontaneous message received.
        """
        print('event received:', event)
        
    def handle_line(self,line):
        
        if(self.trans_lock):
            if(len(packet) == 3):
                if(packet == b'+++'):
                    self.trans_lock = False
                    self.events.put('+AV+CTRANF')
                    self.packet_event_length = self.packet_event_length + len(packet)
                    print ("+++ received p: {} pack event length: {}".format(packet, self.packet_event_length))
                    
            else:
                self.responses.put(packet)
                self.packet_response_length = self.packet_response_length + len(packet)
                if(self.packet_response_length < 100 or self.packet_response_length > 43000):
                    print ("FALSE translock packet: {} pack response length: {}".format(packet, self.packet_response_length))
                
        else:
            try:
                s = packet.decode()
                print("Decoded Packet: {}".format(s))
                if(s.startswith("+AV+CTRANS")):
                    self.trans_lock = True
                if(s.startswith("+AV")):
                    self.events.put(packet.decode())
                    self.packet_event_length = self.packet_event_length + len(packet)
                    """
                    if(self.packet_event_length < 100):
                        print ("+AV. Translock status: {} p: {} pack event length: {}".format(self.trans_lock, packet, self.packet_event_length))
                    """
            except:
                logging.critical("Exception Packet: {}".format(packet))
                self.events.put(packet)
        self.events.task_done()
        self.events.task_done()
        
    def write_line(self, text):
        """
        Write text to the transport. ``text`` is a Unicode string and the encoding
        is applied before sending ans also the newline is append.
        """
        # + is not the best choice but bytes does not support % or .format in py3 and we want a single write call
        self.transport.write(text.encode('utf-8', 'replace') + b'\r\n')
    
    
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

    class PAN1322(AVFrameReader):
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
            self.timeout=100

        def connection_made(self, transport):
            super(PAN1322, self).connection_made(transport)
            # our adapter enables the module with RTS=low
            self.transport.serial.rts = False
            time.sleep(0.3)
            self.transport.serial.reset_input_buffer()

        def handle_event(self, event):
            """Handle events and command responses starting with '+...'"""
            print("event: %s" % (event))
                
            if event.startswith('+AV+CTRANS') and self._awaiting_response_for.startswith('AV+CGETS'):

                img_size = int(event[11:])                    
                expected_lines = int(img_size/64)+1;
                
                print ("Size: {} Expected Lines: {}".format(img_size, expected_lines))
                
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
                
                d_img_size = img_size
                
                
                with open(filename,'wb') as f:
                    exCtr = 0;
                    while d_img_size > 0:
                        try:
                            s = self.responses.get()
                        except queue.Empty:
                            if(exCtr < 5):
                                time.sleep(0.1)
                                exCtr = exCtr+1
                                continue
                            else:
                                print ("Exception counter.  Breaking out of loop...")
                                exCtr = 0
                                break
                            
                        except:
                            logging.critical('Unexpected error - Event: {!r} Error: {}'.format(event,sys.exc_info()))
                            self.stop()
                            exit()
                        
                            #print("%s -> %r" % (event, s))
                            
                        errCtr = 0 
                        
                        while((len(s) < 64 and d_img_size > 64) and errCtr < 4):
                            if d_img_size > 45000 or d_img_size <= 6000:
                                #print ('p: {}'.format(s))
                                print ('s_len: {} f_len: {} img_size: {} d_img_size: {}, packet_event_length: {}, packet_response_length: {}'.format(len(s),f.tell(), img_size, d_img_size, self.packet_event_length, self.packet_response_length))
    
                            self.write_line('-')
                            print ('LESS THAN 64.. getting data again...')
                            print('s_len:{}\nd_img_size:{}\npacket:{}'.format(len(s), d_img_size,s))
                            time.sleep(0.01)
                            s = self.responses.get()
                            errCtr = errCtr + 1
                                
                        if(len(s) == 64):
                            #if d_img_size > 45000 or d_img_size <= 6000:
                                #print ('p: {}'.format(s))
                            print ('PACKET RECV SUCCESS. s_len: {} f_len: {} img_size: {} d_img_size: {}, packet_event_length: {}, packet_response_length: {}'.format(len(s),f.tell(), img_size, d_img_size, self.packet_event_length, self.packet_response_length))
                            f.write(s)
                            d_img_size = d_img_size - len(s)
                            expected_lines = expected_lines-1
                            self.write_line('+')
                        else:
                            print ("Line: {} \nFUCKER DIDNT GET THE BYTES...".format(expected_lines))
                            raise AVException
                        
                        self.event_responses.put('TOK')
                       
                    f.flush()
                    f.close()
                    self.trans_lock=False
                
                self.event_responses.put('+Recv size: {} d_img_size: {} exp_line: {} File size:'.format(img_size, d_img_size, expected_lines))
                
            elif event.startswith('+AV+CTRANF'):
                self.event_responses.put("TRANS_COMP")
            else:
                logging.warning('unhandled event: {!r}, awaiting_response: {}'.format(event[:5],self._awaiting_response_for))

        def command_with_event_response(self, command):
            """Send a command that responds with '+...' line"""
            with self.lock:  # ensure that just one thread is sending commands at once
                self._awaiting_response_for = command
                self.write_line(command)
                response = 'TOK'
                #self.transport.write(('%s\r\n' % command).encode(self.ENCODING, self.UNICODE_HANDLING))
                while response == 'TOK':
                    try:
                        response = self.event_responses.get(timeout=self.timeout)
                        if (response.startswith("+")):
                            break
                        else: 
                            continue
                    except queue.Empty:
                        break
                    except:
                        logging.critical('command_with_event_reponse: unhandled exception {!r}'.format(sys.exc_info()))
                        self.stop()
                        exit()
                self._awaiting_response_for = None
                return response
        

        # - - - example commands

        def reset(self):
            self.command("AV+JRES", response='ROK')      # SW-Reset BT module
            
        def getSnapshot(self):
            return self.command_with_event_response("AV+CGETS")
        
    
    timeout = 10
    #ser = serial.serial_for_url('spy://COM3', baudrate=115200, timeout=timeout)
    ser = serial.Serial('COM3', baudrate=115200, timeout=timeout)
    time.sleep(2)
    with serial.threaded.ReaderThread(ser, PAN1322) as av_module:
        av_module.timeout=timeout
        print ("Snapshot OK {}".format(av_module.getSnapshot()))