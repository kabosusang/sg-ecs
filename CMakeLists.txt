cmake_minimum_required(VERSION 3.30...4.3.1)

if(${CMAKE_VERSION} VERSION_LESS 3.30)
    cmake_policy(VERSION ${CMAKE_MAJOR_VERSION}.${CMAKE_MINOR_VERSION})
else()
  cmake_policy(VERSION 3.30)
endif()

#== == = Clangd compile_commands.json == == =
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

#== == = Module == == =
#set(CMAKE_CXX_FLAGS "-stdlib=libc++")
#set(CMAKE_CXX_MODULE_STD ON)

#== == = Project == == =
project( sg-ecs
         VERSION 1.0
         DESCRIPTION "SG-REFL"
         LANGUAGES CXX)

#== == = Src == ==
file(GLOB_RECURSE SOURCES CONFIGURE_DEPENDS
    ${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp
)

add_executable(sg-ecs ${SOURCES})
target_include_directories(sg-ecs PUBLIC include)

# ===== C++Stand ====
target_compile_features(sg-ecs PUBLIC cxx_std_23)
set_target_properties(sg-ecs PROPERTIES 
    CXX_EXTENSIONS OFF
    #CXX_MODULE_STD ON 
)

#
file(TO_CMAKE_PATH "${PROJECT_BINARY_DIR}/CMakeLists.txt" LOC_PATH)
if(EXISTS "${LOC_PATH}")
    message(FATAL_ERROR "You cannot build in a source directory...")
endif()


# update submodule
find_package(Git QUIET)
if(GIT_FOUND AND EXISTS "${PROJECT_SOURCE_DIR}/.git")
    execute_process(COMMAND ${GIT_EXECUTABLE} submodule update --init --recursive
                    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
                    RESULT_VARIABLE GIT_SUBMOD_RESULT)
    if(NOT GIT_SUBMOD_RESULT EQUAL "0")
        message(FATAL_ERROR "git submodule update --init failed...")
    endif()
endif()


# ccache
find_program(CCACHE_PROGRAM ccache)
if(CCACHE_PROGRAM)
    set(CMAKE_CXX_COMPILER_LAUNCHER "${CCACHE_PROGRAM}")
    set(CMAKE_CUDA_COMPILER_LAUNCHER "${CCACHE_PROGRAM}") # CMake 3.9+
endif()


if(CMAKE_PROJECT_NAME STREQUAL PROJECT_NAME)
    include(CTest)
endif()

















