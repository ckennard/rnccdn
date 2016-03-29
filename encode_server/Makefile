CC		= cc
ENCODE_X	= encode
ENCODE		= encode.c

INTERFACES	= gf.c mt19937-64.c
SRCS		= $(ENCODE) $(INTERFACES)
OBJS		= $(SRCS:.c=.o)

LIBS		= 
LIBPATH		= 
INCPATH		= 
CFLAGS		= -Wall -O2 -g

##################################################################

.c.o:
	$(CC) -c $(CFLAGS) $< -o $@

all: $(ENCODE_X)

$(ENCODE_X) : $(OBJS)
	$(CC) -o $@ $(OBJS) $(LIBPATH) $(LIBS)

clean:
	rm -f *.o *.core $(ENCODE_X) $(LIBRARAY)

depend:
	mkdep $(CFLAGS) $(SRCS)