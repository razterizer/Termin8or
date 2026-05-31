cmake_minimum_required(VERSION 3.24)

project(Termin8or LANGUAGES CXX)

set(TERMIN8OR_CORE_SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/../Core"
    CACHE PATH "Path to the Core checkout")

if(NOT TARGET Core::Core)
  add_subdirectory("${TERMIN8OR_CORE_SOURCE_DIR}"
                   "${CMAKE_CURRENT_BINARY_DIR}/Core"
                   EXCLUDE_FROM_ALL)
endif()

add_library(Termin8or INTERFACE)
add_library(Termin8or::Termin8or ALIAS Termin8or)

target_compile_features(Termin8or INTERFACE cxx_std_20)
target_include_directories(Termin8or INTERFACE
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
  $<INSTALL_INTERFACE:include>
)
target_link_libraries(Termin8or INTERFACE Core::Core)

