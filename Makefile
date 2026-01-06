# Define the compiler to use (g++)
CC = g++

# Define the name of the output executable
target = VirtualKeyboard

# Define the resource directory path
RESDIR := res

# Find all .cpp source files in the src directory
SRCS := $(wildcard ./src/*.cpp)
# Generate corresponding .o object file names from source files
OBJS := $(patsubst %.cpp,%.o,$(SRCS))

# Use sdl2-config for SDL2 flags and pkg-config for SDL2_image/ttf/gfx if available.
# Check if pkg-config is installed
PKGCONFIG := $(shell command -v pkg-config 2>/dev/null)
ifneq ($(PKGCONFIG),)
  # If pkg-config is found, retrieve CFLAGS and LIBS for SDL2 extensions
  IMAGE_CFLAGS := $(shell pkg-config --cflags SDL2_image 2>/dev/null)
  TTF_CFLAGS := $(shell pkg-config --cflags SDL2_ttf 2>/dev/null)
  GFX_CFLAGS := $(shell pkg-config --cflags SDL2_gfx 2>/dev/null)
  IMAGE_LIBS := $(shell pkg-config --libs SDL2_image 2>/dev/null)
  TTF_LIBS := $(shell pkg-config --libs SDL2_ttf 2>/dev/null)
  GFX_LIBS := $(shell pkg-config --libs SDL2_gfx 2>/dev/null)
else
  # If pkg-config is not found, leave these variables empty
  IMAGE_CFLAGS :=
  TTF_CFLAGS :=
  GFX_CFLAGS :=
  IMAGE_LIBS :=
  TTF_LIBS :=
  GFX_LIBS :=
endif

# Combine SDL2 flags from sdl2-config with extension flags
INCLUDE := $(shell sdl2-config --cflags) $(IMAGE_CFLAGS) $(TTF_CFLAGS) $(GFX_CFLAGS)

# Ensure at least SDL2 include present for systems without pkg-config
ifeq ($(strip $(INCLUDE)),)
  # Fallback include path if sdl2-config returns nothing
  INCLUDE := -I/usr/include/SDL2
endif

# Combine SDL2 libs from sdl2-config with extension libs
LIB := $(shell sdl2-config --libs) $(IMAGE_LIBS) $(TTF_LIBS) $(GFX_LIBS)

# fallback to explicit libs if pkg-config didn't find them
ifeq ($(strip $(LIB)),)
  # Fallback linking flags if pkg-config didn't provide them
  LIB := $(shell sdl2-config --libs) -lSDL2_image -lSDL2_ttf -lSDL2_gfx
endif

# Default target: build the executable
all: $(OBJS)
	# Link object files to create the executable
	$(CC) $(OBJS) -o $(target) $(LIB)

# Rule to compile .cpp files to .o files
%.o: %.cpp
	# Compile source file with C++17 standard, define RESDIR macro, and include paths
	$(CC) -std=c++17 -DRESDIR="\"$(RESDIR)\"" -c $< -o $@ $(INCLUDE)

# Clean target to remove generated files
clean:
	# Remove object files and the executable
	rm -f $(OBJS) $(target)
