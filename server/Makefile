OBJS	=	server_main.o server_net.o server_system.o server_scene.o server_loading.o
TARGET	=	server
NDEBUG	=	-DNDEBUG

.c.o:
	gcc -c -g $<
#	gcc -c $(NDEBUG) $<

all: $(TARGET)

$(TARGET):	$(OBJS)
	gcc -o $@ $^ -lm -lSDL

clean:
	rm *.o $(TARGET) *~ core .*.swp .*.swo

server_main.o: server_main.c server_common.h server_func.h ../common.h
server_net.o: server_net.c server_common.h server_func.h ../common.h
server_system.o: server_system.c server_common.h server_func.h ../common.h
server_scene.o: server_scene.c server_scene.h ../common.h server_loading.h
server_loading.o: server_loading.c server_loading.h ../common.h server_net.c
