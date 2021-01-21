# For more information about using CMake with Android Studio, read the
# documentation: https://d.android.com/studio/projects/add-native-code.html

# Sets the minimum version of CMake required to build the native library.

cmake_minimum_required(VERSION 3.4.1)

message(${ANDROID_ABI})

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -std=c++14")
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -O2")
set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -O3")

set(engine_source_dir ${CMAKE_CURRENT_SOURCE_DIR}/../../../../../../../deps/GowEngine/src)
set(core_source_dir ${CMAKE_CURRENT_SOURCE_DIR}/../../../../../../core)

file(GLOB ENGINE_CPP_FILES "${engine_source_dir}/*.cpp")
file(GLOB ENGINE_ANDROID_CPP_FILES "${engine_source_dir}/android/*.cpp")
file(GLOB CORE_CPP_FILES "${core_source_dir}/*.cpp")

include_directories(${engine_source_dir}/../deps/rapidjson/include)
include_directories(${engine_source_dir}/../deps/stb)
include_directories(${engine_source_dir})
include_directories(${core_source_dir})
include_directories(${CMAKE_CURRENT_SOURCE_DIR})

# Creates and names a library, sets it as either STATIC
# or SHARED, and provides the relative paths to its source code.
# You can define multiple libraries, and CMake builds them for you.
# Gradle automatically packages shared libraries with your APK.

add_library( # Sets the name of the library.
             native-lib

             # Sets the library as a shared library.
             SHARED

             ${ENGINE_CPP_FILES}
             ${ENGINE_ANDROID_CPP_FILES}
             ${CORE_CPP_FILES}

             JNIEngine.cpp )

### INCLUDE OBOE LIBRARY ###

# Set the path to the Oboe library directory
set (OBOE_DIR ${engine_source_dir}/../deps/oboe)

# Add the Oboe library as a subproject. Since Oboe is an out-of-tree source library we must also
# specify a binary directory
add_subdirectory(${OBOE_DIR} ./oboe-bin)

# Specify the path to the Oboe header files and the source.
include_directories(
        ${OBOE_DIR}/include
        ${OBOE_DIR}/src
)

### END OBOE INCLUDE SECTION ###

# Specifies libraries CMake should link to your target library. You
# can link multiple libraries, such as libraries you define in this
# build script, prebuilt third-party libraries, or system libraries.

target_link_libraries(native-lib android log GLESv3 EGL oboe)
