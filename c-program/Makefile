CC      = gcc
CFLAGS  = -Wall -Wextra -g
TARGET  = diary
SRCS    = src/main.c src/fileio.c src/session.c src/user.c src/diary_write.c src/diary_read.c
OBJS    = $(SRCS:.c=.o)
INCLUDES = -Isrc

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) -o $@ $^

%.o: %.c
	$(CC) $(CFLAGS) $(INCLUDES) -c $< -o $@

clean:
	rm -f $(OBJS) $(TARGET)

.PHONY: all clean
