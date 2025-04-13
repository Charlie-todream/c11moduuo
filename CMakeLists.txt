cmake_minimum_required(VERSION 3.10)
project(mymoduo)

# 设置C++标准
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
# mymoduo最终编译成so动态库,设置动态库的路径，放在根目录的lib文件夹下面
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR}/lib)
# 设置调试信息 以及启动C++ 17语言标准
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g -std=c++17 -fPIC")

# 定义参与编译的源代码文件
aux_source_directory(. SRC_LIST)
# 编译生成动态库mymuduo
add_library(mymoduo SHARED ${SRC_LIST})
file(GLOB SOURCES "${SRC_LIST}/*.cpp")