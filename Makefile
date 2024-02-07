allocate: allocate.c
	gcc -o allocate allocate.c -lm

clean:
	rm -f allocate
	rm -f *.o
