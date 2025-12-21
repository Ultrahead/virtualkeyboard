CC = g++
target = VirtualKeyboard

RESDIR := res

SRCS := $(wildcard ./src/*.cpp)
OBJS := $(patsubst %.cpp,%.o,$(SRCS))

# Use sdl2-config for SDL2 flags and pkg-config for SDL2_image/ttf/gfx if available.
PKGCONFIG := $(shell command -v pkg-config 2>/dev/null)
ifneq ($(PKGCONFIG),)
  IMAGE_CFLAGS := $(shell pkg-config --cflags SDL2_image 2>/dev/null)
  TTF_CFLAGS := $(shell pkg-config --cflags SDL2_ttf 2>/dev/null)
  GFX_CFLAGS := $(shell pkg-config --cflags SDL2_gfx 2>/dev/null)
  IMAGE_LIBS := $(shell pkg-config --libs SDL2_image 2>/dev/null)
  TTF_LIBS := $(shell pkg-config --libs SDL2_ttf 2>/dev/null)
  GFX_LIBS := $(shell pkg-config --libs SDL2_gfx 2>/dev/null)
else
  IMAGE_CFLAGS :=
  TTF_CFLAGS :=
  GFX_CFLAGS :=
  IMAGE_LIBS :=
  TTF_LIBS :=
  GFX_LIBS :=
endif

INCLUDE := $(shell sdl2-config --cflags) $(IMAGE_CFLAGS) $(TTF_CFLAGS) $(GFX_CFLAGS)
# Ensure at least SDL2 include present for systems without pkg-config
ifeq ($(strip $(INCLUDE)),)
  INCLUDE := -I/usr/include/SDL2
endif

LIB := $(shell sdl2-config --libs) $(IMAGE_LIBS) $(TTF_LIBS) $(GFX_LIBS)
# fallback to explicit libs if pkg-config didn't find them
ifeq ($(strip $(LIB)),)
  LIB := $(shell sdl2-config --libs) -lSDL2_image -lSDL2_ttf -lSDL2_gfx
endif

all: $(OBJS)
	$(CC) $(OBJS) -o $(target) $(LIB)

%.o: %.cpp
	$(CC) -std=c++17 -DRESDIR="\"$(RESDIR)\"" -c $< -o $@ $(INCLUDE)

clean:
	rm -f $(OBJS) $(target)
