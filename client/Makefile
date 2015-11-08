OBJS	=	client_main.o client_net.o client_command.o client_win.o
TARGET	=	client
CFLAGS	=	-c -DNDEBUG

.c.o:
	gcc $(CFLAGS) $<

all: $(TARGET)

$(TARGET):	$(OBJS)
	gcc -o $@ $^ -lm -lSDL -lSDL_image -lSDL_gfx -lSDL_ttf

clean:
	rm *.o $(TARGET) *~ core

client_main.o: client_main.c client_func.h ../common.h
client_net.o: client_net.c client_func.h ../common.h
client_command.o: client_command.c client_func.h ../common.h
client_win.o: client_win.c client_func.h ../common.h
