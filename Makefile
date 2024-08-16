CFLAGS = -std=c11
objects = pwm-fan-speed.o

$(objects): %.o: %.c
	gcc $^ -o $@

clean:
	rm -f *.c *.o
