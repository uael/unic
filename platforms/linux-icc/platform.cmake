set(UNIC_THREAD_MODEL posix)
set(UNIC_IPC_MODEL posix)
set(UNIC_TIME_PROFILER_MODEL posix)
set(UNIC_DIR_MODEL posix)
set(UNIC_LIBRARYLOADER_MODEL posix)

set(UNIC_PLATFORM_LINK_LIBRARIES -pthread rt dl imf svml irng intlc)

set(UNIC_PLATFORM_DEFINES
  -D_REENTRANT
  )
