include(CheckCSourceCompiles)
include(CheckTypeSize)
include(CheckIncludeFile)
include(TestBigEndian)
include(${PROJECT_SOURCE_DIR}/cmake/PlatformDetect.cmake)
include(${PROJECT_SOURCE_DIR}/cmake/VisibilityDetect.cmake)
set(OUTPUT_DIR ${CMAKE_BINARY_DIR})

# Try to detect target platform
unic_detect_target_platform(UNIC_TARGET_PLATFORM)
unic_detect_c_compiler(UNIC_C_COMPILER)
unic_detect_target_os(UNIC_TARGET_OS)
unic_detect_os_bits(UNIC_OS_BITS)

if ((UNIC_TARGET_OS STREQUAL windows) AND NOT (UNIC_TARGET_OS STREQUAL cygwin)
  AND NOT (UNIC_TARGET_OS STREQUAL msys))
  set(UNIC_NATIVE_WINDOWS true)
endif ()

if (COVERAGE)
  if (UNIC_C_COMPILER MATCHES "gcc|clang")
    set(CMAKE_BUILD_TYPE "Debug")
    list(APPEND CMAKE_C_FLAGS "--coverage")
  endif ()
endif ()

# CMP0042, see http://www.cmake.org/Wiki/CMake_RPATH_handling
if (UNIC_TARGET_OS STREQUAL darwin)
  set(CMAKE_MACOSX_RPATH TRUE)
  set(CMAKE_SKIP_BUILD_RPATH FALSE)

  # Fix runtime paths on macOS 10.5 and less
  if (CMAKE_SYSTEM_VERSION VERSION_LESS "10.0.0")
    set(CMAKE_BUILD_WITH_INSTALL_RPATH TRUE)
  else ()
    set(CMAKE_BUILD_WITH_INSTALL_RPATH FALSE)
  endif ()

  set(CMAKE_INSTALL_RPATH "${CMAKE_INSTALL_PREFIX}/lib")
  set(CMAKE_INSTALL_RPATH_USE_LINK_PATH TRUE)

  list(FIND CMAKE_PLATFORM_IMPLICIT_LINK_DIRECTORIES "${CMAKE_INSTALL_PREFIX}/lib" isSystemDir)

  if ("${isSystemDir}" STREQUAL "-1")
    set(CMAKE_INSTALL_RPATH "${CMAKE_INSTALL_PREFIX}/lib")
  endif ("${isSystemDir}" STREQUAL "-1")
endif ()

if (NOT EXISTS "${PROJECT_SOURCE_DIR}/platforms/${UNIC_TARGET_PLATFORM}/")
  message(FATAL_ERROR "unic doesn't support unknown platform ${UNIC_TARGET_PLATFORM}")
endif ()

include(${PROJECT_SOURCE_DIR}/platforms/${UNIC_TARGET_PLATFORM}/platform.cmake)

set(UNIC_INCLUDE_DIR
  ${PROJECT_SOURCE_DIR}/include
  )

set(UNIC_INCLUDE_DIRS
  ${UNIC_INCLUDE_DIR}
  ${PROJECT_SOURCE_DIR}/src
  ${CMAKE_BINARY_DIR}
  )

set(UNIC_PUBLIC_HDRS
  ${UNIC_INCLUDE_DIR}/unic/atomic.h
  ${UNIC_INCLUDE_DIR}/unic/types.h
  ${UNIC_INCLUDE_DIR}/unic/macros.h
  ${UNIC_INCLUDE_DIR}/unic/cc.h
  ${UNIC_INCLUDE_DIR}/unic/arch.h
  ${UNIC_INCLUDE_DIR}/unic/os.h
  ${UNIC_INCLUDE_DIR}/unic/condvar.h
  ${UNIC_INCLUDE_DIR}/unic/hash.h
  ${UNIC_INCLUDE_DIR}/unic/err.h
  ${UNIC_INCLUDE_DIR}/unic/dir.h
  ${UNIC_INCLUDE_DIR}/unic/file.h
  ${UNIC_INCLUDE_DIR}/unic/htable.h
  ${UNIC_INCLUDE_DIR}/unic/inifile.h
  ${UNIC_INCLUDE_DIR}/unic.h
  ${UNIC_INCLUDE_DIR}/unic/dl.h
  ${UNIC_INCLUDE_DIR}/unic/list.h
  ${UNIC_INCLUDE_DIR}/unic/main.h
  ${UNIC_INCLUDE_DIR}/unic/mem.h
  ${UNIC_INCLUDE_DIR}/unic/mutex.h
  ${UNIC_INCLUDE_DIR}/unic/process.h
  ${UNIC_INCLUDE_DIR}/unic/rwlock.h
  ${UNIC_INCLUDE_DIR}/unic/sema.h
  ${UNIC_INCLUDE_DIR}/unic/shm.h
  ${UNIC_INCLUDE_DIR}/unic/shmbuf.h
  ${UNIC_INCLUDE_DIR}/unic/socket.h
  ${UNIC_INCLUDE_DIR}/unic/socketaddr.h
  ${UNIC_INCLUDE_DIR}/unic/spinlock.h
  ${UNIC_INCLUDE_DIR}/unic/string.h
  ${UNIC_INCLUDE_DIR}/unic/profiler.h
  ${UNIC_INCLUDE_DIR}/unic/tree.h
  ${UNIC_INCLUDE_DIR}/unic/uthread.h
  ${UNIC_INCLUDE_DIR}/unic/config.h
  )

set(UNIC_PRIVATE_HDRS
  hash-gost3411.h
  hash-md5.h
  hash-sha1.h
  hash-sha2-256.h
  hash-sha2-512.h
  hash-sha3.h
  err-private.h
  unic-private.h
  sysclose-private.h
  profiler-private.h
  tree-avl.h
  tree-bst.h
  tree-rb.h
  tree-private.h
  uthread-private.h
  )

set(UNIC_SRCS
  hash.c
  hash-gost3411.c
  hash-md5.c
  hash-sha1.c
  hash-sha2-256.c
  hash-sha2-512.c
  hash-sha3.c
  dir.c
  err.c
  file.c
  htable.c
  inifile.c
  list.c
  main.c
  mem.c
  process.c
  shmbuf.c
  socket.c
  socketaddr.c
  string.c
  profiler.c
  tree.c
  tree-avl.c
  tree-bst.c
  tree-rb.c
  uthread.c
  )

if (UNIC_NATIVE_WINDOWS)
  set(UNIC_CLOSE_MODEL win)
elseif (UNIC_TARGET_OS STREQUAL darwin)
  set(UNIC_CLOSE_MODEL darwin)
else ()
  set(UNIC_CLOSE_MODEL unix)
endif ()

if (UNIC_THREAD_MODEL STREQUAL "")
  set(UNIC_THREAD_MODEL none)
endif ()

if (UNIC_IPC_MODEL STREQUAL "")
  set(UNIC_IPC_MODEL none)
endif ()

if (UNIC_TIME_PROFILER_MODEL STREQUAL "")
  set(UNIC_TIME_PROFILER_MODEL generic)
endif ()

if (UNIC_DIR_MODEL STREQUAL "")
  set(UNIC_DIR_MODEL none)
endif ()

if (UNIC_LIBRARYLOADER_MODEL STREQUAL "")
  set(UNIC_LIBRARYLOADER_MODEL none)
endif ()

set(UNIC_PLATFORM_SRCS
  condvar-${UNIC_THREAD_MODEL}.c
  mutex-${UNIC_THREAD_MODEL}.c
  sema-${UNIC_IPC_MODEL}.c
  shm-${UNIC_IPC_MODEL}.c
  sysclose-${UNIC_CLOSE_MODEL}.c
  uthread-${UNIC_THREAD_MODEL}.c
  profiler-${UNIC_TIME_PROFILER_MODEL}.c
  dir-${UNIC_DIR_MODEL}.c
  dl-${UNIC_LIBRARYLOADER_MODEL}.c
  )

if (NOT UNIC_IPC_MODEL STREQUAL none)
  list(APPEND UNIC_PRIVATE_HDRS ipc-private.h)
  list(APPEND UNIC_SRCS ipc.c)
endif ()

# Save compiler flags
set(SAVED_CMAKE_REQUIRED_DEFINITIONS ${CMAKE_REQUIRED_DEFINITIONS})
set(SAVED_CMAKE_REQUIRED_LIBRARIES ${CMAKE_REQUIRED_LIBRARIES})
set(SAVED_CMAKE_REQUIRED_FLAGS ${SAVED_CMAKE_REQUIRED_FLAGS})

list(APPEND CMAKE_REQUIRED_DEFINITIONS ${UNIC_PLATFORM_DEFINES})
list(APPEND CMAKE_REQUIRED_LIBRARIES ${UNIC_PLATFORM_LINK_LIBRARIES})
list(APPEND CMAKE_REQUIRED_FLAGS ${UNIC_PLATFORM_LDFLAGS})

set(UNIC_ATOMIC_LOCK_FREE no)

check_c_source_compiles(
  "int main () {
    int i, tmp_int = 0;
    __atomic_store_4 (&i, 0, __ATOMIC_SEQ_CST);
    __atomic_compare_exchange_n (&i, &tmp_int, 1, 0, __ATOMIC_SEQ_CST, __ATOMIC_SEQ_CST);
    __atomic_fetch_add (&i, 1, __ATOMIC_SEQ_CST);
    return 0;
  }"
  UNIC_HAVE_ATOMIC_INTRIN
)
check_c_source_compiles(
  "int main () {
    int i;
    __sync_synchronize ();
    __sync_bool_compare_and_swap (&i, 0, 1);
    __sync_fetch_and_add (&i, 1);
    return 0;
  }"
  UNIC_HAVE_SYNC_INTRIN
)

if (NOT UNIC_ATOMIC_MODEL)
  message(STATUS "Checking for lock-free atomic intrinsics")
    
  # GCC __atomic* intrinsics
  if (UNIC_HAVE_ATOMIC_INTRIN)
    set(UNIC_ATOMIC_LOCK_FREE yes)
    set(UNIC_ATOMIC_MODEL "c11")
  endif ()

  # GCC __sync* intinsics
  if (NOT UNIC_ATOMIC_LOCK_FREE)
    if (UNIC_HAVE_SYNC_INTRIN)
      set(UNIC_ATOMIC_LOCK_FREE yes)
      set(UNIC_ATOMIC_MODEL "sync")
    endif ()
  endif ()

  if (NOT UNIC_ATOMIC_LOCK_FREE)
    if (UNIC_TARGET_OS STREQUAL windows AND (UNIC_C_COMPILER STREQUAL borland OR
      UNIC_C_COMPILER STREQUAL watcom OR
      UNIC_C_COMPILER STREQUAL icc OR
      UNIC_C_COMPILER STREQUAL msvc))
      set(UNIC_ATOMIC_LOCK_FREE true)
      set(UNIC_ATOMIC_MODEL "win")
    else ()
      set(UNIC_ATOMIC_LOCK_FREE no)
      set(UNIC_ATOMIC_MODEL "sim")
    endif ()
  endif ()

  if (UNIC_ATOMIC_LOCK_FREE)
    message(STATUS "Checking for lock-free atomic intrinsics - works")
  else ()
    message(STATUS "Checking for lock-free atomic intrinsics - not works")
  endif ()
endif ()

list(APPEND UNIC_SRCS
  atomic.c
  spinlock-${UNIC_ATOMIC_MODEL}.c
  )

if (EXISTS UNIC_CONFIG_FILE)
  file(REMOVE ${UNIC_CONFIG_FILE})
endif ()

test_big_endian(UNIC_IS_BIGENDIAN)

check_include_file("arpa/inet.h" UNIC_HAVE_ARPA_INET_H)
check_include_file("assert.h" UNIC_HAVE_ASSERT_H)
check_include_file("atheos/semaphore.h" UNIC_HAVE_ATHEOS_SEMAPHORE_H)
check_include_file("atheos/sysinfo.h" UNIC_HAVE_ATHEOS_SYSINFO_H)
check_include_file("atheos/threads.h" UNIC_HAVE_ATHEOS_THREADS_H)
check_include_file("atheos/tld.h" UNIC_HAVE_ATHEOS_TLD_H)
check_include_file("be/kernel/OS.h" UNIC_HAVE_BE_KERNEL_OS_H)
check_include_file("be/kernel/image.h" UNIC_HAVE_BE_KERNEL_IMAGE_H)
check_include_file("builtins.h" UNIC_HAVE_BUILTINS_H)
check_include_file("ctype.h" UNIC_HAVE_CTYPE_H)
check_include_file("dirent.h" UNIC_HAVE_DIRENT_H)
check_include_file("dl.h" UNIC_HAVE_DL_H)
check_include_file("dlfcn.h" UNIC_HAVE_DLFCN_H)
check_include_file("efndef.h" UNIC_HAVE_EFNDEF_H)
check_include_file("errno.h" UNIC_HAVE_ERRNO_H)
check_include_file("fcntl.h" UNIC_HAVE_FCNTL_H)
check_include_file("float.h" UNIC_HAVE_FLOAT_H)
check_include_file("iledef.h" UNIC_HAVE_ILEDEF_H)
check_include_file("intrin.h" UNIC_HAVE_INTRIN_H)
check_include_file("inttypes.h" UNIC_HAVE_INTTYPES_H)
check_include_file("iosbdef.h" UNIC_HAVE_IOSBDEF_H)
check_include_file("kernel/OS.h" UNIC_HAVE_KERNEL_OS_H)
check_include_file("kernel/scheduler.h" UNIC_HAVE_KERNEL_SCHEDULER_H)
check_include_file("lib$routines.h" UNIC_HAVE_LIB$ROUTINES_H)
check_include_file("limits.h" UNIC_HAVE_LIMITS_H)
check_include_file("mach/mach_time.h" UNIC_HAVE_MACH_MACH_TIME_H)
check_include_file("machine/builtins.h" UNIC_HAVE_MACHINE_BUILTINS_H)
check_include_file("math.h" UNIC_HAVE_MATH_H)
check_include_file("memory.h" UNIC_HAVE_MEMORY_H)
check_include_file("netdb.h" UNIC_HAVE_NETDB_H)
check_include_file("netinet/in.h" UNIC_HAVE_NETINET_IN_H)
check_include_file("os2.h" UNIC_HAVE_OS2_H)
check_include_file("pmacros.h" UNIC_HAVE_PMACROS_H)
check_include_file("process.h" UNIC_HAVE_PROCESS_H)
check_include_file("pthread.h" UNIC_HAVE_PTHREAD_H)
check_include_file("sched.h" UNIC_HAVE_SCHED_H)
check_include_file("semaphore.h" UNIC_HAVE_SEMAPHORE_H)
check_include_file("signal.h" UNIC_HAVE_SIGNAL_H)
check_include_file("ssdef.h" UNIC_HAVE_SSDEF_H)
check_include_file("starlet.h" UNIC_HAVE_STARLET_H)
check_include_file("stdarg.h" UNIC_HAVE_STDARG_H)
check_include_file("stdatomic.h" UNIC_HAVE_STDATOMIC_H)
check_include_file("stdbool.h" UNIC_HAVE_STDBOOL_H)
check_include_file("stddef.h" UNIC_HAVE_STDDEF_H)
check_include_file("stdint.h" UNIC_HAVE_STDINT_H)
check_include_file("stdio.h" UNIC_HAVE_STDIO_H)
check_include_file("stdlib.h" UNIC_HAVE_STDLIB_H)
check_include_file("string.h" UNIC_HAVE_STRING_H)
check_include_file("stropts.h" UNIC_HAVE_STROPTS_H)
check_include_file("stsdef.h" UNIC_HAVE_STSDEF_H)
check_include_file("support/TLS.h" UNIC_HAVE_SUPPORT_TLS_H)
check_include_file("syidef.h" UNIC_HAVE_SYIDEF_H)
check_include_file("synch.h" UNIC_HAVE_SYNCH_H)
check_include_file("sys/ipc.h" UNIC_HAVE_SYS_IPC_H)
check_include_file("sys/mman.h" UNIC_HAVE_SYS_MMAN_H)
check_include_file("sys/param.h" UNIC_HAVE_SYS_PARAM_H)
check_include_file("sys/poll.h" UNIC_HAVE_SYS_POLL_H)
check_include_file("sys/pstat.h" UNIC_HAVE_SYS_PSTAT_H)
check_include_file("sys/select.h" UNIC_HAVE_SYS_SELECT_H)
check_include_file("sys/sem.h" UNIC_HAVE_SYS_SEM_H)
check_include_file("sys/shm.h" UNIC_HAVE_SYS_SHM_H)
check_include_file("sys/socket.h" UNIC_HAVE_SYS_SOCKET_H)
check_include_file("sys/stat.h" UNIC_HAVE_SYS_STAT_H)
check_include_file("sys/sysctl.h" UNIC_HAVE_SYS_SYSCTL_H)
check_include_file("sys/syspage.h" UNIC_HAVE_SYS_SYSPAGE_H)
check_include_file("sys/time.h" UNIC_HAVE_SYS_TIME_H)
check_include_file("sys/types.h" UNIC_HAVE_SYS_TYPES_H)
check_include_file("sys/utsname.h" UNIC_HAVE_SYS_UTSNAME_H)
check_include_file("thread.h" UNIC_HAVE_THREAD_H)
check_include_file("time.h" UNIC_HAVE_TIME_H)
check_include_file("tis.h" UNIC_HAVE_TIS_H)
check_include_file("unistd.h" UNIC_HAVE_UNISTD_H)
check_include_file("values.h" UNIC_HAVE_VALUES_H)
check_include_file("windows.h" UNIC_HAVE_WINDOWS_H)
check_include_file("winsock2.h" UNIC_HAVE_WINSOCK2_H)
check_include_file("ws2tcpip.h" UNIC_HAVE_WS2TCPIP_H)

if (UNIC_HAVE_FLOAT_H)
  set(UNIC_NEED_FLOAT_H TRUE)
  set(UNIC_FLOAT_MIN FLT_MIN)
  set(UNIC_FLOAT_MAX FLT_MAX)
  set(UNIC_DOUBLE_MIN DBL_MIN)
  set(UNIC_DOUBLE_MAX DBL_MAX)
elseif (UNIC_HAVE_VALUES_H)
  set(UNIC_NEED_VALUES_H TRUE)
  set(UNIC_FLOAT_MIN MINFLOAT)
  set(UNIC_FLOAT_MAX MAXFLOAT)
  set(UNIC_DOUBLE_MIN MINDOUBLE)
  set(UNIC_DOUBLE_MAX MAXDOUBLE)
endif ()

if (UNIC_HAVE_LIMITS_H)
  set(UNIC_NEED_LIMITS_H TRUE)
  set(UNIC_SHORT_MIN SHRT_MIN)
  set(UNIC_SHORT_MAX SHRT_MAX)
  set(UNIC_USHORT_MAX USHRT_MAX)
  set(UNIC_INT_MIN INT_MIN)
  set(UNIC_INT_MAX INT_MAX)
  set(UNIC_UINT_MAX UINT_MAX)
  set(UNIC_LONG_MIN LONG_MIN)
  set(UNIC_LONG_MAX LONG_MAX)
  set(UNIC_ULONG_MAX ULONG_MAX)
elseif (UNIC_HAVE_VALUES_H)
  set(UNIC_NEED_VALUES_H TRUE)
  set(UNIC_SHORT_MIN MINSHORT)
  set(UNIC_SHORT_MAX MAXSHORT)
  set(UNIC_USHORT_MAX "(((pushort) U_MAXSHORT) * 2 + 1)")
  set(UNIC_INT_MIN MININT)
  set(UNIC_INT_MAX MAXINT)
  set(UNIC_UINT_MAX "(((puint) U_MAXINT) * 2 + 1)")
  set(UNIC_LONG_MIN MINLONG)
  set(UNIC_LONG_MAX MAXLONG)
  set(UNIC_ULONG_MAX "(((pulong) U_MAXLONG) * 2 + 1)")
endif ()

check_type_size("size_t" UNIC_SIZEOF_SIZE_T)
check_type_size("long" UNIC_SIZEOF_LONG)

if (UNIC_NATIVE_WINDOWS)
  set(UNIC_NEED_WINDOWS_H true)
endif ()

configure_file(${UNIC_INCLUDE_DIR}/unic/config.h.in ${UNIC_INCLUDE_DIR}/unic/config.h)

if (NOT UNIC_NATIVE_WINDOWS)
  # Check for anonymous mmap()
  message(STATUS "Checking whether mmap has anonymous mapping")

  check_c_source_compiles(
    "#include <sys/types.h>
				  #include <sys/mman.h>
				 int main () {
					mmap (0, 1024, PROT_READ | PROT_WRITE, MAP_ANON | MAP_PRIVATE, -1, 0);
					return 0;
				 }"
    UNIC_MMAP_HAS_MAP_ANON
  )

  check_c_source_compiles(
    "#include <sys/types.h>
				  #include <sys/mman.h>
				 int main () {
					mmap (0, 1024, PROT_READ | PROT_WRITE, MAP_ANONYMOUS | MAP_PRIVATE, -1, 0);
					return 0;
				 }"
    UNIC_MMAP_HAS_MAP_ANONYMOUS
  )

  if (UNIC_MMAP_HAS_MAP_ANONYMOUS OR UNIC_MMAP_HAS_MAP_ANON)
    message(STATUS "Checking whether mmap has anonymous mapping - yes")
  else ()
    message(STATUS "Checking whether mmap has anonymous mapping - no")
  endif ()

  if (UNIC_MMAP_HAS_MAP_ANONYMOUS)
    list(APPEND UNIC_COMPILE_DEFS -DUNIC_MMAP_HAS_MAP_ANONYMOUS)
  elseif (UNIC_MMAP_HAS_MAP_ANON)
    list(APPEND UNIC_COMPILE_DEFS -DUNIC_MMAP_HAS_MAP_ANON)
  endif ()

  # Check for clock_nanosleep() call
  message(STATUS "Checking whether clock_nanosleep() presents")

  check_c_source_compiles(
    "#include <time.h>
				 int main () {
					struct timespec time_sp = {0, 500000000L};
					clock_nanosleep (CLOCK_MONOTONIC, 0, &time_sp, NULL);
					return 0;
				 }"
    UNIC_HAS_CLOCKNANOSLEEP
  )

  if (UNIC_HAS_CLOCKNANOSLEEP)
    message(STATUS "Checking whether clock_nanosleep() presents - yes")
  else ()
    message(STATUS "Checking whether clock_nanosleep() presents - no")
  endif ()

  # Check for nanosleep() call
  message(STATUS "Checking whether nanosleep() presents")

  check_c_source_compiles(
    "#include <time.h>
				 int main () {
					struct timespec time_sp = {0, 500000000L};
					nanosleep (&time_sp, NULL);
					return 0;
				 }"
    UNIC_HAS_NANOSLEEP
  )

  if (UNIC_HAS_NANOSLEEP)
    message(STATUS "Checking whether nanosleep() presents - yes")
    list(APPEND UNIC_COMPILE_DEFS -DUNIC_HAS_NANOSLEEP)
  else ()
    message(STATUS "Checking whether nanosleep() presents - no")
  endif ()

  # Prefere clock_nanosleep() over nanosleep() for power consumption
  if (UNIC_HAS_CLOCKNANOSLEEP)
    list(APPEND UNIC_COMPILE_DEFS -DUNIC_HAS_CLOCKNANOSLEEP)
  elseif (UNIC_HAS_NANOSLEEP)
    list(APPEND UNIC_COMPILE_DEFS -DUNIC_HAS_NANOSLEEP)
  endif ()

  # Check for getaddrinfo() call
  message(STATUS "Checking whether getaddrinfo() presents")

  check_c_source_compiles(
    "#include <sys/socket.h>
				  #include <netdb.h>
				 int main () {
					getaddrinfo (0, 0, 0, 0);
					freeaddrinfo (0);

					return 0;
				 }"
    UNIC_HAS_GETADDRINFO
  )

  if (UNIC_HAS_GETADDRINFO)
    message(STATUS "Checking whether getaddrinfo() presents - yes")
    list(APPEND UNIC_COMPILE_DEFS -DUNIC_HAS_GETADDRINFO)
  else ()
    message(STATUS "Checking whether getaddrinfo() presents - no")
  endif ()
endif ()

if (NOT UNIC_RWLOCK_MODEL)
  set(UNIC_RWLOCK_MODEL ${UNIC_THREAD_MODEL})
else ()
  if (NOT UNIC_RWLOCK_MODEL STREQUAL general AND
    NOT UNIC_RWLOCK_MODEL STREQUAL none)
    message(WARNING "It's not recommended to mix threading and read-write lock models")
  endif ()
endif ()

if (UNIC_THREAD_MODEL STREQUAL posix)
  # Some systems only need the difinition to be available
  if (UNIC_TARGET_OS MATCHES "darwin|aix")
    set(UNIC_SCHED_CHECK "defined (_POSIX_THREAD_PRIORITY_SCHEDULING)")
  else ()
    set(UNIC_SCHED_CHECK "defined (_POSIX_THREAD_PRIORITY_SCHEDULING) && (_POSIX_THREAD_PRIORITY_SCHEDULING - 0 >= 0)")
  endif ()

  # Check for thread scheduling
  message(STATUS "Checking whether POSIX thread scheduling presents")

  check_c_source_compiles(
    "#include <unistd.h>
				  #include <pthread.h>
				  #include <sched.h>

				 int main () {
				 	#if ${UNIC_SCHED_CHECK}
					  sched_get_priority_min (0);
					  sched_get_priority_max (0);
					#else
					  stop_compile_here
					#endif
					return 0;
				 }"
    UNIC_HAS_POSIX_SCHEDULING
  )

  if (UNIC_HAS_POSIX_SCHEDULING)
    message(STATUS "Checking whether POSIX thread scheduling presents - yes")
    list(APPEND UNIC_COMPILE_DEFS -DUNIC_HAS_POSIX_SCHEDULING)
  else ()
    message(STATUS "Checking whether POSIX thread scheduling presents - no")
  endif ()

  # Check for thread stack size
  message(STATUS "Checking whether POSIX thread stack size is supported")

  check_c_source_compiles(
    "#include <pthread.h>

				 int main () {
					pthread_attr_t attr;

					pthread_attr_setstacksize (&attr, 0);
					return 0;
				 }"
    UNIC_HAS_POSIX_STACKSIZE
  )

  if (UNIC_HAS_POSIX_STACKSIZE)
    message(STATUS "Checking whether POSIX thread stack size is supported - yes")
    list(APPEND UNIC_COMPILE_DEFS -DUNIC_HAS_POSIX_STACKSIZE)
  else ()
    message(STATUS "Checking whether POSIX thread stack size is supported - no")
  endif ()
endif ()

# Some platforms may have headers, but lack actual implementation,
# thus we can let platform to override read-write lock model with
# general implementation
if (UNIC_THREAD_MODEL STREQUAL posix AND
  UNIC_RWLOCK_MODEL STREQUAL posix)
  # Check for read-write lock support
  message(STATUS "Checking whether POSIX read-write locks are supported")

  check_c_source_compiles(
    "#include <pthread.h>

				 int main () {
					pthread_rwlock_t rwl;

					pthread_rwlock_init (&rwl, 0);
					pthread_rwlock_destroy (&rwl);
					return 0;
				 }"
    UNIC_HAS_POSIX_RWLOCK
  )

  if (UNIC_HAS_POSIX_RWLOCK)
    message(STATUS "Checking whether POSIX read-write locks are supported - yes")
  else ()
    message(STATUS "Checking whether POSIX read-write locks are supported - no")
    set(UNIC_RWLOCK_MODEL "general")
  endif ()
endif ()

list(APPEND UNIC_PLATFORM_SRCS rwlock-${UNIC_RWLOCK_MODEL}.c)

if (UNIC_NATIVE_WINDOWS)
  set(UNIC_SOCKET_INCLUDES "#include <winsock2.h>
				      #include <ws2tcpip.h>
				      #include <windows.h>")
else ()
  set(UNIC_SOCKET_INCLUDES "#include <sys/types.h>
				      #include <sys/socket.h>
				      #include <netinet/in.h>")
endif ()

# Check for socklen_t definition
message(STATUS "Checking whether socklen_t is defined")

check_c_source_compiles(
  "${UNIC_SOCKET_INCLUDES}
			  int main () {
				socklen_t len = sizeof (socklen_t);
				return len > 0 ? 0 : -1;
			  }"
  UNIC_HAS_SOCKLEN_T
)

if (UNIC_HAS_SOCKLEN_T)
  message(STATUS "Checking whether socklen_t is defined - yes")
  list(APPEND UNIC_COMPILE_DEFS -DUNIC_HAS_SOCKLEN_T)
else ()
  message(STATUS "Checking whether socklen_t is defined - no")
endif ()

# Check for sockaddr_storage structure
message(STATUS "Checking whether struct sockaddr_storage is defined")

check_c_source_compiles(
  "${UNIC_SOCKET_INCLUDES}
			  int main () {
				struct sockaddr_storage sock_addr;
				sock_addr.ss_family = AF_INET;

				return 0;
			  }"
  UNIC_HAS_SOCKADDR_STORAGE
)

if (UNIC_HAS_SOCKADDR_STORAGE)
  message(STATUS "Checking whether struct sockaddr_storage is defined - yes")
  list(APPEND UNIC_COMPILE_DEFS -DUNIC_HAS_SOCKADDR_STORAGE)
else ()
  message(STATUS "Checking whether struct sockaddr_storage is defined - no")
endif ()

# Check sa_len field in struct sockaddr
message(STATUS "Checking whether struct sockaddr has sa_len")

check_c_source_compiles(
  "${UNIC_SOCKET_INCLUDES}
			  int main () {
				struct sockaddr sock_addr;
				sock_addr.sa_len = 0;

				return 0;
			  }"
  UNIC_SOCKADDR_HAS_SA_LEN
)

if (UNIC_SOCKADDR_HAS_SA_LEN)
  message(STATUS "Checking whether struct sockaddr has sa_len - yes")
  list(APPEND UNIC_COMPILE_DEFS -DUNIC_SOCKADDR_HAS_SA_LEN)
else ()
  message(STATUS "Checking whether struct sockaddr has sa_len - no")
endif ()

# Check sin6_scope_id field in struct sockaddr_in6
message(STATUS "Checking whether struct sockaddr_in6 has sin6_scope_id")

check_c_source_compiles(
  "${UNIC_SOCKET_INCLUDES}
			  int main () {
				struct sockaddr_in6 sock_addr;
				sock_addr.sin6_scope_id = 0;

				return 0;
			  }"
  UNIC_SOCKADDR_IN6_HAS_SCOPEID
)

if (UNIC_SOCKADDR_IN6_HAS_SCOPEID)
  message(STATUS "Checking whether struct sockaddr_in6 has sin6_scope_id - yes")
  list(APPEND UNIC_COMPILE_DEFS -DUNIC_SOCKADDR_IN6_HAS_SCOPEID)
else ()
  message(STATUS "Checking whether struct sockaddr_in6 has sin6_scope_id - no")
endif ()

# Check sin6_flowinfo field in struct sockaddr_in6
message(STATUS "Checking whether struct sockaddr_in6 has sin6_flowinfo")

check_c_source_compiles(
  "${UNIC_SOCKET_INCLUDES}
			  int main () {
				struct sockaddr_in6 sock_addr;
				sock_addr.sin6_flowinfo = 0;

				return 0;
			  }"
  UNIC_SOCKADDR_IN6_HAS_FLOWINFO
)

if (UNIC_SOCKADDR_IN6_HAS_FLOWINFO)
  message(STATUS "Checking whether struct sockaddr_in6 has sin6_flowinfo - yes")
  list(APPEND UNIC_COMPILE_DEFS -DUNIC_SOCKADDR_IN6_HAS_FLOWINFO)
else ()
  message(STATUS "Checking whether struct sockaddr_in6 has sin6_flowinfo - no")
endif ()

# Check sa_family_t type size in struct sockaddr
set(CMAKE_EXTRA_INCLUDE_FILES_OLD ${CMAKE_EXTRA_INCLUDE_FILES})

if (UNIC_NATIVE_WINDOWS)
  set(CMAKE_EXTRA_INCLUDE_FILES
    winsock2.h
    ws2tcpip.h
    windows.h
    )
else ()
  set(CMAKE_EXTRA_INCLUDE_FILES
    sys/types.h
    sys/socket.h
    netinet/in.h
    )
endif ()

check_type_size("((struct sockaddr *) 0)->sa_family" UNIC_SIZEOF_SAFAMILY_T LANGUAGE C)

set(CMAKE_EXTRA_INCLUDE_FILES ${CMAKE_EXTRA_INCLUDE_FILES_OLD})

# Check for lldiv() call
message(STATUS "Checking whether lldiv() presents")

check_c_source_compiles(
  "#define __USE_ISOC99
			  #include <stdlib.h>
			  int main () {
				lldiv_t res = lldiv (100LL, 13LL);
				res.quot = 0;
				res.rem = 0;

				return 0;
			  }"
  UNIC_HAS_LLDIV
)

if (UNIC_HAS_LLDIV)
  message(STATUS "Checking whether lldiv() presents - yes")
  list(APPEND UNIC_COMPILE_DEFS -DUNIC_HAS_LLDIV)
else ()
  message(STATUS "Checking whether lldiv() presents - no")
endif ()

# Symbols visibility attributes
if (UNIC_VISIBILITY)
  message(STATUS "Checking whether compiler supports visibility")
  unic_detect_visibility(UNIC_VISIBILITY_CFLAGS UNIC_VISIBILITY_LDFLAGS)

  if (NOT UNIC_VISIBILITY_CFLAGS AND NOT UNIC_VISIBILITY_LDFLAGS)
    message(STATUS "Checking whether compiler supports visibility - no")
    set(UNIC_VISIBILITY OFF)
  else ()
    message(STATUS "Checking whether compiler supports visibility - yes")

    if (UNIC_VISIBILITY_CFLAGS)
      list(APPEND UNIC_PLATFORM_CFLAGS ${UNIC_VISIBILITY_CFLAGS})
    endif ()

    if (UNIC_VISIBILITY_LDFLAGS)
      list(APPEND UNIC_PLATFORM_LDFLAGS ${UNIC_VISIBILITY_LDFLAGS})
    endif ()
  endif ()
endif ()

# Restore compiler flags
set(CMAKE_REQUIRED_DEFINITIONS ${SAVED_CMAKE_REQUIRED_DEFINITIONS})
set(CMAKE_REQUIRED_LIBRARIES ${SAVED_CMAKE_REQUIRED_LIBRARIES})
set(SAVED_CMAKE_REQUIRED_FLAGS ${SAVED_CMAKE_REQUIRED_FLAGS})

# Disable useless warnings
if (MSVC)
  list(APPEND UNIC_COMPILE_DEFS -D_CRT_SECURE_NO_WARNINGS)
  list(APPEND UNIC_COMPILE_DEFS -D_WINSOCK_DEPRECATED_NO_WARNINGS)
endif ()

# Prepare compile definitions
list(APPEND UNIC_COMPILE_DEFS -DUNIC_COMPILATION)

if (UNIC_PLATFORM_DEFINES)
  list(APPEND UNIC_COMPILE_DEFS ${UNIC_PLATFORM_DEFINES})
endif ()

# Add targets
add_library(unic SHARED ${UNIC_SRCS} ${UNIC_PLATFORM_SRCS} ${UNIC_PUBLIC_HDRS} ${UNIC_PRIVATE_HDRS})

if (UNIC_BUILD_STATIC)
  add_library(unicstatic STATIC ${UNIC_SRCS} ${UNIC_PLATFORM_SRCS} ${UNIC_PUBLIC_HDRS} ${UNIC_PRIVATE_HDRS})
endif ()

# Add include directories
if (COMMAND target_include_directories)
  target_include_directories(unic PUBLIC ${UNIC_INCLUDE_DIRS})

  if (UNIC_BUILD_STATIC)
    target_include_directories(unicstatic PUBLIC ${UNIC_INCLUDE_DIRS})
  endif ()
else ()
  include_directories(${UNIC_INCLUDE_DIRS})
endif ()

# Add compile definitions
if (COMMAND target_compile_definitions)
  target_compile_definitions(unic PRIVATE ${UNIC_COMPILE_DEFS})

  if (UNIC_BUILD_STATIC)
    target_compile_definitions(unicstatic PRIVATE ${UNIC_COMPILE_DEFS} -DUNIC_STATIC_COMPILATION)
  endif ()
else ()
  add_definitions(${UNIC_COMPILE_DEFS})
endif ()

set_target_properties(unic PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${OUTPUT_DIR})
set_target_properties(unic PROPERTIES LIBRARY_OUTPUT_DIRECTORY ${OUTPUT_DIR})
set_target_properties(unic PROPERTIES ARCHIVE_OUTPUT_DIRECTORY ${OUTPUT_DIR})

if (NOT UNIC_TARGET_OS STREQUAL os2)
  set_target_properties(unic PROPERTIES SOVERSION ${UNIC_VERSION})
endif ()

if (UNIC_BUILD_STATIC)
  set_target_properties(unicstatic PROPERTIES ARCHIVE_OUTPUT_DIRECTORY ${OUTPUT_DIR})

  if (NOT UNIC_TARGET_OS STREQUAL os2)
    set_target_properties(unicstatic PROPERTIES SOVERSION ${UNIC_VERSION})
  endif ()
endif ()

if (UNIC_PLATFORM_CFLAGS)
  set_target_properties(unic PROPERTIES COMPILE_FLAGS "${UNIC_PLATFORM_CFLAGS}")

  if (UNIC_BUILD_STATIC)
    set_target_properties(unicstatic PROPERTIES COMPILE_FLAGS "${UNIC_PLATFORM_CFLAGS}")
  endif ()
endif ()

if (UNIC_PLATFORM_LDFLAGS)
  set_target_properties(unic PROPERTIES LINK_FLAGS "${UNIC_PLATFORM_LDFLAGS}")

  if (UNIC_BUILD_STATIC)
    set_target_properties(unicstatic PROPERTIES LINK_FLAGS "${UNIC_PLATFORM_LDFLAGS}")
  endif ()
endif ()

target_link_libraries(unic ${UNIC_PLATFORM_LINK_LIBRARIES})

if (UNIC_BUILD_STATIC)
  target_link_libraries(unicstatic ${UNIC_PLATFORM_LINK_LIBRARIES})
endif ()

if (UNIC_BUILD_STATIC)
  set(UNIC_INSTALL_TARGETS unic unicstatic)
else ()
  set(UNIC_INSTALL_TARGETS unic)
endif ()

if (UNIC_NATIVE_WINDOWS)
  install(TARGETS ${UNIC_INSTALL_TARGETS}
    DESTINATION lib
    RUNTIME DESTINATION lib
    COMPONENT Core
    )

  if (NOT DEFINED CMAKE_INSTALL_SYSTEM_RUNTIME_LIBS_NO_WARNINGS)
    set(CMAKE_INSTALL_SYSTEM_RUNTIME_LIBS_NO_WARNINGS TRUE)
  endif ()

  set(CMAKE_INSTALL_SYSTEM_RUNTIME_LIBS_SKIP TRUE)
  include(InstallRequiredSystemLibraries)

  install(PROGRAMS ${CMAKE_INSTALL_SYSTEM_RUNTIME_LIBS}
    DESTINATION lib
    COMPONENT Core
    )
endif ()

# Prepare installation dirs
if (NOT CMAKE_INSTALL_LIBDIR)
  set(CMAKE_INSTALL_LIBDIR "lib")
endif ()

if (NOT CMAKE_INSTALL_INCLUDEDIR)
  set(CMAKE_INSTALL_INCLUDEDIR "include")
endif ()

install(TARGETS ${UNIC_INSTALL_TARGETS} EXPORT unic-targets
  DESTINATION ${CMAKE_INSTALL_LIBDIR}
  LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
  COMPONENT Core
  )
install(TARGETS ${UNIC_INSTALL_TARGETS}
  DESTINATION ${CMAKE_INSTALL_LIBDIR}
  ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
  COMPONENT Core
  )
install(FILES
  ${UNIC_PUBLIC_HDRS}
  DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/unic
  COMPONENT Core
  )

# Print summary
set(UNIC_SUMMARY
  "
	== Build configuration ==

	Platfrom:               ${UNIC_TARGET_OS}
	Compiler:               ${UNIC_C_COMPILER}
	Address model:          ${UNIC_OS_BITS} bit

	Thread model:           ${UNIC_THREAD_MODEL}
	RW lock model:          ${UNIC_RWLOCK_MODEL}
	IPC model:              ${UNIC_IPC_MODEL}
	DIR model:              ${UNIC_DIR_MODEL}
	Library loader model:   ${UNIC_LIBRARYLOADER_MODEL}
	Time profiler model:    ${UNIC_TIME_PROFILER_MODEL}
	Atomic model:           ${UNIC_ATOMIC_MODEL}

	Platform defines:       ${UNIC_PLATFORM_DEFINES}
	Platform CFLAGS:        ${UNIC_PLATFORM_CFLAGS}
	Platform LDFLAGS:       ${UNIC_PLATFORM_LDFLAGS}
	Platform libraries:     ${UNIC_PLATFORM_LINK_LIBRARIES}

	Build static library:   ${UNIC_BUILD_STATIC}
	Coverage support:       ${COVERAGE}
	Visibility:             ${UNIC_VISIBILITY}

")

message("${UNIC_SUMMARY}")
