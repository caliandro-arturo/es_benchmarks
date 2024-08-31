CFLAGS = -std=c11
LIBS = -lm
objects = pwm-fan-speed.o

$(objects): %.o: %.c
	gcc -ggdb3 $^ -o $@ $(LIBS)

clean:
	rm -f *.o
