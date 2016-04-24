#@note   Hikvision Digital Technology Co., Ltd. All Rights Reserved.
#@brief  CMake子目录配置文件
#@author Yaopei
#@date   2015/7/31
#@note   the CMakeLists.txt for src. directory


PROJECT(HMSProxy)

SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++0x -g -Wall -fPIC -O0")

SET(SRC_SRCS
  net_client.cpp
  net_server.cpp
  data_work.cpp
  data_handle.cpp
  trace_handel.cpp
  trace_server.cpp
  cli_server.cpp
)

# shared library
#ADD_LIBRARY(NetApp SHARED ${SRC_SRCS} )

#static library
ADD_LIBRARY(NetApp STATIC ${SRC_SRCS})

# link library
#TARGET_LINK_LIBRARIES(NetApp)

# clean
SET_TARGET_PROPERTIES(NetApp PROPERTIES CLEAN_DIRECT_OUTPUT 1)
#SET_TARGET_PROPERTIES(Decoder_static PROPERTIES CLEAN_DIRECT_OUTPUT 1)
