# -*- coding: utf-8 -*-
"""
Created on Fri Aug 18 11:34:44 2017

@author: Robbie
"""

import serial
import time

timeout = 10

with serial.serial_for_url('spy://COM3', baudrate=115200, timeout=timeout) as ser:
#with serial.Serial('COM3', baudrate=115200, timeout=timeout) as ser:
    
read_byte = ser.read()

while read_byte is not None:
    read_byte = ser.read()
    if
        time.sleep(2)
        ser.write(b'AV+CGETS')
        line = ser.readline()
        print("p: {}".format(line))
        
        img_size = int(line[11:])
        print ("Size: {}".format(img_size))   
#%%            
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
        