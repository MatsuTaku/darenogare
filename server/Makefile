OBJS	=	server_main.o server_net.o server_command.o server_system.o
TARGET	=	server
NDEBUG	=	-DNDEBUG

.c.o:
	gcc -c $<
#	gcc -c $(NDEBUG) $<

all: $(TARGET)

$(TARGET):	$(OBJS)
	gcc -o $@ $^ -lm -lSDL

clean:
	rm *.o $(TARGET) *~ core *.swp

server_main.o: server_main.c server_common.h server_func.h ../common.h
server_net.o: server_net.c server_common.h server_func.h ../common.h
server_command.o: server_command.c server_common.h server_func.h ../common.h
server_system.o: server_system.c server_common.h server_func.h ../common.h
