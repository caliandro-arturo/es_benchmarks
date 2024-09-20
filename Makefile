CFLAGS = -std=c11
LDFLAGS = -lm
BENCHES =
TESTS = $(addsuffix -test, $(BENCHES))

all: CFLAGS += -O3
all: $(BENCHES) $(TESTS)

debug: CFLAGS += -ggdb3
debug: $(TESTS)

$(BENCHES):
	$(CC) $@.c -o $@ $(CFLAGS) $(LDFLAGS)

$(TESTS):
	$(CC) $@.c -o $@ $(CFLAGS) $(LDFLAGS)

clean:
	rm -f $(BENCHES) $(TESTS)
