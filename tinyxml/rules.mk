# included from whomever needs us

LOCAL_DIR := $(GET_LOCAL_DIR)

OBJS += \
	$(LOCAL_DIR)/tinystr.o \
	$(LOCAL_DIR)/tinyxml.o \
	$(LOCAL_DIR)/tinyxmlerror.o \
	$(LOCAL_DIR)/tinyxmlparser.o

#CPPFLAGS += -I$(LOCAL_DIR)/..

