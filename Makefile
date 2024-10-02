CFLAGS = -std=c11 -Wall -Wpedantic -Wextra
LDFLAGS = -lm
BENCHES = pwm-fan-speed
TESTS = $(addsuffix -test, $(BENCHES))

all: CFLAGS += -O3
all: $(BENCHES) $(TESTS)

debug: CFLAGS += -ggdb3
debug: $(TESTS)

$(BENCHES): clean
	$(CC) $@.c -o $@ $(CFLAGS) $(LDFLAGS)

$(TESTS): clean
	$(CC) $@.c -o $@ $(CFLAGS) $(LDFLAGS)

clean:
	rm -f $(BENCHES) $(TESTS)
