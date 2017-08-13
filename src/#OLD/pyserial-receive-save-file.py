# -*- coding: utf-8 -*-
"""
Spyder Editor

This is a temporary script file.
"""

import serial

buf = ['0','0','0']


with serial.Serial('COM6',115200,timeout=10) as s:
    x=s.readline(3)
    if s == "%+%":
        fo = open("img.jpg", "wb")
        buf.insert(0,s.read())
        buf.insert(1,s.read())
        buf.insert(2,s.read())
        timeout = millis()
        while(timeout < 10000):

            if buf == ['%','-','0']:
                break
            else:
                fo.write(buf.pop(0))
                