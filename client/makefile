client	= cc
CLIENT_X	= client
CLIENT		= client.c

SRCS		= $(CLIENT)
OBJS		= $(SRCS:.c=.o)

LIBS		=
LIBPATH		=
INCPATH		=
CFLAGS		= -Wall -O2 -g

.c.o:
	$(CC) -c $(CFLAGS) $< -o $@
	
all: $(CLIENT_X)

$(CLIENT_X): $(OBJS)
	$(CC) -o $@ $(OBJS) $(LIBPATH) $(LIBS)

clean:
	rm -f *.o *.core $(CLIENT_X)