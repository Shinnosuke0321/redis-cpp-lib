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
            SOURCE_DIR "${CMAKE_BINARY_DIR}/_deps/hiredis-src/hiredis"
    )
    FetchContent_GetProperties(hiredis)
    if (NOT hiredis_POPULATED)
        set(ENABLE_SSL ${REDIS_ENABLE_SSL} CACHE BOOL "" FORCE)
        set(DISABLE_TESTS ON CACHE BOOL "Disable compilation of tests" FORCE)
        set(ENABLE_EXAMPLES OFF CACHE BOOL "" FORCE)
        set(BUILD_SHARED_LIBS OFF CACHE BOOL "" FORCE)
        FetchContent_MakeAvailable(hiredis)

        set(_hiredis_include_dir "${CMAKE_BINARY_DIR}/_deps/hiredis-src")
        set_target_properties(hiredis PROPERTIES
                INTERFACE_INCLUDE_DIRECTORIES "$<BUILD_INTERFACE:${_hiredis_include_dir}>")
        if (REDIS_ENABLE_SSL)
            set_target_properties(hiredis_ssl PROPERTIES
                    INTERFACE_INCLUDE_DIRECTORIES "$<BUILD_INTERFACE:${_hiredis_include_dir}>")
        endif ()
    endif ()
endif ()

target_link_libraries(redis-cpp-lib PUBLIC hiredis::hiredis)
if (REDIS_ENABLE_SSL)
    target_link_libraries(redis-cpp-lib PUBLIC hiredis::hiredis_ssl)
endif ()

# libevent is required for the async event loop (redisLibeventAttach)
# On macOS, probe Homebrew prefix locations to avoid picking up x86_64-only libs
if (APPLE)
    foreach(_brew_prefix /opt/homebrew /usr/local)
        if (EXISTS "${_brew_prefix}/lib/libevent.dylib")
            find_library(LIBEVENT_LIB NAMES event
                    HINTS "${_brew_prefix}/lib"
                    NO_DEFAULT_PATH)
            find_library(LIBEVENT_PTHREADS_LIB NAMES event_pthreads
                    HINTS "${_brew_prefix}/lib"
                    NO_DEFAULT_PATH)
            if (LIBEVENT_LIB)
                break()
            endif ()
        endif ()
    endforeach()
endif ()
if (NOT LIBEVENT_LIB)
    find_library(LIBEVENT_LIB NAMES event REQUIRED)
endif ()
if (NOT LIBEVENT_PTHREADS_LIB)
    find_library(LIBEVENT_PTHREADS_LIB NAMES event_pthreads REQUIRED)
endif ()
target_link_libraries(redis-cpp-lib PUBLIC ${LIBEVENT_LIB} ${LIBEVENT_PTHREADS_LIB})
