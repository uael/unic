set(CMAKE_LEGACY_CYGWIN_WIN32 0)

cmake_minimum_required(VERSION 2.8.0)
project(plibsys C)

set(PLIBSYS_VERSION_MAJOR 0)
set(PLIBSYS_VERSION_MINOR 0)
set(PLIBSYS_VERSION_PATCH 3)
set(PLIBSYS_VERSION_NUM 0x000003)
set(PLIBSYS_VERSION ${PLIBSYS_VERSION_MAJOR}.${PLIBSYS_VERSION_MINOR}.${PLIBSYS_VERSION_PATCH})
set(VERSION ${PLIBSYS_VERSION})

set(top_srcdir ${PROJECT_SOURCE_DIR})

option(PLIBSYS_TESTS "Build unit tests" ON)
option(PLIBSYS_BUILD_STATIC "Also build static version of the library" ON)
option(COVERAGE "Enable gcov coverage (GCC and Clang)" OFF)
option(PLIBSYS_VISIBILITY "Use explicit symbols visibility if possible" ON)
option(PLIBSYS_BUILD_DOC "Enable building HTML documentation" ON)

if (NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE "Debug")
endif ()

subdirs(src)

message(STATUS "Checking whether to enable unit tests")
if (PLIBSYS_TESTS)
  message(STATUS "Checking whether to enable unit tests - yes")
  enable_testing()

  list(APPEND PLIBSYS_TEST_INCLUDE_DIRS ${PROJECT_SOURCE_DIR}/include ${CMAKE_BINARY_DIR})

  if (MSVC)
    list(APPEND PLIBSYS_TEST_COMPILE_DEFS -D_CRT_SECURE_NO_WARNINGS)
  endif ()

  if (CMAKE_VERSION VERSION_LESS "3.1")
    if (GCC)
      set(CMAKE_C_FLAGS "-std=c99 ${CMAKE_C_FLAGS}")
    endif ()
  else ()
    set(CMAKE_C_STANDARD 99)
  endif ()

  macro(plibsys_add_test_executable TEST_NAME SRC_FILE)
    add_executable(${TEST_NAME} ${SRC_FILE})
    target_link_libraries(${TEST_NAME} plibsys m)

    # Add include directories
    if (COMMAND target_include_directories)
      target_include_directories(${TEST_NAME} PUBLIC ${PLIBSYS_TEST_INCLUDE_DIRS})
    else ()
      include_directories(${PLIBSYS_TEST_INCLUDE_DIRS})
    endif ()

    # Add compile definitions
    if (COMMAND target_compile_definitions)
      target_compile_definitions(${TEST_NAME} PRIVATE ${PLIBSYS_TEST_COMPILE_DEFS})
    else ()
      add_definitions(${PLIBSYS_TEST_COMPILE_DEFS})
    endif ()

    if (${TEST_NAME} STREQUAL "dl_test")
      add_test(NAME ${TEST_NAME} COMMAND ${TEST_NAME} ARGS "$<TARGET_FILE:plibsys>")
    else ()
      add_test(NAME ${TEST_NAME} COMMAND ${TEST_NAME})
    endif ()

    set(UNIT_TEST_TARGETS ${UNIT_TEST_TARGETS} ${TEST_NAME})
  endmacro()

  plibsys_add_test_executable(atomic_test test/atomic.c)
  plibsys_add_test_executable(condvar_test test/condvar.c)
  plibsys_add_test_executable(hash_test test/hash.c)
  plibsys_add_test_executable(error_test test/error.c)
  plibsys_add_test_executable(dir_test test/dir.c)
  plibsys_add_test_executable(file_test test/file.c)
  plibsys_add_test_executable(htable_test test/htable.c)
  plibsys_add_test_executable(inifile_test test/inifile.c)
  plibsys_add_test_executable(dl_test test/dl.c)
  plibsys_add_test_executable(list_test test/list.c)
  plibsys_add_test_executable(macros_test test/macros.c)
  plibsys_add_test_executable(main_test test/main.c)
  plibsys_add_test_executable(mem_test test/mem.c)
  plibsys_add_test_executable(mutex_test test/mutex.c)
  plibsys_add_test_executable(process_test test/process.c)
  plibsys_add_test_executable(rwlock_test test/rwlock.c)
  plibsys_add_test_executable(sema_test test/sema.c)
  plibsys_add_test_executable(shm_test test/shm.c)
  plibsys_add_test_executable(shmbuf_test test/shmbuf.c)
  plibsys_add_test_executable(socket_test test/socket.c)
  plibsys_add_test_executable(socketaddr_test test/socketaddr.c)
  plibsys_add_test_executable(spinlock_test test/spinlock.c)
  plibsys_add_test_executable(string_test test/string.c)
  plibsys_add_test_executable(profiler_test test/profiler.c)
  plibsys_add_test_executable(tree_test test/tree.c)
  plibsys_add_test_executable(types_test test/types.c)
  plibsys_add_test_executable(uthread_test test/uthread.c)

  add_custom_target(tests ALL
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

if (PLIBSYS_BUILD_DOC)
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
set(CPACK_PACKAGE_VERSION_MAJOR ${PLIBSYS_VERSION_MAJOR})
set(CPACK_PACKAGE_VERSION_MINOR ${PLIBSYS_VERSION_MINOR})
set(CPACK_PACKAGE_VERSION_PATCH ${PLIBSYS_VERSION_PATCH})
set(CPACK_PACKAGE_VERSION ${PLIBSYS_VERSION})
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
