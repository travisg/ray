TARGET := $(TARGET)
BUILDDIR-$(TARGET) := build-$(TARGET)
#$(warning BUILDDIR = $(BUILDDIR-$(TARGET)))
OBJS-$(TARGET) := $(OBJS)
OBJS-$(TARGET) := $(addprefix $(BUILDDIR-$(TARGET))/,$(OBJS-$(TARGET)))
#$(warning OBJS = $(OBJS-$(TARGET)))
DEPS-$(TARGET) := $(OBJS-$(TARGET):.o=.d)
#$(warning DEPS = $(DEPS-$(TARGET)))

$(TARGET).do: TARGET:=$(TARGET)
$(TARGET).do: $(BUILDDIR-$(TARGET))/$(TARGET) $(BUILDDIR-$(TARGET))/$(TARGET).lst $(BUILDDIR-$(TARGET))/$(TARGET).debug.lst
	@ln -sf $(BUILDDIR-$(TARGET))/$(TARGET)

$(BUILDDIR-$(TARGET))/$(TARGET): TARGET:=$(TARGET)
$(BUILDDIR-$(TARGET))/$(TARGET): $(OBJS-$(TARGET))
	@$(MKDIR)
	@echo linking $<
	@$(CC) $(LDFLAGS) $(OBJS-$(TARGET)) -o $@ $(LDLIBS)

ifeq ($(UNAME),Darwin)
$(BUILDDIR-$(TARGET))/$(TARGET).lst: $(BUILDDIR-$(TARGET))/$(TARGET)
	otool -Vt $< | $(CPPFILT) > $@

$(BUILDDIR-$(TARGET))/$(TARGET).debug.lst: $(BUILDDIR-$(TARGET))/$(TARGET)
	otool -Vt $< | $(CPPFILT) > $@
else
$(BUILDDIR-$(TARGET))/$(TARGET).lst: $(BUILDDIR-$(TARGET))/$(TARGET)
	$(OBJDUMP) -d $< | $(CPPFILT) > $@

$(BUILDDIR-$(TARGET))/$(TARGET).debug.lst: $(BUILDDIR-$(TARGET))/$(TARGET)
	$(OBJDUMP) -S $< | $(CPPFILT) > $@
endif

# makes sure the target dir exists
MKDIR = if [ ! -d $(dir $@) ]; then mkdir -p $(dir $@); fi

$(BUILDDIR-$(TARGET))/%.o: %.c
	@$(MKDIR)
	@echo compiling $<
	@$(CC) $(CFLAGS) -c $< -MD -MT $@ -MF $(@:%o=%d) -o $@

$(BUILDDIR-$(TARGET))/%.o: %.cpp
	@$(MKDIR)
	@echo compiling $<
	@$(CC) $(CPPFLAGS) -c $< -MD -MT $@ -MF $(@:%o=%d) -o $@

$(BUILDDIR-$(TARGET))/%.o: %.S
	@$(MKDIR)
	@echo compiling $<
	@$(CC) $(ASMFLAGS) -c $< -MD -MT $@ -MF $(@:%o=%d) -o $@

clean-$(TARGET): TARGET:=$(TARGET)
clean-$(TARGET):
	rm -f $(TARGET)
	rm -f $(OBJS-$(TARGET)) $(DEPS-$(TARGET))

#$(warning DEPS-$(TARGET) = $(DEPS-$(TARGET)))
ifeq ($(filter $(MAKECMDGOALS), clean), )
-include $(DEPS-$(TARGET))
endif

TARGET :=

