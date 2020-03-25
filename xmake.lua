add_rules("mode.debug", "mode.release")
set_languages("c++17")

target("sodium")
    set_kind("static")
    add_includedirs("3rd/libsodium/src//libsodium/include")
    add_includedirs("3rd/libsodium/src//libsodium/include/sodium")    
    add_files("3rd/libsodium/src/libsodium/**.c")




target("chimney-c")
    set_kind("binary")
    add_files("src/**.cpp")
    add_deps("sodium")
    add_includedirs("src/include")
    add_includedirs("3rd/logfault/include")
    add_includedirs("3rd/rapidjson/include")
    add_includedirs("3rd/libsodium/src//libsodium/include")
    add_links("pthread")

