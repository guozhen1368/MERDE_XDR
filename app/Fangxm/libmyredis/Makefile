CC	= g++
CFLAGS	= -Wall -fPIC
CFLAGS	+= -g
CFLAGS	+= -O2
LNKFLAGS= -Wl,-rpath,. -L.

all: libmyredis.so test

libmyredis.so: myredis_async.h myredis_async.cpp hiredis/libhiredis.a
	$(CC) $(CFLAGS) -fPIC -shared -o $@ $^ -lpthread

test: test.c
	$(CC) $(CFLAGS) $(LNKFLAGS) -o $@ $^ -lmyredis

clean:
	rm -f *.o libmyredis.so test
