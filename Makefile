CCFLAGS = -std=c11
LIBS = -lm
objects = pwm-fan-speed.o

debug: CCFLAGS += -ggdb3
debug: $(objects)

$(objects): %.o: %.c
	$(CC) $(CCFLAGS) $^ -o $@ $(LIBS)

clean:
	rm -f *.o
