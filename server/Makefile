client	= cc
SERVER_X	= server
SERVER		= server.c

SRCS		= $(SERVER)
OBJS		= $(SRCS:.c=.o)

LIBS		=
LIBPATH		=
INCPATH		=
CFLAGS		= -Wall -O0 -g

.c.o:
	$(CC) -c $(CFLAGS) $< -o $@
	
all: $(SERVER_X)

$(SERVER_X): $(OBJS)
	$(CC) -o $@ $(OBJS) $(LIBPATH) $(LIBS)

clean:
	rm -f *.o *.core $(SERVER_X)
