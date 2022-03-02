cmake_minimum_required(VERSION 3.0)
project(webserver)

set(CMAKE_CXX_STANDARD 17)
# 为了显示执行构建过程中更加详细的信息，如为了得到更详细的错误信息
set(CMAKE_VERBOSE_MAKEFILE on)

set(CMAKE_CXX_FLAGS "$ENV{CXXFLAGS} -rdynamic -O3 -fPIC -ggdb -std=c++11 -Wall -Wno-deprecated -Werror -Wno-unused-function -Wno-builtin-macro-redefined -Wno-deprecated-declarations")
set(CMAKE_C_FLAGS "$ENV{CXXFLAGS} -rdynamic -O3 -fPIC -ggdb -std=c11 -Wall -Wno-deprecated -Werror -Wno-unused-function -Wno-builtin-macro-redefined -Wno-deprecated-declarations")


include_directories(webserver)

add_executable(LIB_SRC
        webserver/log.cc
        )

add_library(webserver SHARED ${LIB_SRC})

add_executable(test tests/test.cc)  # 通过指定的源文件列表构建出可执行目标文件
add_dependencies(test webserver)
target_link_libraries(test webserver)

SET(EXECUTABLE_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/bin)
SET(LIBRARY_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/lib)
    
