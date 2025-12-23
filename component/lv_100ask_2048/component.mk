# 获取当前Makefile所在目录作为基础路径
COMPONENT_DIR := $(dir $(lastword $(MAKEFILE_LIST)))

# 添加源文件
COMPONENT_SRCS += $(wildcard $(COMPONENT_DIR)*.c)

# 添加头文件路径
COMPONENT_INCLUDES += -I$(COMPONENT_DIR)

