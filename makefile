include macros.mk

TARGET := ray
BUILDDIR := build-$(TARGET)

# compiler flags, default libs to link against
COMPILEFLAGS := -Wall -g -O2 -I. -DBOOST_NO_EXCEPTIONS=1
CFLAGS := $(COMPILEFLAGS)
CPPFLAGS := $(COMPILEFLAGS) -fno-exceptions
ASMFLAGS := $(COMPILEFLAGS)
LDFLAGS :=
LDLIBS := -lSDL -lstdc++
OBJDUMP := objdump
CPPFILT := c++filt

UNAME := $(shell uname -s)
ARCH := $(shell uname -m)

# switch any platform specific stuff here
# ifeq ($(findstring CYGWIN,$(UNAME)),CYGWIN)
# ...
# endif
# ifeq ($(UNAME),Darwin)
# ...
# endif

OBJS := \
	main.o \
	DisplayWindow.o \
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

$(BUILDDIR)/$(TARGET):  $(OBJS)
	$(CC) $(LDFLAGS) $(OBJS) -o $@ $(LDLIBS)

$(BUILDDIR)/$(TARGET).lst: $(BUILDDIR)/$(TARGET)
	$(OBJDUMP) -d $< | $(CPPFILT) > $@

$(BUILDDIR)/$(TARGET).debug.lst: $(BUILDDIR)/$(TARGET)
	$(OBJDUMP) -S $< | $(CPPFILT) > $@

clean:
	rm -f $(OBJS) $(DEPS) $(TARGET)

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
