finc	= ../finc
bindir	= ../bin
CC	= g++
CPPFLAGS= -Wall -march=native -I$(finc)
CPPFLAGS+= -O2
CPPFLAGS+= -g
LDFLAGS	= -lpthread \
	-Wl,-rpath,./lib -L$(bindir)/lib -lmyredis

finc_obj= $(addprefix $(finc)/, set_corefile.o getcfg_v2.o strtrim.o \
	syslog.o getrname.o tcplsn.o set_keepalive.o recvn.o tcpconn.o)

obj	= main.o xdr_recv.o xdr_pkt.o xdr_proc.o xdr_proc_u.o xdr_proc_s.o \
	xdr_sendto_merge.o xdr_sendto_app.o

$(bindir)/xdr_handler: $(finc_obj) $(obj)
	@mkdir -p $(bindir)
	$(CC) $(CPPFLAGS) $(LDFLAGS) -o $@ $^

main.o: main.c xdr_handler.h

clean:
	rm -f $(finc_obj) $(obj) $(bindir)/xdr_handler tags
