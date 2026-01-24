CC=g++

MAKEFLAGS := --jobs=$(shell nproc)

CFLAGS= -shared -fPIC -std=c++23 -g --no-gnu-unique

LDFLAGS= -l:libGLEW.so.2.1 -lSDL2 -lvulkan -lcurl libs/funchook/libfunchook.a libs/funchook/libdistorm.a -z execstack -g

OBJ_FILES =  main.cpp.o # Unity build
OBJ_FILES += libsigscan/libsigscan.c.o # Sigscan library
OBJ_FILES += imgui/imgui_tables.cpp.o imgui/imgui_draw.cpp.o imgui/imgui_impl_sdl2.cpp.o imgui/imgui_demo.cpp.o imgui/imgui_impl_opengl3.cpp.o imgui/imgui_impl_vulkan.cpp.o imgui/imgui_widgets.cpp.o imgui/imgui.cpp.o imgui/imgui_stdlib.cpp.o # GUI library and OpenGL wrapper
OBJ_FILES += hacks/navbot/micropather/micropather.cpp.o # Path solving library
# OBJ_FILES += bitbuf/bitbuf.cpp.o

OBJS = $(addprefix obj/, $(OBJ_FILES)) 

BIN = tf2.so

.PHONY: all clean debug

#-------------------------------------------------------------------------------

all: $(BIN)

clean:
	rm -f $(OBJS)
	rm -f $(BIN)

#-------------------------------------------------------------------------------

$(BIN): $(OBJS)
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

obj/%.cpp.o: %.cpp
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -c -o $@ $<

obj/%.c.o: %.c
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -c -o $@ $<
