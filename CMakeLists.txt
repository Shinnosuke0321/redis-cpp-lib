cmake_minimum_required(VERSION 3.20)
project(redis-cpp-lib LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

add_library(redis-cpp-lib STATIC
        lib/src/redis_priv.cpp
        lib/src/redis_pub.cpp
)

target_include_directories(redis-cpp-lib
        PUBLIC
        "lib/include"
)

add_library(RedisCppLib::RedisCppLib ALIAS redis-cpp-lib)

target_compile_features(redis-cpp-lib PUBLIC cxx_std_23)

include(CTest)
if (PROJECT_IS_TOP_LEVEL AND BUILD_TESTING)
    add_subdirectory(tests)
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
        set(ENABLE_SSL ON CACHE BOOL "" FORCE)
        set(DISABLE_TESTS ON CACHE BOOL "Disable compilation of tests" FORCE)
        set(ENABLE_EXAMPLES OFF CACHE BOOL "" FORCE)
        set(BUILD_SHARED_LIBS ON CACHE BOOL "" FORCE)
        FetchContent_MakeAvailable(hiredis)
    endif ()
endif ()

target_link_libraries(redis-cpp-lib PUBLIC
        hiredis::hiredis
        hiredis::hiredis_ssl
)

add_subdirectory(dependencies/core)
target_link_libraries(redis-cpp-lib PUBLIC
        DbConnectionPool::DbConnectionPool
)