add_requires("conan::libsodium/1.0.18@bincrafters/stable", {alias = "libsodium"}) 
add_requires("conan::glog/0.4.0@bincrafters/stable", {alias = "glog"}) 

add_rules("mode.debug", "mode.release")
set_languages("c++17")


target("chimney-c")
    set_kind("binary")
    add_files("src/**.cpp")
    add_includedirs("src/include")
    add_includedirs("3rd/rapidjson/include")
    add_links("pthread", "dl")
    add_packages("libsodium", "glog")
 
