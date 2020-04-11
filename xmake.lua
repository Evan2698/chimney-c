
add_rules("mode.debug", "mode.release")
set_languages("c++17")


target("sodium")
    set_kind("static")
    add_files("3rd/libsodium/src/libsodium/crypto_aead/aes256gcm/aesni/aead_aes256gcm_aesni.c",
	"3rd/libsodium/src/libsodium/crypto_aead/chacha20poly1305/sodium/aead_chacha20poly1305.c",
	"3rd/libsodium/src/libsodium/crypto_aead/xchacha20poly1305/sodium/aead_xchacha20poly1305.c", 
	"3rd/libsodium/src/libsodium/crypto_core/ed25519/ref10/ed25519_ref10.c", 
	"3rd/libsodium/src/libsodium/crypto_core/hchacha20/core_hchacha20.c", 
	"3rd/libsodium/src/libsodium/crypto_core/salsa/ref/core_salsa_ref.c", 
	"3rd/libsodium/src/libsodium/crypto_generichash/blake2b/ref/blake2b-compress-ref.c", 
	"3rd/libsodium/src/libsodium/crypto_generichash/blake2b/ref/blake2b-ref.c", 
	"3rd/libsodium/src/libsodium/crypto_generichash/blake2b/ref/generichash_blake2b.c",  
	"3rd/libsodium/src/libsodium/crypto_onetimeauth/poly1305/onetimeauth_poly1305.c",  
	"3rd/libsodium/src/libsodium/crypto_onetimeauth/poly1305/donna/poly1305_donna.c",  
	"3rd/libsodium/src/libsodium/crypto_pwhash/crypto_pwhash.c",  
	"3rd/libsodium/src/libsodium/crypto_pwhash/argon2/argon2-core.c",  
	"3rd/libsodium/src/libsodium/crypto_pwhash/argon2/argon2.c",  
	"3rd/libsodium/src/libsodium/crypto_pwhash/argon2/argon2-encoding.c",  
	"3rd/libsodium/src/libsodium/crypto_pwhash/argon2/argon2-fill-block-ref.c",  
	"3rd/libsodium/src/libsodium/crypto_pwhash/argon2/blake2b-long.c",  
	"3rd/libsodium/src/libsodium/crypto_pwhash/argon2/pwhash_argon2i.c",  
	"3rd/libsodium/src/libsodium/crypto_scalarmult/curve25519/scalarmult_curve25519.c",  
	"3rd/libsodium/src/libsodium/crypto_scalarmult/curve25519/ref10/x25519_ref10.c",  
	"3rd/libsodium/src/libsodium/crypto_stream/chacha20/stream_chacha20.c",  
	"3rd/libsodium/src/libsodium/crypto_stream/chacha20/ref/chacha20_ref.c",  
	"3rd/libsodium/src/libsodium/crypto_stream/salsa20/stream_salsa20.c",  
	"3rd/libsodium/src/libsodium/crypto_stream/salsa20/ref/salsa20_ref.c",  
	"3rd/libsodium/src/libsodium/crypto_verify/sodium/verify.c",  
	"3rd/libsodium/src/libsodium/randombytes/randombytes.c",  
	"3rd/libsodium/src/libsodium/randombytes/sysrandom/randombytes_sysrandom.c",  
	"3rd/libsodium/src/libsodium/sodium/core.c",  
	"3rd/libsodium/src/libsodium/sodium/runtime.c",  
	"3rd/libsodium/src/libsodium/sodium/utils.c",  
	"3rd/libsodium/src/libsodium/sodium/version.c", 
    "3rd/libsodium/src/libsodium/crypto_pwhash/argon2/pwhash_argon2id.c", 
    "3rd/libsodium/src/libsodium/sodium/codecs.c", 
    "3rd/libsodium/src/libsodium/crypto_generichash/crypto_generichash.c", 
    "3rd/libsodium/src/libsodium/crypto_stream/crypto_stream.c", 
    "3rd/libsodium/src/libsodium/crypto_stream/xsalsa20/stream_xsalsa20.c", 
    "3rd/libsodium/src/libsodium/crypto_core/hsalsa20/ref2/core_hsalsa20_ref2.c",
    "3rd/libsodium/src/libsodium/crypto_stream/xchacha20/stream_xchacha20.c",
    "3rd/libsodium/src/libsodium/crypto_auth/hmacsha256/auth_hmacsha256.c",
    "3rd/libsodium/src/libsodium/crypto_hash/sha256/cp/hash_sha256_cp.c"
    )

    add_defines(
        "PACKAGE_VERSION=\"1.0.18\"",
        "PACKAGE_STRING=\"libsodium-1.0.18\"",
		"PACKAGE_BUGREPORT=\"https://github.com/jedisct1/libsodium/issues\"",
		"PACKAGE_URL=\"https://github.com/jedisct1/libsodium\"",
		"PACKAGE=\"libsodium\"",
        "VERSION=\"1.0.18\"",
		"HAVE_PTHREAD=1",                  
		"STDC_HEADERS=1",                  
		"HAVE_SYS_TYPES_H=1",              
		"HAVE_SYS_STAT_H=1",               
		"HAVE_STDLIB_H=1",                 
		"HAVE_STRING_H=1",                 
		"HAVE_MEMORY_H=1",                 
		"HAVE_STRINGS_H=1",                
		"HAVE_INTTYPES_H=1",               
		"HAVE_STDINT_H=1",                 
		"HAVE_UNISTD_H=1",                 
		"__EXTENSIONS__=1",                
		"_ALL_SOURCE=1",                   
		"_GNU_SOURCE=1",                   
		"_POSIX_PTHREAD_SEMANTICS=1",      
		"_TANDEM_SOURCE=1",                
		"HAVE_DLFCN_H=1",                  
		"LT_OBJDIR=\".libs/\"",            
		"HAVE_SYS_MMAN_H=1",               
		"NATIVE_LITTLE_ENDIAN=1",          
		"ASM_HIDE_SYMBOL=.hidden",         
		"HAVE_WEAK_SYMBOLS=1",             
		"HAVE_ATOMIC_OPS=1",               
		"HAVE_ARC4RANDOM=1",               
		"HAVE_ARC4RANDOM_BUF=1",           
		"HAVE_MMAP=1",                     
		"HAVE_MLOCK=1",                    
		"HAVE_MADVISE=1",                  
		"HAVE_MPROTECT=1",                 
		"HAVE_NANOSLEEP=1",                
		"HAVE_POSIX_MEMALIGN=1",           
		"HAVE_GETPID=1",                   
		"CONFIGURED=1",
        "DEV_MODE=1",
		"HAVE_TMMINTRIN_H=1",
        "HAVE_WMMINTRIN_H=1",
--[[#PC on here]]
        "HAVE_AMD64_ASM_V=1",
		"HAVE_AVX_ASM_V=1",
		"AVE_CPUID_V=1",
		"HAVE_TI_MODE_V=1",      
        "HAVE_CPUID=1",
        "__x86_64__=1")
    add_includedirs(
       "3rd/libsodium/src/libsodium/include", 
       "3rd/libsodium/src/libsodium/include/sodium")


target("easyloggingpp")
    set_kind("static")
    add_files("3rd/easyloggingpp/src/**.cc")
    add_includedirs("3rd/easyloggingpp/src")
    add_defines("ELPP_THREAD_SAFE", 
	             "ELPP_FRESH_LOG_FILE",
				 "ELPP_FEATURE_ALL",
				 "ELPP_LOGGING_FLAGS_FROM_ARG",
				 "ELPP_NO_DEFAULT_LOG_FILE")
 


target("chimney-c")
    set_kind("binary")
    add_files("src/**.cpp")
    add_deps("easyloggingpp","sodium")
    add_includedirs("src/include", "3rd/rapidjson/include", "3rd/easyloggingpp/src")
    add_links("pthread")
	add_defines("ELPP_THREAD_SAFE", 
	             "ELPP_FRESH_LOG_FILE",
				 "ELPP_FEATURE_ALL",
				 "ELPP_LOGGING_FLAGS_FROM_ARG",
				 "ELPP_NO_DEFAULT_LOG_FILE")

 
