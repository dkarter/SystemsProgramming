FILES = hello.c
CC = gcc
CFLAGS = -g -Wall -O2
OBJS = hello.o

hello: $(OBJS)
	$(CC) $(CFLAGS) -o hello $(OBJS)

handin:
	@./handin.rb cs351 prelimlab $(FILES)

clean:
	rm -f *~ *.o hello
