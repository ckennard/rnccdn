CC		= cc
DECODE_X	= decode
DECODE 		= decode.c

INTERFACES	= gf.c mt19937-64.c
SRCS_2		= $(DECODE) $(INTERFACES)
OBJS_2		= $(SRCS_2:.c=.o)

LIBS		= 
LIBPATH		= 
INCPATH		= 
CFLAGS		= -Wall -O2 -g

##################################################################

.c.o:
	$(CC) -c $(CFLAGS) $< -o $@

all: $(DECODE_X)

$(DECODE_X) : $(OBJS_2)
	$(CC) -o $@ $(OBJS_2) $(LIBPATH) $(LIBS)

clean:
	rm -f *.o *.core $(DECODE_X)

depend:
	mkdep $(CFLAGS) $(SRCS)

#bench : $(ENCODE_X)
#	@basename `pwd`
#	@./$(ENCODE_X)

