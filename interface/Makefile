CC		= cc
ENCODE_X	= encode
ENCODE		= encode.c
DECODE_X	= decode
DECODE 		= decode.c
RAND_X		= gen_rand_file
RAND 		= gen_rand_file.c

INTERFACES	= gf.c mt19937-64.c
SRCS		= $(ENCODE) $(INTERFACES)
OBJS		= $(SRCS:.c=.o)
SRCS_2		= $(DECODE) $(INTERFACES)
OBJS_2		= $(SRCS_2:.c=.o)
SRCS_3		= $(RAND) $(INTERFACES)
OBJS_3		= $(SRCS_3:.c=.o)

LIBS		= 
LIBPATH		= 
INCPATH		= 
CFLAGS		= -Wall -O2 -g

##################################################################

.c.o:
	$(CC) -c $(CFLAGS) $< -o $@

all: $(ENCODE_X) $(DECODE_X) $(RAND_X)

$(ENCODE_X) : $(OBJS)
	$(CC) -o $@ $(OBJS) $(LIBPATH) $(LIBS)

$(DECODE_X) : $(OBJS_2)
	$(CC) -o $@ $(OBJS_2) $(LIBPATH) $(LIBS)

$(RAND_X) : $(OBJS_3)
	$(CC) -o $@ $(OBJS_3) $(LIBPATH) $(LIBS)

clean:
	rm -f *.o *.core $(ENCODE_X) $(LIBRARAY) $(DECODE_X) $(RAND_X)

depend:
	mkdep $(CFLAGS) $(SRCS)

#bench : $(ENCODE_X)
#	@basename `pwd`
#	@./$(ENCODE_X)

