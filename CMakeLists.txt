set(CMAKE_LEGACY_CYGWIN_WIN32 0)

cmake_minimum_required(VERSION 2.8.0)
project(unic C)

set(UNIC_VERSION_MAJOR 0)
set(UNIC_VERSION_MINOR 1)
set(UNIC_VERSION_PATCH 0)
set(UNIC_VERSION_NUM 0x000100)
set(UNIC_VERSION ${UNIC_VERSION_MAJOR}.${UNIC_VERSION_MINOR}.${UNIC_VERSION_PATCH})
set(VERSION ${UNIC_VERSION})

set(top_srcdir ${PROJECT_SOURCE_DIR})

option(UNIC_TESTS "Build unit tests" ON)
option(UNIC_BUILD_STATIC "Also build static version of the library" ON)
option(COVERAGE "Enable gcov coverage (GCC and Clang)" OFF)
option(UNIC_VISIBILITY "Use explicit symbols visibility if possible" ON)
option(UNIC_BUILD_DOC "Enable building HTML documentation" ON)

if (NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE "Debug")
endif ()

subdirs(src)

message(STATUS "Checking whether to enable unit tests")
if (UNIC_TESTS)
  message(STATUS "Checking whether to enable unit tests - yes")
  enable_testing()
  include(${PROJECT_SOURCE_DIR}/cmake/PlatformDetect.cmake)
  unic_detect_target_os(UNIC_TESTS_TARGET_OS)

  list(APPEND UNIC_TEST_INCLUDE_DIRS ${PROJECT_SOURCE_DIR}/include ${CMAKE_BINARY_DIR})

  if (MSVC)
    list(APPEND UNIC_TEST_COMPILE_DEFS -D_CRT_SECURE_NO_WARNINGS)
  endif ()

  if (CMAKE_VERSION VERSION_LESS "3.1")
    if (GCC)
      set(CMAKE_C_FLAGS "-std=c99 ${CMAKE_C_FLAGS}")
    endif ()
  else ()
    set(CMAKE_C_STANDARD 99)
  endif ()

  macro(unic_add_test_executable TEST_NAME SRC_FILE)
    add_executable(${TEST_NAME} ${SRC_FILE})

    target_link_libraries(${TEST_NAME} unic)
    # QNX requires libm for sqrt() and friends
    if (NOT UNIC_TESTS_TARGET_OS STREQUAL windows)
      target_link_libraries(${TEST_NAME} m)
    endif ()

    # Add include directories
    if (COMMAND target_include_directories)
      target_include_directories(${TEST_NAME} PUBLIC ${UNIC_TEST_INCLUDE_DIRS})
    else ()
      include_directories(${UNIC_TEST_INCLUDE_DIRS})
    endif ()

    # Add compile definitions
    if (COMMAND target_compile_definitions)
      target_compile_definitions(${TEST_NAME} PRIVATE ${UNIC_TEST_COMPILE_DEFS})
    else ()
      add_definitions(${UNIC_TEST_COMPILE_DEFS})
    endif ()

    if (${TEST_NAME} STREQUAL "dl_test")
      add_test(NAME ${TEST_NAME} COMMAND ${TEST_NAME} ARGS "$<TARGET_FILE:unic>")
    else ()
      add_test(NAME ${TEST_NAME} COMMAND ${TEST_NAME})
    endif ()

    set(UNIT_TEST_TARGETS ${UNIT_TEST_TARGETS} ${TEST_NAME})
  endmacro()

  unic_add_test_executable(atomic_test test/atomic.c)
  unic_add_test_executable(condvar_test test/condvar.c)
  unic_add_test_executable(hash_test test/hash.c)
  unic_add_test_executable(error_test test/error.c)
  unic_add_test_executable(dir_test test/dir.c)
  unic_add_test_executable(file_test test/file.c)
  unic_add_test_executable(htable_test test/htable.c)
  unic_add_test_executable(inifile_test test/inifile.c)
  unic_add_test_executable(dl_test test/dl.c)
  unic_add_test_executable(list_test test/list.c)
  unic_add_test_executable(macros_test test/macros.c)
  unic_add_test_executable(main_test test/main.c)
  unic_add_test_executable(mem_test test/mem.c)
  unic_add_test_executable(mutex_test test/mutex.c)
  unic_add_test_executable(process_test test/process.c)
  unic_add_test_executable(rwlock_test test/rwlock.c)
  unic_add_test_executable(sema_test test/sema.c)
  unic_add_test_executable(shm_test test/shm.c)
  unic_add_test_executable(shmbuf_test test/shmbuf.c)
  unic_add_test_executable(socket_test test/socket.c)
  unic_add_test_executable(socketaddr_test test/socketaddr.c)
  unic_add_test_executable(spinlock_test test/spinlock.c)
  unic_add_test_executable(string_test test/string.c)
  unic_add_test_executable(profiler_test test/profiler.c)
  unic_add_test_executable(tree_test test/tree.c)
  unic_add_test_executable(types_test test/types.c)
  unic_add_test_executable(thread_test test/thread.c)

  add_custom_target(tests
    DEPENDS ${UNIT_TEST_TARGETS}
  )
  add_custom_command(TARGET tests
    COMMENT "Run tests"
    POST_BUILD COMMAND ${CMAKE_CTEST_COMMAND} ARGS --output-on-failure
    WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
  )
else ()
  message(STATUS "Checking whether to enable unit tests - no")
endif ()

if (UNIC_BUILD_DOC)
  find_package(Doxygen)

  if (DOXYGEN_FOUND)
    configure_file(${PROJECT_SOURCE_DIR}/Doxyfile.in ${PROJECT_BINARY_DIR}/Doxyfile)

    add_custom_target(doc ALL
      COMMAND ${DOXYGEN_EXECUTABLE} Doxyfile
      WORKING_DIRECTORY ${PROJECT_BINARY_DIR}
      COMMENT "Generating API documentation with Doxygen"
      VERBATIM
      )

    install(DIRECTORY ${PROJECT_BINARY_DIR}/doc/html DESTINATION share/doc)
  endif ()
endif ()

set(CPACK_PACKAGE_NAME ${PROJECT_NAME}-installer)
set(CPACK_PACKAGE_VENDOR "Alexander Saprykin")
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "${PROJECT_NAME} - System routines library")
set(CPACK_PACKAGE_VERSION_MAJOR ${UNIC_VERSION_MAJOR})
set(CPACK_PACKAGE_VERSION_MINOR ${UNIC_VERSION_MINOR})
set(CPACK_PACKAGE_VERSION_PATCH ${UNIC_VERSION_PATCH})
set(CPACK_PACKAGE_VERSION ${UNIC_VERSION})
set(CPACK_PACKAGE_INSTALL_DIRECTORY ${PROJECT_NAME}-${CPACK_PACKAGE_VERSION})
set(CPACK_COMPONENTS_ALL Core)
set(CPACK_COMPONENT_CORE_DISPLAY_NAME "Core components")
set(CPACK_COMPONENT_CORE_DESCRIPTION "Core library with headers")
set(CPACK_COMPONENT_CORE_REQUIRED TRUE)

if (WIN32 AND NOT UNIX)
  set(CPACK_NSIS_DISPLAY_NAME ${PROJECT_NAME})
elseif (UNIX)
  set(CPACK_GENERATOR STGZ)
endif ()

include(CPack)
