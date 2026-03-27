option(REDIS_ENABLE_SSL "Enable SSL/TLS support (requires OpenSSL)" ON)

# On macOS, probe both Apple Silicon and Intel Homebrew prefix locations for OpenSSL
if (APPLE AND NOT DEFINED OPENSSL_ROOT_DIR)
    foreach(_brew_prefix /opt/homebrew /usr/local)
        if (EXISTS "${_brew_prefix}/opt/openssl")
            set(OPENSSL_ROOT_DIR "${_brew_prefix}/opt/openssl")
            break()
        endif ()
    endforeach()
endif ()

find_package(hiredis QUIET)
if (NOT hiredis_FOUND)
    include(FetchContent)
    FetchContent_Declare(
            hiredis
            GIT_REPOSITORY https://github.com/redis/hiredis.git
            GIT_TAG v1.3.0
    )
    FetchContent_GetProperties(hiredis)
    if (NOT hiredis_POPULATED)
        set(ENABLE_SSL ${REDIS_ENABLE_SSL} CACHE BOOL "" FORCE)
        set(DISABLE_TESTS ON CACHE BOOL "Disable compilation of tests" FORCE)
        set(ENABLE_EXAMPLES OFF CACHE BOOL "" FORCE)
        set(BUILD_SHARED_LIBS OFF CACHE BOOL "" FORCE)
        FetchContent_MakeAvailable(hiredis)
    endif ()
endif ()

target_link_libraries(redis-cpp-lib PUBLIC hiredis::hiredis)
if (REDIS_ENABLE_SSL)
    target_link_libraries(redis-cpp-lib PUBLIC hiredis::hiredis_ssl)
endif ()
