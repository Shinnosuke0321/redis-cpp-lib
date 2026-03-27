cmake_minimum_required(VERSION 3.20)
project(redis-cpp-lib VERSION 1.0.0 LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

option(REDIS_CPP_LIB_SHARED "Build redis-cpp-lib as a shared library" OFF)

if (REDIS_CPP_LIB_SHARED)
    add_library(redis-cpp-lib SHARED
            lib/src/redis_priv.cpp
            lib/src/redis_pub.cpp
    )
    # On Windows, export all symbols automatically for shared builds
    if (WIN32)
        set_target_properties(redis-cpp-lib PROPERTIES WINDOWS_EXPORT_ALL_SYMBOLS ON)
    endif ()
else ()
    add_library(redis-cpp-lib STATIC
            lib/src/redis_priv.cpp
            lib/src/redis_pub.cpp
    )
endif ()

target_include_directories(redis-cpp-lib
        PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/lib/include>
        $<INSTALL_INTERFACE:include>
)

add_library(RedisCppDriver::RedisCppDriver ALIAS redis-cpp-lib)

target_compile_features(redis-cpp-lib PUBLIC cxx_std_23)

# Platform-specific settings
if (WIN32)
    target_compile_definitions(redis-cpp-lib PRIVATE
            WIN32_LEAN_AND_MEAN
            NOMINMAX
            _CRT_SECURE_NO_WARNINGS
    )
    target_link_libraries(redis-cpp-lib PRIVATE ws2_32)
endif ()

if (MSVC)
    target_compile_options(redis-cpp-lib PRIVATE /W4 /utf-8)
else ()
    target_compile_options(redis-cpp-lib PRIVATE -Wall -Wextra)
endif ()

include(CTest)
if (PROJECT_IS_TOP_LEVEL AND BUILD_TESTING)
    add_subdirectory(tests)
endif ()

include(cmake/hiredis.cmake)

add_subdirectory(dependencies/core)
target_link_libraries(redis-cpp-lib PUBLIC
        DbConnectionPool::DbConnectionPool
)

# Install support (find_package consumption)
include(GNUInstallDirs)
include(CMakePackageConfigHelpers)

install(TARGETS redis-cpp-lib
        EXPORT RedisCppLibTargets
        ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
        LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
        RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

install(DIRECTORY lib/include/
        DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
)

install(EXPORT RedisCppLibTargets
        FILE RedisCppLibTargets.cmake
        NAMESPACE RedisCppDriver::
        DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/RedisCppLib
)

configure_package_config_file(
        cmake/RedisCppLibConfig.cmake.in
        ${CMAKE_CURRENT_BINARY_DIR}/RedisCppLibConfig.cmake
        INSTALL_DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/RedisCppLib
)

write_basic_package_version_file(
        ${CMAKE_CURRENT_BINARY_DIR}/RedisCppLibConfigVersion.cmake
        VERSION ${PROJECT_VERSION}
        COMPATIBILITY SameMajorVersion
)

install(FILES
        ${CMAKE_CURRENT_BINARY_DIR}/RedisCppLibConfig.cmake
        ${CMAKE_CURRENT_BINARY_DIR}/RedisCppLibConfigVersion.cmake
        DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/RedisCppLib
)
