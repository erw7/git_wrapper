cmake_minimum_required(VERSION 2.8.12)
project(git_wrapper)

option(USE_COTIRE "Use cotire" OFF)

if(USE_COTIRE)
  set(CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR}/cotire/CMake)
  include(cotire)
  set(COTIRE_MINIMUM_NUMBER_OF_TARGET_SOURCES 1)
endif()

get_filename_component(srcdir "${CMAKE_SOURCE_DIR}" REALPATH)
get_filename_component(bindir "${CMAKE_BINARY_DIR}" REALPATH)
if("${srcdir}" STREQUAL "${bindir}")
  message("")
  message("git_wrapper does't support in-tree builds.")
  message("use a build/ subdirectory:")
  message("    mkdir build")
  message("    cd build")
  message("    cmake <OPTIONS> ..")
  message("")
  message("Make sure to cleanup some CMake artifacts from this failed build")
  message("with:")
  message("    del CMakeCache.txt")
  message("    rmdir /Q /S CMakeFiles")
  message("")
  message(FATAL_ERROR "Stopping build.")
endif()

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -municode")
find_package(Boost REQUIRED COMPONENTS
             filesystem system)
add_executable(git main.cpp git_wrapper.h)
target_link_libraries(git Boost::filesystem Boost::system)
if(USE_COTIRE)
  cotire(git)
endif()

install(TARGETS git RUNTIME DESTINATION bin)
set(install_prefix ${CMAKE_INSTALL_PREFIX})
string(REPLACE "/" "\\\\" install_prefix ${install_prefix})
install(CODE "execute_process(
    COMMAND ${CMAKE_COMMAND} -E remove ${CMAKE_INSTALL_PREFIX}/bin/gitk.exe
    COMMAND ${CMAKE_COMMAND} -E remove ${CMAKE_INSTALL_PREFIX}/bin/git-gui.exe
  )"
)
install(CODE "execute_process(
    COMMAND cmd.exe /c mklink /h ${install_prefix}\\\\bin\\\\gitk.exe ${install_prefix}\\\\bin\\\\git.exe
    COMMAND cmd.exe /c mklink /h ${install_prefix}\\\\bin\\\\git-gui.exe ${install_prefix}\\\\bin\\\\git.exe
  )"
)
