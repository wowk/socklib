LIB=libsock.so

CFLAGS   = -I./ -fPIC -shared -rdynamic -std=gnu99
LIBFLAGS = 

OBJS = sock.o

all: $(OBJS)
	$(CC) $(CFLAGS) $(OBJS) -o $(LIB) $(LIBFLAGS)
	$(CC) test.c -o test -L./ -lsock

clean:
	rm -rf *.o *.so test

