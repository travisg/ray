include macros.mk

TARGET := ray
BUILDDIR := build-$(TARGET)

UNAME := $(shell uname -s)
ARCH := $(shell uname -m)

# switch any platform specific stuff here
# ifeq ($(findstring CYGWIN,$(UNAME)),CYGWIN)
# ...
# endif
# ifeq ($(UNAME),Darwin)
# ...
# endif

# Darwin (Mac OS X)
ifeq ($(UNAME),Darwin)
COMPILEFLAGS += -DASM_LEADING_UNDERSCORES=1 -mdynamic-no-pic -fast -I/sw/include -I/opt/local/include
LDLIBS += -framework Cocoa -L/sw/lib -L/opt/local/lib -lSDLmain
endif

# linux
ifeq ($(UNAME),Linux)
COMPILEFLAGS += -march=native
ifeq ($(ARCH),ppc)
COMPILEFLAGS += -fno-pic -mregnames
COMPILEFLAGS += -mcpu=7450
endif
ifeq ($(ARCH),ppc64)
COMPILEFLAGS += -mregnames -fno-pic
COMPILEFLAGS += -mcpu=970
endif
ifeq ($(ARCH),x86_64)
#COMPILEFLAGS += -march=k8
endif
endif

# compiler flags, default libs to link against
ifneq ($(WITH_PROFILE),)
COMPILEFLAGS += -pg
LDFLAGS += -pg
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
	Tracer.o \
	TraceMaster.o \
	Scene.o \

include drawables/rules.mk
include lights/rules.mk
include shaders/rules.mk
include libmath/rules.mk

OBJS := $(addprefix $(BUILDDIR)/,$(OBJS))

DEPS := $(OBJS:.o=.d)

all: $(BUILDDIR)/$(TARGET) $(BUILDDIR)/$(TARGET).lst $(BUILDDIR)/$(TARGET).debug.lst
	ln -sf $(BUILDDIR)/$(TARGET)
	$(MAKE) -C rayview

$(BUILDDIR)/$(TARGET):  $(OBJS)
	$(CC) $(LDFLAGS) $(OBJS) -o $@ $(LDLIBS)

ifeq ($(UNAME),Darwin)
$(BUILDDIR)/$(TARGET).lst: $(BUILDDIR)/$(TARGET)
	otool -Vt $< | $(CPPFILT) > $@

$(BUILDDIR)/$(TARGET).debug.lst: $(BUILDDIR)/$(TARGET)
	otool -Vt $< | $(CPPFILT) > $@
else
$(BUILDDIR)/$(TARGET).lst: $(BUILDDIR)/$(TARGET)
	$(OBJDUMP) -d $< | $(CPPFILT) > $@

$(BUILDDIR)/$(TARGET).debug.lst: $(BUILDDIR)/$(TARGET)
	$(OBJDUMP) -S $< | $(CPPFILT) > $@
endif

clean:
	rm -f $(OBJS) $(DEPS) $(TARGET)
	$(MAKE) -C rayview clean

spotless:
	rm -rf build-*

# makes sure the target dir exists
MKDIR = if [ ! -d $(dir $@) ]; then mkdir -p $(dir $@); fi

$(BUILDDIR)/%.o: %.c
	@$(MKDIR)
	@echo compiling $<
	@$(CC) $(CFLAGS) -c $< -MD -MT $@ -MF $(@:%o=%d) -o $@

$(BUILDDIR)/%.o: %.cpp
	@$(MKDIR)
	@echo compiling $<
	@$(CC) $(CPPFLAGS) -c $< -MD -MT $@ -MF $(@:%o=%d) -o $@

$(BUILDDIR)/%.o: %.S
	@$(MKDIR)
	@echo compiling $<
	@$(CC) $(ASMFLAGS) -c $< -MD -MT $@ -MF $(@:%o=%d) -o $@

ifeq ($(filter $(MAKECMDGOALS), clean), )
-include $(DEPS)
endif
