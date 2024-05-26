CC = gcc

CFLAGS = -Wall -g

SRCS = main.c 

OBJS = $(SRCS:.c=.o)

EXEC = scheduler

all: $(EXEC)

$(EXEC): $(OBJS)
	$(CC) $(CFLAGS) -o $(EXEC) $(OBJS)

%.o : %.c
	$(CC) $(CFLAGS) -c $<

clean:
	rm -f $(OBJS) $(EXEC)
