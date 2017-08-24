include(${PROJECT_SOURCE_DIR}/platforms/common/SCOSV.cmake)

set(UNIC_THREAD_MODEL posix)
set(UNIC_IPC_MODEL sysv)
set(UNIC_TIME_PROFILER_MODEL posix)
set(UNIC_DIR_MODEL posix)
set(UNIC_LIBRARYLOADER_MODEL posix)

set(UNIC_PLATFORM_DEFINES
  -D_REENTRANT
  -D_SIMPLE_R
  )

if (CMAKE_SYSTEM_VERSION VERSION_LESS "5.0")
  set(UNIC_PLATFORM_LINK_LIBRARIES socket nsl gthreads malloc)

  unic_scosv_print_threading_message()
else ()
  set(UNIC_PLATFORM_LINK_LIBRARIES socket nsl -pthread)
  set(UNIC_PLATFORM_CFLAGS -pthread)

  message(
    "SCO OpenServer 6 was not actually tested with GCC
    compiler. This build may or may not work properly.
    Consider running tests before usage.
    ")
endif ()
