CC      = gcc
CFLAGS  = -Wall -Wextra -g
LDFLAGS = -lcurl -lsqlite3

TARGET  = devlog
DAEMON  = devlogd
SRCDIR  = src
DAEMONDIR = ../daemon
SRCS    = $(wildcard $(SRCDIR)/*.c)
OBJS    = $(SRCS:.c=.o)

.PHONY: all clean install

all: $(TARGET) $(DAEMON)

$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

$(DAEMON): $(DAEMONDIR)/devlogd.c
	$(CC) $(CFLAGS) -o $@ $<

$(SRCDIR)/%.o: $(SRCDIR)/%.c
	$(CC) $(CFLAGS) -c -o $@ $<

install: $(TARGET) $(DAEMON)
	install -m 755 $(TARGET) /usr/local/bin/$(TARGET)
	install -m 755 $(DAEMON) /usr/local/bin/$(DAEMON)

clean:
	rm -f $(OBJS) $(TARGET) $(DAEMON)
