
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


