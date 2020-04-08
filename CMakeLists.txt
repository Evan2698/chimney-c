cmake_minimum_required(VERSION 3.10)

project(chimney-c)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++17")

set (current_dir  ${CMAKE_CURRENT_SOURCE_DIR})

set (CORE_SRCS
  ${current_dir}/src/main.cpp
  ${current_dir}/src/core/address.cpp
  ${current_dir}/src/core/funs.cpp
  ${current_dir}/src/core/minipool.cpp
  ${current_dir}/src/core/peerfactory.cpp
  ${current_dir}/src/core/socketassistant.cpp
  ${current_dir}/src/core/socks5peer.cpp
  ${current_dir}/src/core/socks5server.cpp
  ${current_dir}/src/core/stream.cpp
)

set (PRIVACY_SRCS
  ${current_dir}/src/privacy/chacha20.cpp
  ${current_dir}/src/privacy/gcm.cpp
  ${current_dir}/src/privacy/privacy.cpp
  ${current_dir}/src/privacy/raw.cpp
  ${current_dir}/src/privacy/xcha20_poly1305.cpp 
)

set (UDPSERVER_SRCS
    #${current_dir}/src/udpserver/udppack.cpp
    ${current_dir}/src/udpserver/udpserver.cpp   
   )


set (MAIN_SRCS
    ${CORE_SRCS}
    ${PRIVACY_SRCS}
    ${UDPSERVER_SRCS}
)

set (NAV_INCLUDE_DIR
    ${current_dir}/src/include
    ${current_dir}/3rd/libsodium/src/libsodium/include
    ${current_dir}/3rd/glog/src
    ${current_dir}/3rd/rapidjson/include)


add_subdirectory(3rd/glog)
add_subdirectory(3rd/gflags)
  
add_executable(chimney-c ${MAIN_SRCS})
target_include_directories(chimney-c PRIVATE  ${NAV_INCLUDE_DIR})

target_link_libraries( # Specifies the target library.
                       chimney-c

                       sodium

                       pthread

                       glog

                       gflags

                       )