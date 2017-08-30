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

try:
    import queue
except ImportError:
    import Queue as queue


class AVException(Exception):
    pass

class AVSerial:        
    
    def __init__(self, port, baudrate):
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
    
    def read(self, bytes=32):
        if(self.ser.isOpen):
            self.ser.read(32)
            
    def write(self, data):
        if(self.ser.isOpen):
            b = bytes(data,'utf-8')
            self.ser.write(b)
    
    def flush(self):
        if(self.ser.isOpen):
            self.ser.reset_input_buffer()

    
    def close(self):
        self.ser.close()
        

class AVLineReader(serial.threaded.LineReader):
    
    TERMINATOR = b'\r\n'
    
    def __init__(self):
        super(AVLineReader, self).__init__()
        self.alive = True
        self.responses = queue.Queue()
        self.events = queue.Queue()
        self._event_thread = threading.Thread(target=self._run_event)
        self._event_thread.daemon = True
        self._event_thread.name = 'av-event'
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
                
    def connection_made(self,transport):
        super(AVLineReader, self).connection_made.transport
        serialLog.insert(0.0, "Serial port opened!\n")
        
    def handle_line(self, data):
        
        return data

    def write_line(self,line):
        
        self.write_line(line)
        
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
                    raise AVException('ED command timeout ({!r})'.format(command))
        
class AVPacketReader(serial.threaded.Packetizer):
    
    def connection_made(self,transport):
        super(AVPacketReader, self).connection_made.transport
        serialLog.insert(0.0, "Serial port opened!\n")
        
    def handle_packet(self,packet=32):
        

bgColor = "#FFFFFF"

serPort = "COM3"
serBaudRate = 115200

root = Tk() #Makes the window
root.wm_title("AVALON 0.1") #Makes the title that will appear in the top left
root.config(bg = "#828481")
    


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
    
    with open(filename,'wb') as f:
        
        # Timeout of receiving image
        timeout = 2000 # ms 
        
        imgSize = 0
        
        av = AVSerial(serPort, serBaudRate)
        serialLog.insert(0.0, "Opening serial port\n")
        av.open()
        serialLog.insert(0.0, "Flushing serial\n")
        av.flush()
        
        with serial.threaded.ReaderThread(av, AVLineReader) as avlr:
            avlr.write_line('AV+CGETS')
        
        av.readLine()
        # clearing buffer
        while(len(av.line)>0):
            av.readLine()
            serialLog.insert(0.0, "Buffer clear: {0}\n".format(av.line))
        
        serialLog.insert(0.0, "Sending AV+CGETS cmd\n")
        av.write("AV+CGETS")
        av.read(32)
        av.readLine()
        serialLog.insert(0.0, "Line 1: {0}".format(av.line))
        av.readLine()
        serialLog.insert(0.0, "Line 2: {0}".format(av.line))
        #av.readLine()
        #serialLog.insert(0.0, "Line 3: {0}".format(av.line))
        
        """
        startTime = int(round(time.time() * 1000))
        
        
        
        while(((int(round(time.time() * 1000))-startTime)<timeout)):
            av.readLine()
            serialLog.insert(0.0, "Line: {0}".format(av.line))
           
            # start of image transfer
            
            if av.line.startswith("+AV+CTRANS,"):
                s = av.line.split(',')
                imgSize = int(s[1])
                serialLog.insert(0.0, "Image Size: {0}".format(imgSize))
                
                av.readLine()

                if av.line.startswith(">>>"):
                    serialLog.insert(0.0, "Start image transfer...\n")
                    ctrans_flag = True
                elif av.line.startswith("<<<"):
                    serialLog.insert(0.0, "Image transfer complete...\n")
            elif ctrans_flag:
                #serialLog.insert(0.0, ">{0}".format(av.line))
                f.write(av.line)
                
         while(((int(round(time.time() * 1000))-startTime)<timeout)):
            ctrans_flag = False
            f.flush()
         """    
        av.close()
    
    filename='IMAGE00.jpg'
    av.imageEx = ImageTk.PhotoImage(Image.open(filename))

def startLog():
    serialLog.insert(0.0, "Start Log CMD called\n")
    #ser.write("EG+STALG")
	
def stopLog():
    serialLog.insert(0.0, "Stop Log CMD called\n")
    #ser.write("EG+STOLG")

def viewImage():
    serialLog.insert(0.0, "View Image CMD called\n")

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