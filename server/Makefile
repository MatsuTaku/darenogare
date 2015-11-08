OBJS	=	server_main.o server_net.o server_command.o
TARGET	=	server
CFLAGS	=	-c -DNDEBUG

.c.o:
#	gcc $(CFLAGS) $<
	gcc -c $<

all: $(TARGET)

$(TARGET):	$(OBJS)
	gcc -o $@ $^ -lm -lSDL

clean:
	rm *.o $(TARGET) *~ core

server_main.o: server_main.c server_common.h server_func.h ../common.h
server_net.o: server_net.c server_common.h server_func.h ../common.h
server_command.o: server_command.c server_common.h server_func.h ../common.h
