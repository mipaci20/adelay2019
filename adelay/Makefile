#
#    Makefile for Avatar
#

# TRACER FLAGS
TRACER = 

INCLUDE_PATH = 

# Other constants
CONST =

FLAGS = -Wall -Wstrict-prototypes \
	-Wcast-qual -Wcast-align -finline-functions -g

LIB_PATH =

CC = gcc ${FLAGS} $(TRACER) $(CONST)

LIB = $(LIB_PATH) -lm

OBJS = fft.o adelay.o aufileutil.o exp_tilde.o flags.o open_file.o skip_space.o af.o snd.o aiff.o strsave.o convertIEEE.o

TESTS = fft.t 
TARFILE = faq setup.gif setup.fig
ARCHIVE = adelay.tar.gz
ALL =  adelay

all : $(ALL)

adelay : $(OBJS)
	$(CC) $(INCLUDE_PATH) -o adelay $(OBJS) $(LIB)

%.o : %.c
	$(CC) $(INCLUDE_PATH) -c $<

fft.t : fft.t.c fft.o
	$(CC) -o $@ fft.o $@.c $(LIB)

tar:
	tar cvf - *.[hc] Makefile | \
	gzip -c > $(ARCHIVE)

clean :
	- /bin/rm -f *.o $(TESTS) $(ALL) a.out core
