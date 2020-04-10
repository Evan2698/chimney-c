cmake_minimum_required(VERSION 3.10)

project(chimney-c)

set(CMAKE_CXX_STANDARD 17)
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
  ${current_dir}/src/core/mobile_core.cpp
)

set (PRIVACY_SRCS
  ${current_dir}/src/privacy/chacha20.cpp
  ${current_dir}/src/privacy/gcm.cpp
  ${current_dir}/src/privacy/privacy.cpp
  ${current_dir}/src/privacy/raw.cpp
  ${current_dir}/src/privacy/xcha20_poly1305.cpp 
  ${current_dir}/src/privacy/xchacha20.cpp
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
    ${current_dir}/3rd/easyloggingpp/src
    ${current_dir}/3rd/rapidjson/include)


add_subdirectory(3rd/easyloggingpp)
add_subdirectory(3rd/libsodium)

  
add_executable(chimney-c ${MAIN_SRCS})
target_include_directories(chimney-c PRIVATE  ${NAV_INCLUDE_DIR})
target_compile_definitions(chimney-c PRIVATE  ELPP_THREAD_SAFE  ELPP_FEATURE_ALL)
target_link_libraries(
                       chimney-c

                       PRIVATE

                       sodium

                       pthread

                       easyloggingpp

                       )