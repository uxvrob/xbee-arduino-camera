#%%
from tkinter import *
#%%
from PIL import ImageTk, Image
#import logging
import serial
import serial.threaded
#import threading
import io
import time
#%%

try:
    import queue
except ImportError:
    import Queue as queue


class AVException(Exception):
    pass
#%%

class AvProtocol(serial.threaded.LineReader):

    TERMINATOR = b'\r\n'
    
    def __init__(self, port, baudrate):
        super(AvProtocol, self).__init__()
        self.alive = True
        self.responses = queue.Queue()
        self.events = queue.Queue()
        self._event_thread = threading.Thread(target=self._run_event)
        self._event_thread.daemon = True
        self._event_thread.name = 'av-event'
        self._event_thread.start()
        self.lock = threading.Lock()
 """
        self.port = port
        self.baudrate = baudrate
        self.line = ""
        self.imageEx = ImageTk.PhotoImage(Image.open("IMAGE00.jpg"))
        
        
    def open(self):
        self.ser = serial.Serial(port=self.port, baudrate = self.baudrate, timeout=2)
        self.sio = io.TextIOWrapper(io.BufferedRWPair(self.ser, self.ser))
        
    def readLine(self):
        if(self.ser.isOpen):
            self.line = self.sio.readline()
            
    def write(self, data):
        if(self.ser.isOpen):
            b = bytes(data,'utf-8')
            self.ser.write(b)
    
    def flush(self):
        if(self.ser.isOpen):
            self.ser.flush()

    
    def close(self):
        self.ser.close()
        
        def stop(self):
        """
        #Stop the event processing thread, abort pending commands, if any.
        """
        self.alive = False
        self.events.put(None)
        self.responses.put('<exit>')
     """ 
     
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
        if line.startswith('AV+'):
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

#%%

if __name__ == '__main__':
    import time

    bgColor = "#FFFFFF"
    
    serPort = "COM3"
    serBaudRate = 115200
    
    root = Tk() #Makes the window
    root.wm_title("AVALON 0.1") #Makes the title that will appear in the top left
    root.config(bg = "#828481")
    
    #av = AvSerial(serPort, serBaudRate)
    
    class ServerNode(AvProtocol):
        def __init__(self):
            super(ServerNode, self).__init__()
            self.event_responses = queue.Queue()
            self._awaiting_response_for = None
            self._flag_ctrans = False

        def connection_made(self, transport):
            super(ServerNode, self).connection_made(transport)
            # our adapter enables the module with RTS=low
            self.transport.serial.rts = False
            time.sleep(0.3)
            self.transport.serial.reset_input_buffer()

        def handle_event(self, event):
            
            """Handle events and command responses starting with '+...'"""
            """and self._awaiting_response_for.startswith('AT+JRBD'):"""
            if event.startswith('+AV+CTRANS'):                     
                    #print ("Snapshot event received")
                    img_size = event[10:10 + 3]
                    #print ("Image size: {0}".format(img_size))
                    
                    response = self.event_responses.get()
                    if(response.startswith(">>>")):
                        startTime = int(round(time.time() * 1000))
                        
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
                
                        with open(filename,'wb') as f:
                            
                            while(((int(round(time.time() * 1000))-startTime)<timeout)):
                                response = self.event_responses.get()
                                if(response.startswith("<<<")):
                                    break;
                                f.write(response)
                                
                            f.flush
                                
                            
                            
                            
                            #serialLog.insert(0.0, "Line: {0}".format(response))
        
                        if av.line.startswith(">>>"):
                            print ("Start image transfer...")
                            serialLog.insert(0.0, "Start image transfer...\n")
                        elif av.line.startswith("<<<"):
                        print ("Image transfer complete!\n")
                        serialLog.insert(0.0, "Image transfer complete...\n")
                    else:
                        serialLog.insert(0.0, ">{0}".format(av.line))
                        #f.write(av.line)
                    
                    
                
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
        
        """
        def takeSnapshot():
        
            serialLog.insert(0.0, "Take Snapshot cmd called!\n")
            
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
        
            serialLog.insert(0.0, "File to write: {0}\n".format(filename))
            
            return self.command_with_event_response("AV+CGETS")
            
            
            with open(filename,'wb') as f:
                
                # Timeout of receiving image
                timeout = 5000 # ms 
                
                imgSize = 0
                
                serialLog.insert(0.0, "Opening serial port\n")
                av.open()
                serialLog.insert(0.0, "Flushing serial\n")
                av.flush()
                
                   
                # clearing buffer
                while(len(av.line)>0):
                    av.readLine()
                    serialLog.insert(0.0, "Buffer clear: {0}\n".format(av.line))
                
                av.write("AV+CGETS")
                
                startTime = int(round(time.time() * 1000))
                
                while(((int(round(time.time() * 1000))-startTime)<timeout) or av.line.startswith("<<<")):
                    av.readLine()
                    serialLog.insert(0.0, "Line: {0}".format(av.line))
                    
                    # start of image transfer
                    
                    if av.line.startswith("+AV+CTRANS,"):
                        s = av.line.split(',')
                        imgSize = int(s[1])
                        serialLog.insert(0.0, "Image Size: {0}".format(imgSize))
        
                        if av.line.startswith(">>>"):
                            print ("Start image transfer...")
                            serialLog.insert(0.0, "Start image transfer...\n")
                        elif av.line.startswith("<<<"):
                        print ("Image transfer complete!\n")
                        serialLog.insert(0.0, "Image transfer complete...\n")
                    else:
                        serialLog.insert(0.0, ">{0}".format(av.line))
                        #f.write(av.line)
                        
                    f.flush()
                    
                av.close()
            
            filename='IMAGE00.jpg'
            av.imageEx = ImageTk.PhotoImage(Image.open(filename))
            
            """
    
    ser = serial.Serial(serPort, baudrate=115200, timeout=1)
    with serial.threaded.ReaderThread(ser, ServerNode) as ed_module:
        
        def startLog():
            serialLog.insert(0.0, "Start Log CMD called\n")
            #ser.write("EG+STALG")
        	
        def stopLog():
            serialLog.insert(0.0, "Stop Log CMD called\n")
            #ser.write("EG+STOLG")
        
        def viewImage():
            serialLog.insert(0.0, "View Image CMD called\n")
         
        def takeSnapshot():
            serialLog.insert(0.0, "Take Snapshot CMD called\n")
            ed_module.command_with_event_response("AV+CGETS")
            
            
        print ("Listening for messages...")
    
    
        #Left Frame and its contents
        leftFrame = Frame(root, width=200, height = 600, bg=bgColor, highlightthickness=2, highlightbackground="#111")
        leftFrame.grid(row=0, column=0, padx=10, pady=2, sticky=N+S)
        
        Inst = Label(leftFrame, text="Instructions:", anchor=W, bg=bgColor)
        Inst.grid(row=0, column=0, padx=10, pady=2, sticky=W)
        
        instructions = "Take snapshot: Take a picture from node and receive it here.  "
        Instruct = Label(leftFrame, width=22, height=10, text=instructions, takefocus=0, wraplength=170, anchor=W, justify=LEFT, bg=bgColor)
        Instruct.grid(row=1, column=0, padx=10, pady=2)
        
        
        Label(leftFrame, image=av.imageEx).grid(row=2, column=0, padx=10, pady=2)
        
        #Right Frame and its contents
        rightFrame = Frame(root, width=200, height = 600, bg=bgColor, highlightthickness=2, highlightbackground="#111")
        rightFrame.grid(row=0, column=1, padx=10, pady=2, sticky=N+S)
        
        btnLbl = Label(rightFrame, text="Commands:", anchor=W, bg=bgColor)
        btnLbl.grid(row=1, column=0, padx=10, pady=2, sticky=W)
        btnFrame = Frame(rightFrame, width=200, height = 200, bg=bgColor)
        btnFrame.grid(row=2, column=0, padx=10, pady=2)
        
        serLbl = Label(rightFrame, text="Serial Log:", anchor=W, bg=bgColor)
        serLbl.grid(row=3, column=0, padx=10, pady=2, sticky=W)
        
        serialLog = Text(rightFrame, width = 30, height = 10, takefocus=0, highlightthickness=1, highlightbackground="#333")
        serialLog.grid(row=4, column=0, padx=10, pady=2)
        
        takeSnapshot= Button(btnFrame, text="Take Snapshot", command=takeSnapshot, bg="#EC6E6E")
        takeSnapshot.grid(row=0, column=0, padx=10, pady=2)
        
        startLogging = Button(btnFrame, text="Start Logging", command=startLog, bg="#ECE86E")
        startLogging.grid(row=0, column=1, padx=10, pady=2)
        
        stopLogging = Button(btnFrame, text="Stop Logging", command=stopLog, bg="#6EEC77")
        stopLogging.grid(row=0, column=2, padx=10, pady=2)
        
        
        mainloop()