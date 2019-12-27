
CFLAGS = -std=c99 -Wall

OBJS = commands.o history.o

%.o : %.c
	gcc $(CFLAGS) -c -o $@ $<

all: smash rules.d

rules.d:  $(wildcard *.c) $(wildcard *.h)
	gcc -MM $(wildcard *.c) >rules.d

-include <rules.d>

#smash:  rules.d
#	gcc $(CFLAGS) *.c -g -o smash

#debug: rules.d
#	gcc $(CFLAGS) *.c -g -o smash

myLib.a : $(OBJS)
	ar r $@ $?

smash: smash.o myLib.a #rules.d
	gcc -g -o $@ $^

clean:
	rm -f *.o *.a smash *~
