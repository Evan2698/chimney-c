cmake_minimum_required(VERSION 3.10)

project(chimney-c)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++17")

set (current_dir  ${CMAKE_CURRENT_SOURCE_DIR})

set(ANDORID_P 0)
if (DEFINED CMAKE_ANDROID_ARCH  )  
  set(ANDORID_P 1)
endif()

list (APPEND CORE_SRCS 
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

if (ANDORID_P EQUAL  0)
  list (APPEND CORE_SRCS 
    ${current_dir}/src/main.cpp)
endif()

set (PRIVACY_SRCS
  ${current_dir}/src/privacy/chacha20.cpp
  ${current_dir}/src/privacy/gcm.cpp
  ${current_dir}/src/privacy/privacy.cpp
  ${current_dir}/src/privacy/raw.cpp
  ${current_dir}/src/privacy/xcha20_poly1305.cpp 
  ${current_dir}/src/privacy/xchacha20.cpp
)

set (UDPSERVER_SRCS
    ${current_dir}/src/udpserver/udpserver.cpp   
   )

set (RPC_SRCS
     ${current_dir}/src/rpc/callee.cpp
     ${current_dir}/src/rpc/callerpool.cpp
     ${current_dir}/src/rpc/clientcaller.cpp
)


set (MAIN_SRCS
    ${CORE_SRCS}
    ${PRIVACY_SRCS}
    ${UDPSERVER_SRCS}
    ${RPC_SRCS}
)

set (NAV_INCLUDE_DIR
    ${current_dir}/src/include
    ${current_dir}/3rd/libsodium/src/libsodium/include
    ${current_dir}/3rd/easyloggingpp/src
    ${current_dir}/3rd/rapidjson/include)


add_subdirectory(3rd/easyloggingpp)
add_subdirectory(3rd/libsodium)

if (ANDORID_P EQUAL  0) 
    add_executable(chimney-c ${MAIN_SRCS})
else()
    add_library(chimney-c ${MAIN_SRCS})
endif()

target_include_directories(chimney-c PUBLIC  ${NAV_INCLUDE_DIR})

list(APPEND link_LIB_TARGET 
                          sodium 
                          easyloggingpp )
if(ANDORID_P EQUAL  0) 
    list(APPEND link_LIB_TARGET 
                          pthread)
endif()

target_compile_definitions(chimney-c PUBLIC  
                  ELPP_THREAD_SAFE 
                  ELPP_FEATURE_ALL
                  ELPP_LOGGING_FLAGS_FROM_ARG
                  ELPP_NO_DEFAULT_LOG_FILE
                  ELPP_FRESH_LOG_FILE)

target_link_libraries(
                       chimney-c

                       PUBLIC

                       ${link_LIB_TARGET})