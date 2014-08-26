export LD_LIBRARY_PATH="$LD_LIBRARY_PATH:/home/yiyang/work/lib:/home/yiyang/zzy/redis/lib"



echo "set args -F --loglevel debug --logsize 500 --logfile out.log --cfgfile mg.cfg"
gdb ./mg_xdr