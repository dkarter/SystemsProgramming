HANDIN_FILES = README graph.h graph.c main.c
CC = gcc
CFLAGS = -g -Wall
OBJS = graph.o

graph: main.c $(OBJS)
	$(CC) $(CFLAGS) -o graph main.c $(OBJS)

graph.o: graph.c

clean:
	rm -f *~ *.o graph

handin:
	@./handin.rb cs351 clab $(HANDIN_FILES)
