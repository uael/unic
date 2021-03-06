set(UNIC_THREAD_MODEL beos)
set(UNIC_IPC_MODEL none)
set(UNIC_TIME_PROFILER_MODEL beos)
set(UNIC_DIR_MODEL posix)
set(UNIC_LIBRARYLOADER_MODEL beos)
set(UNIC_RWLOCK_MODEL general)

set(UNIC_PLATFORM_DEFINES
  -D_REENTRANT
  )

set(UNIC_PLATFORM_LINK_LIBRARIES root socket bind)
