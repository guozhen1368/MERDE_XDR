#! /usr/bin/python
#coding=utf-8

import socket
import ParseData
import readxml
import struct
import time
import ConfigParser
import random


HOST = ''
PORT = 50001
ADDR = (HOST, PORT)
BUFFSIZE = 65535

def xmlparse(fname):
    configdict = readxml.ConvertXmlToDict(fname)
    #data = struct.pack('!B3sBHB2I', 1, '234', 0x00, 6, 7, 8, 9)
    #data += struct.pack('!2H2B4s2B3sBB5sB', 1, 2, 1, 4, 'asdf', 2, 3, 'qwe', 8, 5, 'qwert', 112)
    #parse(data, configdict['policy'])
    return configdict['policy']

def readxdr(fname):
    xdrs = []
    datas = ''
    f = open(fname, 'r')
    
    print 'open file = %s readxdr' % fname
    while True:
        data =  f.read(2048)
        if len(data) == 0:
            break
        datas += data
        while len(datas) > 2:
            l, = struct.unpack('!H', datas[:2])
            if len(datas) < l:
                break
            xdrs.append(datas[:l])
            datas = datas[l:]
            #del datas[:l]
            
    f.close()    
    return xdrs

def parse(msg):
    mmeid = []
    for m in msg:
        s1ap, group, code = struct.unpack('!IHB', m[67:74])
        ids = [s1ap, group, code]
        mmeid.append(ids)
        
    return mmeid
       
    
def initserver():
    #xmldict = xmlparse('DataParse.xml')
    #print "success parse xml ..."
    
    cfg = ConfigParser.ConfigParser()
    cfg.read('server.ini')
    logname = cfg.get('SETTING', 'log')
    f = open(logname, 'w')
    ip = cfg.get('SETTING', 'SERVER')
    port = cfg.getint('SETTING', 'PORT')
    addr = (ip, port)
    
    uu = cfg.get('SETTING', 'uu')
    x2 = cfg.get('SETTING', 'x2')
    mme = cfg.get('SETTING', 'mme')
    s6a = cfg.get('SETTING', 's6a')
    sgs = cfg.get('SETTING', 'sgs')
    
    msg = []
    msg.append(readxdr(uu))
    msg.append(readxdr(x2))
    msg.append(readxdr(mme))
    msg.append(readxdr(s6a))
    msg.append(readxdr(sgs))
    
    mmeid = parse(msg[2])
    
    print addr
    tcpclient = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    tcpclient.connect(addr)
    
    print "waiting for client connect ...."
    
    readmsg(mmeid, msg, tcpclient)
    #readmsg(mmeid, msg, f)

    time.sleep(10)
    tcpclient.close()
    f.close()

def readmsg(mmeid, msg,  f):
    count = 0
    while True:
        i = random.randint(0, 4)
        n = random.randint(0, len(mmeid)-1)
        m = mmeid[n]
        if len(msg[i]) == 0:
            break

    #for i in range(0, 5):
        #m = mmeid[0]
        item = msg[i].pop()
        
        t = time.time()
        lenth = len(item) + 20
        p = lenth % 4
        l = lenth / 4
        sc = 0
        if p > 0:
            sc = 4 - p
            l += 1

        #item = struct.pack('!22sQ25sdd', item[22], 1, item[:25], t, t) + item[71:]
            
        if i==0:
            item = struct.pack('!86sIHB', item[:86], m[0], m[1], m[2]) + item[93:]

            item = struct.pack('!2BH4B2H2I55sdd', 0x7E, 0x5A, l, 0x09, 0, 0, sc, 0, 0, 0, 0, \
            item[:55], t, t) + item[71:]    
        elif i==1:
            item = struct.pack('!78sIHB', item[:78], m[0], m[1], m[2]) + item[85:]

            item = struct.pack('!2BH4B2H2I55sdd', 0x7E, 0x5A, l, 0x09, 0, 0, sc, 0, 0, 0, 0, \
            item[:55], t, t) + item[71:]
        else:
            item = struct.pack('!2BH4B2H2I22sQ25sdd', 0x7E, 0x5A, l, 0x09, 0, 0, sc, 0, 0, 0, 0, \
            item[:22], 1, item[:25], t, t) + item[71:]

        lenthss,city,inf, = struct.unpack('!HHB', item[20:25])
        lenthaa = len(item) - 20
        print lenthss, city, inf, lenthaa
        #print ' '.join(item[20:])

        count += 1
        f.sendall(item)
        #f.write(item)
    print count
            
                
if __name__ == '__main__':
    initserver()
    time.sleep(1)
                
            
            