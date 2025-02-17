CC = gcc
CFLAGS = -std=c11 -lm -Wall -Wextra -O2

SRCDIR := Test
BUILDDIR := Test-build
SRCS := $(wildcard $(SRCDIR)/*.c)
EXECS := $(patsubst $(SRCDIR)/%.c, $(BUILDDIR)/%, $(SRCS))

.PHONY: all
all: $(BUILDDIR) $(EXECS)

$(BUILDDIR):
	mkdir -p $(BUILDDIR)

$(BUILDDIR)/%: $(SRCDIR)/%.c | $(BUILDDIR)
	$(CC) $(CFLAGS) -o $@ $<

.PHONY: $(notdir $(EXECS))
$(notdir $(EXECS)): % : $(BUILDDIR)/%

clean:
	rm -rf $(BUILDDIR)
