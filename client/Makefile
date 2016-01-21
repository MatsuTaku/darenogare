OBJS	=	client_main.o client_net.o client_command.o client_win.o client_system.o client_scene.o client_menu.o
TARGET	=	client
NDEBUG	=	-DNDEBUG

.c.o:
	gcc -g -c $<
#	gcc -c $(NDEBUG) $<

all: $(TARGET)

$(TARGET):	$(OBJS)
	gcc -o $@ $^ -lm -lSDL -lSDL_image -lSDL_gfx -lSDL_ttf

clean:
	rm *.o $(TARGET) *~ core *.swp

client_main.o: client_main.c client_func.h ../common.h
client_net.o: client_net.c client_func.h ../common.h client_scene.h
client_command.o: client_command.c client_func.h ../common.h
client_win.o: client_win.c client_func.h ../common.h client_scene.h
client_system.o: client_system.c client_func.h ../common.h client_scene.h
client_scene.o: client_scene.c client_scene.h client_func.h client_menu.h
client_menu.o: client_menu.c client_menu.h client_scene.h
