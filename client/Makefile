OBJS	=	client_main.o client_net.o client_win.o client_system.o client_scene.o client_title.o client_loading.o client_result.o
TARGET	=	client
NDEBUG	=	-DNDEBUG
INCLUDE	=	-I ../
LINK	=	-lm -lSDL -lSDL_image -lSDL_gfx -lSDL_ttf

.c.o:
	gcc -g -c $(INCLUDE) $<

all: $(TARGET)

$(TARGET):	$(OBJS)
	gcc -o $@ $^ $(LINK)

release: $(OBJS)
	gcc -o $(TARGET) $^ $(LINK) $(NDEBUG)

clean:
	rm *.o $(TARGET) *~ core .*.swp .*.swo

client_main.o: client_main.c client_func.h ../common.h
client_net.o: client_net.c client_func.h ../common.h client_scene.h
client_win.o: client_win.c client_func.h ../common.h client_scene.h
client_system.o: client_system.c client_func.h ../common.h client_scene.h client_battle.h
client_scene.o: client_scene.c client_scene.h client_func.h client_title.h client_result.h
client_title.o: client_title.c client_title.h client_scene.h client_func.h client_common.h
client_loading.o: client_loading.c client_loading.h client_scene.h client_func.h client_common.h ../common.h
client_result.o: client_result.c client_result.h ../common.h client_battle.h client_common.h client_func.h
