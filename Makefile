
CFLAGS = -std=c99 -Wall 

all:	rules.d smash 
	
rules.d:  $(wildcard *.c) $(wildcard *.h)
	gcc -MM $(wildcard *.c) >rules.d
	
-include <rules.d>
	
smash:  rules.d
	gcc $(CFLAGS) *.c -o smash
	
debug: rules.d
	gcc $(CFLAGS) *.c -g -o smash

clean:
	rm -f *.o smash *~

 
