CFLAGS = -std=c11
objects =

$(objects): %.o: %.c
	gcc $^ -o $@

clean:
	rm -f *.c *.o
