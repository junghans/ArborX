cmake_minimum_required(VERSION 3.16)
project(ArborX CXX)

if(CMAKE_VERSION VERSION_GREATER_EQUAL 3.30.0)
  message(STATUS "Setting policy CMP0167 to use FindBoost module")
  cmake_policy(SET CMP0167 OLD)
endif()

find_package(Kokkos 4.3 REQUIRED CONFIG)
message(STATUS "Found Kokkos: ${Kokkos_DIR} (version \"${Kokkos_VERSION}\")")

# We use minimum compiler versions from Kokkos, with the exception for NVCC (>= 11.5)
if(Kokkos_CXX_COMPILER_ID STREQUAL "NVIDIA" AND Kokkos_CXX_COMPILER_VERSION VERSION_LESS 11.5.0)
  message(FATAL_ERROR "NVCC versions prior to 11.5 are not supported by ArborX")
endif()

add_library(ArborX INTERFACE)
target_link_libraries(ArborX INTERFACE Kokkos::kokkos)
set_target_properties(ArborX PROPERTIES INTERFACE_COMPILE_FEATURES cxx_std_17)
# As all executables using ArborX depend on it, depending on record_hash allows
# updating hash each time executable is rebuilt, including when called from
# within a subdirectory.
add_dependencies(ArborX record_hash)

include(CMakeDependentOption)
cmake_dependent_option(ARBORX_ENABLE_ROCTHRUST "Enable rocThrust support" ON "Kokkos_ENABLE_HIP" OFF)
if(Kokkos_ENABLE_HIP AND ARBORX_ENABLE_ROCTHRUST)
  find_package(rocthrust REQUIRED CONFIG)
  target_link_libraries(ArborX INTERFACE roc::rocthrust)
endif()

if(Kokkos_ENABLE_HIP AND NOT ARBORX_ENABLE_ROCTHRUST)
  message(WARNING "rocThrust is NOT enabled.\nThis will negatively impact performance on AMD GPUs.")
endif()

cmake_dependent_option(ARBORX_ENABLE_ONEDPL "Enable oneDPL support" ON "Kokkos_ENABLE_SYCL" OFF)
if(Kokkos_ENABLE_SYCL AND ARBORX_ENABLE_ONEDPL)
  include(CheckIncludeFileCXX)
  check_include_file_cxx(oneapi/dpl/execution ARBORX_COMPILER_HAS_ONEDPL_EXECUTION_HEADER)
  check_include_file_cxx(oneapi/dpl/algorithm ARBORX_COMPILER_HAS_ONEDPL_ALGORITHM_HEADER)
  if (ARBORX_COMPILER_HAS_ONEDPL_EXECUTION_HEADER AND
      ARBORX_COMPILER_HAS_ONEDPL_ALGORITHM_HEADER)
    set(ARBORX_CXX_COMPILER_BUNDLED_ONEDPL TRUE)
  else()
    set(ARBORX_CXX_COMPILER_BUNDLED_ONEDPL FALSE)
  endif()
  if(NOT ARBORX_CXX_COMPILER_BUNDLED_ONEDPL)
    find_package(oneDPL REQUIRED)
    target_link_libraries(ArborX INTERFACE oneDPL)
  endif()
endif()

# Refer to the alias target in examples and benchmarks so they can be built
# against an installed ArborX
add_library(ArborX::ArborX ALIAS ArborX)

option(ARBORX_ENABLE_MPI "Enable MPI support" OFF)
if(ARBORX_ENABLE_MPI)
  find_package(MPI REQUIRED)
  target_link_libraries(ArborX INTERFACE MPI::MPI_CXX)
endif()
cmake_dependent_option(ARBORX_ENABLE_CUDA_AWARE_MPI
                       "Allow using device data in MPI communication"
                       OFF "ARBORX_ENABLE_MPI" OFF)
if(ARBORX_USE_CUDA_AWARE_MPI)
  set(ARBORX_ENABLE_GPU_AWARE_MPI ON)
    message(DEPRECATION "ARBORX_USE_CUDA_AWARE_MPI is deprecated, use ARBORX_ENABLE_GPU_AWARE_MPI instead")
endif()

cmake_dependent_option(ARBORX_ENABLE_GPU_AWARE_MPI
                       "Allow using device data in MPI communication"
                       OFF "ARBORX_ENABLE_MPI" OFF)

target_include_directories(ArborX INTERFACE
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/src>
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/src/geometry>
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/src/cluster>
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/src/distributed>
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/src/interpolation>
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/src/spatial>
  $<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}/include>
  $<INSTALL_INTERFACE:include/ArborX>
  $<INSTALL_INTERFACE:include/ArborX/geometry>
  $<INSTALL_INTERFACE:include/ArborX/cluster>
  $<INSTALL_INTERFACE:include/ArborX/distributed>
  $<INSTALL_INTERFACE:include/ArborX/interpolation>
  $<INSTALL_INTERFACE:include/ArborX/spatial>
)

install(TARGETS ArborX
  EXPORT ArborXTargets
  ARCHIVE LIBRARY PUBLIC_HEADER
)

install(EXPORT ArborXTargets
  NAMESPACE ArborX::
  DESTINATION lib/cmake/ArborX
)

set(ARBORX_VERSION_STRING "2.0 (dev)")

# Make sure that the git hash in ArborX_Version.hpp is considered to be always
# out of date, and thus is updated every recompile.
add_custom_target(
  record_hash ALL VERBATIM
  COMMAND ${CMAKE_COMMAND}
    -DSOURCE_DIR=${CMAKE_CURRENT_SOURCE_DIR}
    -DBINARY_DIR=${CMAKE_CURRENT_BINARY_DIR}
    -DARBORX_VERSION_STRING=${ARBORX_VERSION_STRING}
    -P cmake/SetupVersion.cmake
  WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR})
# Also run the record_hash command during configuration stage to have a visible
# ArborX_Version.hpp at all times.
execute_process(
  COMMAND ${CMAKE_COMMAND}
    -DSOURCE_DIR=${CMAKE_CURRENT_SOURCE_DIR}
    -DBINARY_DIR=${CMAKE_CURRENT_BINARY_DIR}
    -DARBORX_VERSION_STRING=${ARBORX_VERSION_STRING}
    -P cmake/SetupVersion.cmake
  WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR})

configure_file(${CMAKE_CURRENT_SOURCE_DIR}/src/ArborX_Config.hpp.in
               ${CMAKE_CURRENT_BINARY_DIR}/include/ArborX_Config.hpp)

configure_file(${CMAKE_CURRENT_SOURCE_DIR}/cmake/ArborXSettings.cmake.in
               ${CMAKE_CURRENT_BINARY_DIR}/ArborXSettings.cmake
               @ONLY)

include(CMakePackageConfigHelpers)
configure_package_config_file(cmake/ArborXConfig.cmake.in
  ${CMAKE_CURRENT_BINARY_DIR}/ArborXConfig.cmake
  INSTALL_DESTINATION lib/cmake/ArborX
)
write_basic_package_version_file(${CMAKE_CURRENT_BINARY_DIR}/ArborXConfigVersion.cmake
  VERSION ${ARBORX_VERSION_STRING}
  COMPATIBILITY AnyNewerVersion
)
install(FILES
  ${CMAKE_CURRENT_BINARY_DIR}/ArborXConfig.cmake
  ${CMAKE_CURRENT_BINARY_DIR}/ArborXConfigVersion.cmake
  ${CMAKE_CURRENT_BINARY_DIR}/ArborXSettings.cmake
  DESTINATION lib/cmake/ArborX )

if(ARBORX_ENABLE_MPI)
  install(DIRECTORY ${PROJECT_SOURCE_DIR}/src/ DESTINATION include/ArborX
          FILES_MATCHING PATTERN "*.hpp")
else()
  install(DIRECTORY ${PROJECT_SOURCE_DIR}/src/ DESTINATION include/ArborX
          FILES_MATCHING PATTERN "*.hpp"
          PATTERN "*Distribut*" EXCLUDE)
endif()
install(DIRECTORY ${PROJECT_BINARY_DIR}/include/ DESTINATION include/ArborX
        FILES_MATCHING PATTERN "*.hpp")

if(NOT CMAKE_BUILD_TYPE)
  set(default_build_type "RelWithDebInfo")
  message(STATUS "Setting build type to '${default_build_type}' as none was specified.")
  set(CMAKE_BUILD_TYPE "${default_build_type}" CACHE STRING
    "Choose the type of build, options are: Debug, Release, RelWithDebInfo and MinSizeRel."
    FORCE)
endif()

option(ARBORX_ENABLE_TESTS "Enable tests" OFF)
option(ARBORX_ENABLE_EXAMPLES "Enable examples" OFF)
option(ARBORX_ENABLE_BENCHMARKS "Enable benchmarks" OFF)

if(ARBORX_ENABLE_TESTS OR ARBORX_ENABLE_EXAMPLES OR ARBORX_ENABLE_BENCHMARKS)
  enable_testing()
endif()

cmake_dependent_option(ARBORX_ENABLE_HEADER_SELF_CONTAINMENT_TESTS "Enable header self-containment unit tests" ON "ARBORX_ENABLE_TESTS" OFF)
if(ARBORX_ENABLE_HEADER_SELF_CONTAINMENT_TESTS)
  # Globbing all the header filenames to test for self-containment and presence of header guards
  file(GLOB_RECURSE ArborX_HEADERS RELATIVE ${CMAKE_SOURCE_DIR}/src src/*.hpp)
  # Findout what headers are using macros defined in ArborX_Config.hpp
  file(STRINGS src/ArborX_Config.hpp.in ArborX_DEFINITIONS REGEX "define ARBORX_")
  foreach(_definition ${ArborX_DEFINITIONS})
    string(REGEX REPLACE "(#define |#cmakedefine )" "" _macro ${_definition})
    list(APPEND ArborX_MACROS ${_macro})
  endforeach()
  foreach(_file ${ArborX_HEADERS})
    foreach(_macro ${ArborX_MACROS})
      file(STRINGS src/${_file} _includes_mpi REGEX "mpi.h")
      if(_includes_mpi)
        list(APPEND ArborX_HEADERS_MUST_ENABLE_MPI ${_file})
      endif()
      file(STRINGS src/${_file} _has_macro REGEX "${_macro}")
      if(_has_macro)
        list(APPEND ArborX_HEADERS_MUST_INCLUDE_CONFIG_HPP ${_file})
        continue()
      endif()
    endforeach()
  endforeach()
endif()
if(ARBORX_ENABLE_TESTS)
  add_subdirectory(test)
endif()
if(ARBORX_ENABLE_EXAMPLES)
  add_subdirectory(examples)
endif()
if(ARBORX_ENABLE_BENCHMARKS)
  add_subdirectory(benchmarks)
endif()
