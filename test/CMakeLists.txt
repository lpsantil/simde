cmake_minimum_required(VERSION 3.0)

project(portable-snippet-tests)

set(CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/cmake)
include (ExtraWarningFlags)

enable_testing()

if(NOT EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/munit/munit.c")
  find_program(GIT git)
  if(GIT)
    execute_process(COMMAND ${GIT} submodule update --init --recursive)
  else()
    message (FATAL_ERROR "It looks like you don't have submodules checked out.  Please run `git submodule update --init --recursive'")
  endif()
endif()

if(CMAKE_BUILD_TYPE STREQUAL "")
  set(CMAKE_BUILD_TYPE "Debug")
elseif(CMAKE_BUILD_TYPE STREQUAL "Coverage")
  set(orig_req_libs "${CMAKE_REQUIRED_LIBRARIES}")
  set(CMAKE_REQUIRED_LIBRARIES "${CMAKE_REQUIRED_LIBRARIES};--coverage")
  check_c_compiler_flag("--coverage" CFLAG___coverage)
  set(CMAKE_REQUIRED_LIBRARIES "${orig_req_libs}")

  if(CFLAG___coverage)
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} --coverage")
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_C_FLAGS} --coverage")
    add_definitions("-DSIMDE_NO_INLINE")
  else()
    set(CMAKE_BUILD_TYPE "Debug")
  endif()
endif()

add_library(munit STATIC munit/munit.c)

include(CheckFunctionExists)
check_function_exists(clock_gettime CLOCK_GETTIME_RES)
if(CLOCK_GETTIME_RES)
  set(CLOCK_GETTIME_EXISTS yes)
else()
  set(orig_req_libs "${CMAKE_REQUIRED_LIBRARIES}")
  set(CMAKE_REQUIRED_LIBRARIES "${CMAKE_REQUIRED_LIBRARIES};rt")

  check_function_exists(clock_gettime CLOCK_GETTIME_LIBRT_RES)
  if(CLOCK_GETTIME_LIBRT_RES)
    set(CLOCK_GETTIME_EXISTS yes)
    set(CLOCK_GETTIME_LIBRARY "rt")
  endif()

  set(CMAKE_REQUIRED_LIBRARIES "${orig_req_libs}")
  unset(orig_req_libs)
endif()

find_library(LIBM m)

set_property(TARGET munit PROPERTY C_STANDARD "99")
if("${CLOCK_GETTIME_EXISTS}")
  target_compile_definitions(munit PRIVATE "MUNIT_ALLOW_CLOCK_GETTIME")
  target_link_libraries(munit "${CLOCK_GETTIME_LIBRARY}")
endif()

set(TEST_SOURCES
  test.c
  test-common.c
  test-mmx.c
  test-sse.c
  test-sse2.c
  test-sse3.c
  test-ssse3.c
  test-sse4.1.c
  test-sse4.2.c)

set_compiler_specific_flags(
  VARIABLE extra_cflags
  INTEL -wd3179
  GCCISH -march=native)

check_c_compiler_flag("-fopenmp-simd" CFLAG__fopenmp_simd)
check_c_compiler_flag("-qopenmp-simd" CFLAG__qopenmp_simd)
if(CFLAG__fopenmp_simd)
  set(extra_cflags "${extra_cflags} -fopenmp-simd -DSIMDE_ENABLE_OPENMP")
elseif(CFLAG__qopenmp_simd)
  set(extra_cflags "${extra_cflags} -qopenmp-simd -DSIMDE_ENABLE_OPENMP")
endif()

foreach(variant native emul)
  add_executable(test-${variant} ${TEST_SOURCES})
  target_link_libraries(test-${variant} munit)
  if(LIBM)
    target_link_libraries(test-${variant} ${LIBM})
  endif()
  foreach(tst "/mmx" "/sse" "/sse2" "/sse3" "/ssse3" "/sse4.1" "/sse4.2")
    add_test(NAME "/${variant}${tst}" COMMAND $<TARGET_FILE:test-${variant}> "/${variant}${tst}")
  endforeach()
  target_add_extra_warning_flags(test-${variant})
  set_property (TARGET test-${variant} PROPERTY C_STANDARD "99")
  target_add_compiler_flags(test-${variant} ${extra_cflags})
endforeach()
target_compile_definitions(test-emul PRIVATE -DSIMDE_NO_NATIVE)

add_executable(test-cmp
  test-cmp.c
  test-common.c
  test-cmp-mmx.c
  test-cmp-sse.c
  test-cmp-sse2.c)
target_link_libraries(test-cmp munit)
if(LIBM)
  target_link_libraries(test-cmp ${LIBM})
endif()
add_test(NAME "/cmp" COMMAND $<TARGET_FILE:test-cmp>)
target_add_extra_warning_flags(test-cmp)
set_property (TARGET test-cmp PROPERTY C_STANDARD "99")
target_add_compiler_flags(test-cmp ${extra_cflags})
