#!/bin/sh
export LD_LIBRARY_PATH="$LD_LIBRARY_PATH:/home/yiyang/work/lib:/home/yiyang/zzy/redis/lib"
rm out.log
ulimit -c unlimited
./mg_xdr -F --loglevel debug --logsize 500 --logfile out.log --cfgfile mg.cfg
