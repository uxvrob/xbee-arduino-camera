from Tkinter import *
from PIL import ImageTk, Image
#import logging
import serial
#import threading
import io
import time

class AvSerial:
    
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
            self.line = sio.readline()
            
    def write(self, data):
        if(self.ser.isOpen):
            b = bytes(data,'utf-8')
            self.ser.write(b)
    
    def close(self):
        self.ser.close()


bgColor = "#FFFFFF"

serPort = "COM5"
serBaudRate = 115200

root = Tk() #Makes the window
root.wm_title("AVALON 0.1") #Makes the title that will appear in the top left
root.config(bg = "#828481")

av = AvSerial(serPort, serBaudRate)

def takeSnapshot():
    
    
    # Open image file 
    filename = "IMAGE00.jpg"
    
    for i in range(0,100):
        filename[5] = '0' + i/10
        filename[6] = '0' + i%10
        # Check if file exists, if not break loop and         
        try:
            f = open(filename)
            f.close()
        except IOError:
            break;
    
    with open(filename,'wb') as f:
        
        # Timeout of receiving image
        timeout = 5000 # ms 
        
        imgSize = 0

        av.write("ED+CGETS")
        
        startTime = int(round(time.time() * 1000))
        
        while((int(round(time.time() * 1000))-startTime)<timeout):
            av.readLine()
            print av.line
            
            # start of image transfer
            
            if av.line.startswith("+ED+CTRANS,"):
                s = av.line.split(',')
                imgSize = int(s[1])
                print ("Image Size: {0}".format(imgSize))
            elif av.line.startswith(">>>"):
                print "Start image transfer..."
                serialLog.insert(0.0, "Start image transfer...\n")
            elif av.line.startswith("<<<"):
                print "Image transfer complete!"
                serialLog.insert(0.0, "Image transfer complete...\n")
            else:
                f.write(av.line)
            f.flush()
    av.imageEx = ImageTk.PhotoImage(Image.open(filename))
    
    
def startLog():
    serialLog.insert(0.0, "Start Log CMD called\n")
    #ser.write("EG+STALG")
	
def stopLog():
    serialLog.insert(0.0, "Stop Log CMD called\n")
	#ser.write("EG+STOLG")

def viewImage():
    
    serialLog.insert(0.0, "View Image function called\n")


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