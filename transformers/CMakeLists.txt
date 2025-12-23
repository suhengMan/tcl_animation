# 函数：解析 component.mk 文件，提取源文件和包含路径
function(parse_component_mk mk_file out_sources out_includes)
    # 读取 .mk 文件内容
    file(READ "${mk_file}" mk_content)
    
    # 获取 .mk 文件所在目录
    get_filename_component(component_dir "${mk_file}" DIRECTORY)
    
    # 获取当前值（从父作用域）
    set(current_sources ${${out_sources}})
    set(current_includes ${${out_includes}})
    
    # 解析 COMPONENT_SRCS 行：查找 wildcard 模式
    # 例如：COMPONENT_SRCS += $(wildcard $(COMPONENT_DIR)*.c)
    string(REGEX MATCH "COMPONENT_SRCS[^#\n]*wildcard[^#\n]*\\.c" src_line "${mk_content}")
    if(src_line)
        # 在 component 目录中查找所有 .c 文件
        file(GLOB component_srcs "${component_dir}/*.c")
        list(APPEND current_sources ${component_srcs})
    endif()
    
    # 解析 COMPONENT_INCLUDES 行：查找 -I 路径
    # 例如：COMPONENT_INCLUDES += -I$(COMPONENT_DIR)
    string(REGEX MATCH "COMPONENT_INCLUDES[^#\n]*" inc_line "${mk_content}")
    if(inc_line)
        # 如果包含 -I$(COMPONENT_DIR) 或类似模式，添加 component 目录
        if(inc_line MATCHES "COMPONENT_DIR")
            list(APPEND current_includes "${component_dir}")
        endif()
        # 也支持直接指定路径的情况
        string(REGEX MATCH "-I[^\\s]+" direct_inc "${inc_line}")
        if(direct_inc)
            string(REGEX REPLACE "-I" "" inc_path "${direct_inc}")
            get_filename_component(inc_path "${inc_path}" ABSOLUTE BASE_DIR "${component_dir}")
            list(APPEND current_includes "${inc_path}")
        endif()
    endif()
    
    # 返回结果到父作用域
    set(${out_sources} ${current_sources} PARENT_SCOPE)
    set(${out_includes} ${current_includes} PARENT_SCOPE)
endfunction()

# 查找所有 component.mk 文件
file(GLOB_RECURSE COMPONENT_MK_FILES 
    "${CMAKE_CURRENT_SOURCE_DIR}/../component/*/component.mk"
)

# 从 .mk 文件收集源文件和包含路径
set(MK_SOURCES "")
set(MK_INCLUDES "")
foreach(mk_file ${COMPONENT_MK_FILES})
    parse_component_mk("${mk_file}" MK_SOURCES MK_INCLUDES)
endforeach()

# 递归收集当前目录下的 .c/.cpp 源码
file(GLOB_RECURSE UI_SRC CONFIGURE_DEPENDS
    "${CMAKE_CURRENT_SOURCE_DIR}/*.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/page/*/*.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp"
)

# 合并从 .mk 文件解析出的源文件
list(APPEND UI_SRC ${MK_SOURCES})

# 组织需要导出的包含路径
set(UI_INC
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/page_manager/inc
    ${CMAKE_CURRENT_SOURCE_DIR}/page/**
    ${CMAKE_CURRENT_SOURCE_DIR}/component
)
# 合并从 .mk 文件解析出的包含路径
list(APPEND UI_INC ${MK_INCLUDES})
# 去重
list(REMOVE_DUPLICATES UI_INC)

# 把源码与包含路径“导出”给上层（不生成任何目标/库）
set(AI_S2_SOURCES ${UI_SRC} PARENT_SCOPE)
set(AI_S2_INCLUDE_DIRS ${UI_INC} PARENT_SCOPE)

# 如果需要给全局加一些编译宏（例如启用简化 lv_conf 包含），可按需导出变量或在顶层添加：
# set(AI_S2_DEFS LV_CONF_INCLUDE_SIMPLE PARENT_SCOPE)
