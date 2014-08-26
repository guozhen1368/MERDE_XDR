#! /usr/bin/python
#coding=utf-8

import socket
import ParseData
import readxml
import struct
import time
import ConfigParser


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
    
def initserver():
    xmldict = xmlparse('DataParse.xml')
    print "success parse xml ..."
    
    cfg = ConfigParser.ConfigParser()
    cfg.read('server.ini')
    logname = cfg.get('SETTING', 'log')
    f = open(logname, 'w')
    port = cfg.getint('SETTING', 'PORT')
    addr = ('', port)
    
    tcpsrv = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    tcpsrv.bind(addr)
    tcpsrv.listen(10)
    
    print "waiting for client connect ...."
    
    while True:
        clientsock , c_addr = tcpsrv.accept()
        print 'accept sock'
        while True:
            try:
                #clientsock.settimeout(5)
                msg = clientsock.recv(BUFFSIZE)
                print "msg lenth", len(msg)
                if not msg: break
                else:
                    try:
                        ack,log = ParseData.parse(msg, xmldict)
                        clientsock.send(ack)
                        f.write(log + '\r')
                    except struct.error:
                        print 'parse error'
                        clientsock.close()
                        break
            except socket.timeout:
                clientsock.close()
                print 'socket close'
                break
            except socket.error:
                clientsock.close()
                print 'socket close'
                break
                
                
    tcpsrv.close()
    f.close()
                
                
if __name__ == '__main__':
    initserver()
    time.sleep(1)
                
            
            
    