# included from whomever needs us

LOCAL_DIR := $(GET_LOCAL_DIR)

OBJS += \
	$(LOCAL_DIR)/Math.o \
	$(LOCAL_DIR)/Matrix4x4.o \
	$(LOCAL_DIR)/Vector3.o \
	$(LOCAL_DIR)/Vector4.o

CPPFLAGS += -I$(LOCAL_DIR)/..

