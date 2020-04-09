
add_rules("mode.debug", "mode.release")
set_languages("c++17")

target("easyloggingpp")
    set_kind("static")
    add_files("3rd/easyloggingpp/src/**.cc")
    add_includedirs("3rd/easyloggingpp/src")
    add_defines("ELPP_THREAD_SAFE")
    add_defines("ELPP_FEATURE_ALL")

target("chimney-c")
    set_kind("binary")
    add_files("src/**.cpp")
    add_deps("easyloggingpp")
    add_includedirs("src/include", "3rd/rapidjson/include", "3rd/easyloggingpp/src", "3rd/libsodium/src/libsodium/include")
    add_linkdirs("3rd/gflags/build/lib", "3rd/glog/.libs")
    add_links("pthread", "sodium")
    add_defines("ELPP_THREAD_SAFE")
    add_defines("ELPP_FEATURE_ALL")
 
