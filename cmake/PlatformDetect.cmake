function (unic_detect_c_compiler result)
        # Get target system OS
        string (TOLOWER ${CMAKE_SYSTEM_NAME} UNIC_TARGET_OS)

        # Detect C compiler by it's ID
        if (CMAKE_C_COMPILER_ID STREQUAL GNU)
                set (UNIC_C_COMPILER gcc)
        elseif (CMAKE_C_COMPILER_ID STREQUAL MSVC)
                set (UNIC_C_COMPILER msvc)
        else()
                string (TOLOWER ${CMAKE_C_COMPILER_ID} UNIC_C_COMPILER)
        endif()

        # Fix gcc -> qcc naming on QNX 6
        if ((UNIC_TARGET_OS STREQUAL qnx) AND (UNIC_C_COMPILER STREQUAL gcc))
                set (UNIC_C_COMPILER qcc)
        endif()

        # Rename intel -> icc
        if (UNIC_C_COMPILER STREQUAL intel)
                set (UNIC_C_COMPILER icc)
        endif()

        # Rename openwatcom -> watcom
        if (UNIC_C_COMPILER STREQUAL openwatcom)
                set (UNIC_C_COMPILER watcom)
        endif()

        # Rename xl -> xlc
        if (UNIC_C_COMPILER STREQUAL xl)
                set (UNIC_C_COMPILER xlc)
        endif()

        # Assign result
        set (${result} ${UNIC_C_COMPILER} PARENT_SCOPE)
endfunction (unic_detect_c_compiler)

function (unic_detect_os_bits result)
        if (CMAKE_SIZEOF_VOID_P EQUAL 8)
                set (UNIC_ARCH_BITS 64)
        else()
                set (UNIC_ARCH_BITS 32)
        endif()

        set (${result} ${UNIC_ARCH_BITS} PARENT_SCOPE)
endfunction (unic_detect_os_bits)

function (unic_detect_cpu_arch result)
        if (CMAKE_SYSTEM_PROCESSOR MATCHES "(i[1-9]86)|(x86_64)")
                if (CMAKE_CROSSCOMPILING)
                        if (CMAKE_SYSTEM_PROCESSOR MATCHES "i[1-9]86")
                                set (UNIC_PROCESSOR_ARCH "x86")
                        else()
                                set (UNIC_PROCESSOR_ARCH "x64")
                        endif()
                else()
                        unic_detect_os_bits (UNIC_OS_BITS)
                        if (UNIC_OS_BITS STREQUAL "32")
                                set (UNIC_PROCESSOR_ARCH "x86")
                        else()
                                set (UNIC_PROCESSOR_ARCH "x64")
                        endif()
                endif()
        else()
                set (UNIC_PROCESSOR_ARCH ${CMAKE_SYSTEM_PROCESSOR})
        endif()

        set (${result} ${UNIC_PROCESSOR_ARCH} PARENT_SCOPE)
endfunction (unic_detect_cpu_arch)

function (unic_detect_target_os result)
        string (TOLOWER ${CMAKE_SYSTEM_NAME} UNIC_TARGET_OS)

        # Rename mingw -> windows
        if (UNIC_TARGET_OS MATCHES "(mingw.*)")
                set (UNIC_TARGET_OS windows)
        endif()

        # Rename hp-ux -> hpux
        if (UNIC_TARGET_OS STREQUAL hp-ux)
                set (UNIC_TARGET_OS hpux)
        endif()

        # Rename sco_sv -> scosv
        if (UNIC_TARGET_OS STREQUAL sco_sv)
                set (UNIC_TARGET_OS scosv)
        endif()

        # Rename osf1 -> tru64
        if (UNIC_TARGET_OS STREQUAL osf1)
                set (UNIC_TARGET_OS tru64)
        endif()

        set (${result} ${UNIC_TARGET_OS} PARENT_SCOPE)
endfunction (unic_detect_target_os)

function (unic_detect_target_platform result)
        unic_detect_target_os (UNIC_TARGET_OS)
        unic_detect_os_bits (UNIC_OS_BITS)
        unic_detect_c_compiler (UNIC_C_COMPILER)

        if (UNIC_TARGET_OS STREQUAL windows)
                set (UNIC_TARGET_PLATFORM win${UNIC_OS_BITS})
        else()
                set (UNIC_TARGET_PLATFORM ${UNIC_TARGET_OS})
        endif()

        set (UNIC_TARGET_PLATFORM ${UNIC_TARGET_PLATFORM}-${UNIC_C_COMPILER})
        set (${result} ${UNIC_TARGET_PLATFORM} PARENT_SCOPE)
endfunction (unic_detect_target_platform)
