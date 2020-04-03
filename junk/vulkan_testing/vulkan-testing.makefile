PROGRAM_NAME = tests/vulkan-testing
PROGRAM_OBJ = tests/vulkan_testing range print

LDLIBS += -lvulkan

#VULKAN_PATH=/opt/vulkan
#LDLIBS += -L$(VULKAN_PATH)/lib
#CFLAGS += -I$(VULKAN_PATH)/include

PKG_LDLIBS != pkg-config --libs glfw3
PKG_CFLAGS != pkg-config --cflags glfw3

CFLAGS += $(PKG_CFLAGS)
LDLIBS += $(PKG_LDLIBS)
