
add_rules("mode.debug", "mode.release")
set_languages("c++17")


target("chimney-c")
    set_kind("binary")
    add_files("src/**.cpp")
    add_includedirs("src/include", "3rd/rapidjson/include", "3rd/glog/src", "3rd/libsodium/src/libsodium/include")
    add_linkdirs("3rd/gflags/build/lib", "3rd/glog/.libs")
    add_links("pthread", "glog","gflags", "sodium")
 
