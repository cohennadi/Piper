CC=gcc
LIBS=-lm

all:
        $(CC) piper.c $(LIBS) -o piper

clean:
        rm piper