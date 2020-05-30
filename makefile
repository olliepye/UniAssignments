TARGET= Assignment1
FLAGS= -std=gnu99 -Wall -Werror
LIBS= -I../ZDK -L../ZDK -lzdk -lncurses -lm

all:
	gcc $(TARGET).c -o $(TARGET) $(FLAGS) $(LIBS)