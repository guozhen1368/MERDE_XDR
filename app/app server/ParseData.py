#! /usr/bin/env python
#coding=utf-8

import readxml
import struct
import datetime
import sys

configdict = readxml.ConvertXmlToDict('DataParse.xml')

def uc_head(data):
    head_data = []
    if len(data) >= 16:
        ver,signa,msgtype,msgno,resv,msgseq,msglen = struct.unpack('!B3sBHc2I', data[:16])
        head_data.append('msgtype=%d'%msgtype)
        head_data.append('msgno=%d'%msgno)
        head_data.append('msgseq=%d'%msgseq)
        head_data.append('msglen=%d'%msglen)
        log = ' '.join(head_data)
        return msgtype, msgno, msgseq, log


def field(data, d, total):
    lenth = int(d['len'])
    count = int(d['fieldnum'])
    nam = d['name']
    typ = d['type']
    
    if typ == 's':
        s, = struct.unpack('=%ds'%lenth, data[:lenth])
        total.append('%s=%s'%(nam, s))
        #data = data[lenth:]
        return lenth
    
    a = 0
    if lenth > 0:
        a, = struct.unpack('!'+typ, data[:lenth])
        total.append('%s=%d'%(nam, a))
        data = data[lenth:]
    
    if count > 0:
        for i in sorted(d):
            if type(d[i]) == readxml.XmlDictObject:
                if d[i]['type']  == 's':
                    s, = struct.unpack('%ds'%a, data[:a])
                    total.append('%s=%s'%(d[i]['name'], s))
                    #data = data[a:]
                    lenth += a
                    return lenth
                elif d[i]['type'] == 'ip':
                    ip1,ip2,ip3,ip4 = struct.unpack('4B', data[:4])
                    total.append('%s=%d.%d.%d.%d'%(d[i]['name'],ip1,ip2,ip3,ip4))
                    #data = data[4:]
                    lenth += 4
                    return lenth
                    
        for n in range(0, a):
            for i in sorted(d):
                if type(d[i]) == readxml.XmlDictObject:                   
                    l = field(data, d[i], total)
                    data = data[l:]
                    lenth += l
                
    return lenth

def parse(data, dic):
    msgtype, msgno, msgseq, log = uc_head(data)
    data = data[16:]
    
    ack = struct.pack('!B3sBHB2IBHIBH', 1, 'CUC', 0xcd, 1, 0, 2222, 26, \
    msgtype, msgno, msgseq, 200, 0)
    
    
    total_data = []

    if not dic.has_key('msgtype%x'%msgtype):
        print 'not find msgtype=%x'%msgtype
        return
    print 'this msgtype = %x'%msgtype
        
    d = dic['msgtype%x'%msgtype]
    
    name = d['name']
    total_data.append('msgname=%s'%name)
    
    #解析字段
    for s in sorted(d):
        if type(d[s]) == readxml.XmlDictObject:
            data = data[field(data, d[s], total_data):]  
        
    curtime = datetime.datetime.now()
    curTime = curtime.strftime('%Y-%m-%m %H:%M:%S')     #转换格式   
    log = curTime+'  '+log+' ' + ' '.join(total_data)
    print log
    return ack,log
    

if __name__ == '__main__':
    configdict = readxml.ConvertXmlToDict('DataParse.xml')
    data = struct.pack('!B3sBHB2I', 1, '234', 0x01, 6, 7, 8, 9)
    data += struct.pack('!2H2B4sB3s2B3sB2sB5sB3s', 1, 2, 2, 4, 'asdf',3,'kjl', 2, 3, 'qwe', 2, 'qq', 5, 'qwert', 3, 'wer')
    parse(data, configdict['policy'])