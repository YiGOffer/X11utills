# cmake 最低版本需求
cmake_minimum_required(VERSION 3.4)

# 工程名称
project(DACS_Preload)

set(DACS_Preload_VERSION_MAJOR 1)
set(DACS_Preload_VERSION_MINOR 0)

# 设置
set(CMAKE_CXX_STANDARD 11)

# 编译源码生成目标
# add_executable(DACS_Preload dacspreload.c)

#set(CMAKE_CXX_FLAGS -g)
#set(CMAKE_CXX_FLAGS -Wall)
#set(CMAKE_CXX_FLAGS -fPIC)
#set(CMAKE_CXX_FLAGS -shared)
#set(CMAKE_CXX_FLAGS -pthread)
#set(CMAKE_CXX_FLAGS -lpthread)

set(CMAKE_C_FLAGS   "-fPIC -shared -ldl")  

option(RELEASE_BUILD    "Release build" ON)
message("RELEASE_BUILD is ${RELEASE_BUILD}")
if(RELEASE_BUILD)
    add_definitions(-DRELEASE_BUILD)
else(RELEASE_BUILD)
    set(CMAKE_C_FLAGS     "${CMAKE_C_FLAGS} -g")  
endif(RELEASE_BUILD)

message("Preload:")
message(STATUS "CMAKE_C_FLAGS = ${CMAKE_C_FLAGS}")

# CheckAtomName编译相关
set(CMAKE_C_FLAGS   "-I/usr/include/X11 -lX11")
if(RELEASE_BUILD )

else(RELEASE_BUILD)
    set(CMAKE_C_FLAGS     "${CMAKE_C_FLAGS} -g")  
endif(RELEASE_BUILD)

message("CheckAtomName:")
message(STATUS "CMAKE_C_FLAGS = ${CMAKE_C_FLAGS}")

set(GETATOMNANE_SRCS
    CheckAtomName.c
    window_manage.c
)

add_executable(CheckAtomName ${GETATOMNANE_SRCS})
TARGET_LINK_LIBRARIES(CheckAtomName X11)

# Monitor编译相关
set(CMAKE_C_FLAGS   "-I/usr/include/X11 -lX11")  
if(RELEASE_BUILD )

else(RELEASE_BUILD)
    set(CMAKE_C_FLAGS     "${CMAKE_C_FLAGS} -g")  
endif(RELEASE_BUILD)

set(GETWINDOWATTRIBUTES_SRC
    GetWindowAttributes.c
)
add_executable(GetWindowAttributes ${GETWINDOWATTRIBUTES_SRC})
TARGET_LINK_LIBRARIES(GetWindowAttributes X11)

# Monitor编译相关
set(CMAKE_C_FLAGS   "-I/usr/include/X11 -lX11")  
if(RELEASE_BUILD )

else(RELEASE_BUILD)
    set(CMAKE_C_FLAGS     "${CMAKE_C_FLAGS} -g")  
endif(RELEASE_BUILD)
