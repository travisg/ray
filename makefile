include macros.mk
include host.mk

TARGETS := ray rayview ray2tga raysplit

all: $(addsuffix .do,$(TARGETS))

TARGET := ray

# Darwin (Mac OS X)
ifeq ($(UNAME),Darwin)
LDLIBS += -framework Cocoa -L/sw/lib -L/opt/local/lib -lSDLmain
endif

COMPILEFLAGS += -Wall -g -O2 -I. -Wno-multichar
CFLAGS += $(COMPILEFLAGS)
CPPFLAGS += $(COMPILEFLAGS) -fno-exceptions
ASMFLAGS += $(COMPILEFLAGS)
LDFLAGS +=
LDLIBS += -lSDL -lstdc++
OBJDUMP := objdump
CPPFILT := c++filt

OBJS := \
	main.o \
	RenderSurface.o \
	RayFile.o \
	Tracer.o \
	TraceMaster.o \
	Scene.o \

include drawables/rules.mk
include lights/rules.mk
include shaders/rules.mk
include libmath/rules.mk

include compile.mk

# rayview
TARGET := rayview

OBJS := \
	rayview.o

include compile.mk

# ray2tga
TARGET := ray2tga

OBJS := \
	ray2tga.o \
	RayFile.o

include compile.mk

# raysplit
TARGET := raysplit

OBJS := \
	raysplit.o \
	RayFile.o

include compile.mk

spotless:
	rm -rf build-*

clean: $(addprefix clean-,$(TARGETS))

ifeq ($(filter $(MAKECMDGOALS), clean), )
-include $(DEPS)
endif
