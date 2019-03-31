LIB=libsock.so

CFLAGS   = -I./ -fPIC -shared -rdynamic -std=gnu99
LIBFLAGS = 

OBJS = sock.o

all: $(OBJS)
	$(CC) $(CFLAGS) $(OBJS) -o $(LIB) $(LIBFLAGS)

clean:
	rm -rf *.o *.so

