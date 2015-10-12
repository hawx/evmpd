default: evmpd

%.o: %.c
		gcc -c $< -o $@

evmpd: evmpd.o
		gcc evmpd.o -o $@ -std=c99 -lmpdclient -levdev

clean:
		-rm -f evmpd.o
		-rm -f evmpd.c\~
		-rm -f evmpd
