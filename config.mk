PROJECT=eszFW

ifeq ($(OS),Windows_NT)
	OUT=lib/$(PROJECT).lib
	TOOLCHAIN=i686-w64-mingw32
	AR=$(TOOLCHAIN)-ar
	CC=$(TOOLCHAIN)-cc
else
	OUT=lib/$(PROJECT).a
	TOOLCHAIN=local
	UNAME_S := $(shell uname -s)
endif

LIBS=\
	-lSDL2\
	-lSDL2_image\
	-lSDL2_mixer\
	-lSDL2_ttf\
	-lxml2 -lz -llzma -lm

CFLAGS=\
	-D_REENTRANT\
	-DSDL_MAIN_HANDLED\
	-DWANT_ZLIB\
	-Isrc\
	-Isrc/inih\
	-Isrc/tmx/src\
	-isystem /usr/$(TOOLCHAIN)/include/libxml2\
	-O2\
	-pedantic-errors\
	-std=c99\
	-Wall\
	-Werror\
	-Wextra

SRCS=\
	$(wildcard src/*.c)\
	$(wildcard src/inih/*.c)\
	$(wildcard src/tmx/src/*.c)

OBJS=$(patsubst %.c, %.o, $(SRCS))
