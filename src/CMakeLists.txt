aux_source_directory(. SRC_LIST)
# set(SRC_LIST mprpcapplication.cc 
#              mprpcconfig.cc
#              mprpcchannel.cc
#              mprpccontroller.cc
#              rpcheader.pb.cc 
#              rpcprovide.cc
#              zookeeperutil.cc
#              )
add_library(mprpc STATIC ${SRC_LIST})
target_link_libraries(mprpc muduo_net muduo_base pthread zookeeper_mt symlog)