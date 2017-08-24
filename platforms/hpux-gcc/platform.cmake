include(${PROJECT_SOURCE_DIR}/platforms/common/HPUX.cmake)

set(UNIC_THREAD_MODEL posix)
set(UNIC_IPC_MODEL sysv)
set(UNIC_TIME_PROFILER_MODEL solaris)
set(UNIC_DIR_MODEL posix)

unic_hpux_detect_libraryloader_model(UNIC_LIBRARYLOADER_MODEL)

set(UNIC_PLATFORM_LINK_LIBRARIES xnet rt -pthread)

set(UNIC_PLATFORM_DEFINES
  -D_REENTRANT
  -D_THREAD_SAFE
  -D_XOPEN_SOURCE_EXTENDED=1
  )
