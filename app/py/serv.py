#! /usr/bin/python
#coding=utf-8

import socket
import struct
import time
import ConfigParser


HOST = ''
PORT = 50001
ADDR = (HOST, PORT)
BUFFSIZE = 65535

def initserver():
    
    cfg = ConfigParser.ConfigParser()
    cfg.read('server.ini')
    logname = cfg.get('SETTING', 'log')
    f = open(logname, 'w')
    port = cfg.getint('serv', 'PORT')
    addr = ('', port)
    
    tcpsrv = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    tcpsrv.bind(addr)
    tcpsrv.listen(10)
    
    print "waiting for client connect ...."
    
    while True:
        clientsock , _ = tcpsrv.accept()
        msgs = []
        count = 0
        print 'accept sock'
        while True:
            try:
                #clientsock.settimeout(5)
                msg = clientsock.recv(BUFFSIZE)
                msgs += msg
                if len(msg) == 0: break
                else:
                    while len(msgs) > 2:
                        
                        lenth, = struct.unpack('!H', msgs[:2])
                        print lenth,
                        if len(msgs) >= lenth:
                            parseData(msgs[:lenth])
                            del msgs[:lenth]
                        count += 1
            except socket.timeout:
                clientsock.close()
                print 'socket close'
                break
            except socket.error:
                clientsock.close()
                print 'socket close'
                break
        print count
                
                
    tcpsrv.close()
    f.close()

def parseData(msg):
    msg 
                
                
if __name__ == '__main__':
    initserver()
    time.sleep(1)
                
            
            
    