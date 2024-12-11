CC = gcc
CFLAGS = -std=c11

SRCDIR := Test
SRCS = $(wildcard $(SRCDIR)/*.c)
OBJS = $(patsubst $(SRCDIR)/%.c, $(BUILDDIR)/%.o, $(SRCS))
BUILDDIR = Test-build

$(OBJS:.o=): $(OBJS)
	$(CC) -o $@ $^

$(OBJS): $(SRCS)
	test -d $(BUILDDIR) || mkdir -p $(BUILDDIR)
	$(CC) $(CFLAGS) -o $@ -c $<

clean:
	rm -rf $(BUILDDIR)
