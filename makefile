include macros.mk
include host.mk

TARGETS := ray rayview ray2tga raysplit

all: $(addsuffix .do,$(TARGETS))

TARGET := ray

COMPILEFLAGS += -Wextra -Wall -g -O2 -I. -Wno-multichar
CFLAGS += $(COMPILEFLAGS)
CPPFLAGS += $(COMPILEFLAGS)
ASMFLAGS += $(COMPILEFLAGS)
LDFLAGS +=
LDLIBS += -lstdc++ -lm -lboost_program_options-mt -lboost_thread-mt
OBJDUMP := objdump
CPPFILT := c++filt
ALLOBJS :=

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
include mesh/rules.mk
include libvec/rules.mk
include tinyxml/rules.mk

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

# rayview
TARGET := rayview

LDLIBS += -lSDL

# Darwin (Mac OS X)
ifeq ($(UNAME),Darwin)
LDLIBS += -framework Cocoa -lSDLmain
endif

OBJS := \
	rayview.o

include compile.mk

spotless:
	rm -rf build-*

clean: $(addprefix clean-,$(TARGETS))

