# set(SRC_LIST userservice.cc ../user.pb.cc)

set(SRC_LIST friendservice.cc ../friend.pb.cc)
add_executable(provider ${SRC_LIST})
target_link_libraries(provider mprpc protobuf)

# # 设置cmake的最低版本和项目名称
# cmake_minimum_required(VERSION 3.0)
# project(callee)

# # 生成debug版本，可以进行gdb调试
# set(CMAKE_BUILD_TYPE "Debug")
# # 设置项目可执行文件输出的路径
# set(EXECUTABLE_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/bin)
# # 设置项目库文件输出的路径
# # set(LIBRARY_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/lib)

# # 设置项目编译头文件搜索路径 -I
# include_directories(/home/mzy/Code/MpRpc/src/include)
# include_directories(/usr/include/mprpc)
# # include_directories(${PROJECT_SOURCE_DIR}/example)
# # include_directories(${PROJECT_SOURCE_DIR}/thirdparty)
# # 设置项目库文件搜索路径 -L
# link_directories(/usr/local/lib)

# add_executable(provider friendservice.cc  friend.pb.cc)
# target_link_libraries(provider mprpc protobuf pthread zookeeper_mt)



# cmake_minimum_required(VERSION 3.0)
# project(callee)

# # 生成debug版本，可以进行gdb调试
# set(CMAKE_BUILD_TYPE "Debug")

# # 设置项目可执行文件输出的路径
# set(EXECUTABLE_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/bin)

# # 设置项目库文件输出的路径
# # set(LIBRARY_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/lib)

# # 设置项目编译头文件搜索路径 -I
# # include_directories(/home/mzy/Code/MpRpc/src/include)
# include_directories(/usr/include/mprpc)
# include_directories(/usr/local/include)  # 可能需要包含muduo的头文件

# # 设置项目库文件搜索路径 -L
# link_directories(/usr/local/lib)

# # 源文件列表
# set(SRC_LIST friendservice.cc friend.pb.cc)

# # 添加可执行文件
# add_executable(provider ${SRC_LIST})

# # 链接库文件
# target_link_libraries(provider mprpc protobuf pthread zookeeper_mt muduo_net muduo_base)
