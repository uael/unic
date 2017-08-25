$!
$! Copyright 2011, Richard Levitte <richard@levitte.org>
$! Copyright 2014, John Malmberg <wb8tyw@qsl.net>
$! Copyright 2016, Alexander Saprykin <xelfium@gmail.com>
$!
$! Permission to use, copy, modify, and/or distribute this software for any
$! purpose with or without fee is hereby granted, provided that the above
$! copyright notice and this permission notice appear in all copies.
$!
$! THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
$! WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
$! MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
$! ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
$! WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
$! ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT
$! OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
$!
$!===========================================================================
$! Command-line options:
$!
$!    32            Compile with 32-bit pointers.
$!    BIGENDIAN     Compile for a big endian host.
$!    CCQUAL=x      Add "x" to the C compiler qualifiers.
$!    DEBUG         Build in debug mode.
$!    CLEAN         Only perform clean after the previous build.
$!    TESTS=(x)     Build library tests. Comma separated test names or leave
$!                  empty to build all the tests.
$!                  Example 1 (curtain tests): TESTS=(pmem,pthread)
$!                  Example 2 (all tests): TESTS
$!    RUN_TESTS     Runs all tests.
$!    BOOST_ROOT=x  Boost root directory.
$!                  Example: BOOST_ROOT=/SYS$COMMON/boost_1_43_1
$!    NOLIB         Skip library buidling. Useful when you want to rebuild
$!                  particular tests.
$!===========================================================================
$!
$!
$! Save the original default dev:[dir], and arrange for its restoration
$! at exit.
$!---------------------------------------------------------------------
$ orig_def = f$environment("DEFAULT")
$ on error then goto common_exit
$ on control_y then goto common_exit
$!
$ ctrl_y       = 1556
$ proc         = f$environment("PROCEDURE")
$ proc_fid     = f$file_attributes(proc, "FID")
$ proc_dev     = f$parse(proc, , , "DEVICE")
$ proc_dir     = f$parse(proc, , , "DIRECTORY")
$ proc_name    = f$parse(proc, , , "NAME")
$ proc_type    = f$parse(proc, , , "TYPE")
$ proc_dev_dir = proc_dev + proc_dir
$!
$! Have to manually parse the device for a search list.
$! Can not use the f$parse() as it will return the first name
$! in the search list.
$!
$ orig_def_dev = f$element(0, ":", orig_def) + ":"
$ if orig_def_dev .eqs. "::" then orig_def_dev = "sys$disk:"
$ test_proc = orig_def_dev + proc_dir + proc_name + proc_type
$!
$! If we can find this file using the default directory
$! then we know that we should use the original device from the
$! default directory which could be a search list.
$!
$ test_proc_fid = f$file_attributes(test_proc, "FID")
$!
$ if (test_proc_fid .eq. proc_fid)
$ then
$     proc_dev_dir = orig_def_dev + proc_dir
$ endif
$!
$! Verbose output message stuff. Define symbol to "write sys$output".
$! vo_c - verbose output for compile
$!
$ vo_c := "write sys$output"
$!
$! Determine the main distribution directory ("[--]") in an
$! ODS5-tolerant (case-insensitive) way.  (We do assume that the only
$! "]" or ">" is the one at the end.)
$!
$! Some non-US VMS installations report ">" for the directory delimiter
$! so do not assume that it is "]".
$!
$ orig_def_len = f$length(orig_def)
$ delim = f$extract(orig_def_len - 1, 1, orig_def)
$!
$ set default 'proc_dev_dir'
$ set default [--.src]
$ base_src_dir = f$environment("default")
$ set default 'proc_dev_dir'
$!
$! Define the architecture-specific destination directory name
$! -----------------------------------------------------------
$!
$ if (f$getsyi("HW_MODEL") .lt. 1024)
$ then
$      'vo_c' "%UNIC-F-NOTSUP, VAX platform is not supported, sorry :("
$      goto common_exit
$ else
$      arch_name = ""
$      arch_name = arch_name + f$edit(f$getsyi("ARCH_NAME"), "UPCASE")
$!
$      if (arch_name .eqs. "") then arch_name = "UNK"
$!
$      node_swvers   = f$getsyi("node_swvers")
$      version_patch = f$extract(1, f$length(node_swvers), node_swvers)
$      maj_ver       = f$element(0, ".", version_patch)
$      min_ver_patch = f$element(1, ".", version_patch)
$      min_ver       = f$element(0, "-", min_ver_patch)
$      patch         = f$element(1, "-", min_ver_patch)
$!
$      if maj_ver .lts. "8" .or. min_ver .lts. "4"
$      then
$          'vo_c' "%UNIC-F-NOTSUP, only OpenVMS 8.4 and above are supported, sorry :("
$          goto common_exit
$      endif
$ endif
$!
$ objdir = proc_dev_dir - delim + ".''arch_name'" + delim
$!
$! Parse input arguments
$! ---------------------
$! Allow arguments to be grouped together with comma or separated by spaces
$! Do no know if we will need more than 8.
$ args = "," + p1 + "," + p2 + "," + p3 + "," + p4 + ","
$ args = args + p5 + "," + p6 + "," + p7 + "," + p8 + ","
$!
$! Provide lower case version to simplify parsing.
$ args_lower = f$edit(args, "LOWERCASE,COLLAPSE")
$!
$ args_len = f$length(args)
$ args_lower_len = f$length(args_lower)
$!
$ if f$locate(",clean,", args_lower) .lt. args_lower_len
$ then
$     'vo_c' "Cleaning up previous build..."
$     set default 'proc_dev_dir'
$!
$     if f$search("''arch_name'.DIR") .nes. ""
$     then
$         set prot=w:d []'arch_name'.DIR;*
$         delete/tree [.'arch_name'...]*.*;*
$         delete []'arch_name'.DIR;*
$     endif
$!
$     goto common_exit
$ endif
$!
$ build_64   = 1
$ if f$locate(",32,", args_lower) .lt. args_lower_len
$ then
$     build_64 = 0
$ endif
$!
$ big_endian = 0
$ if f$locate(",bigendian,", args_lower) .lt. args_lower_len
$ then
$     big_endian = 1
$ endif
$!
$ cc_extra = ""
$ args_loc = f$locate(",ccqual=", args_lower)
$ if args_loc .lt. args_lower_len
$ then
$     arg = f$extract(args_loc + 1, args_lower_len, args_lower)
$     arg_val = f$element(0, ",", arg)
$     cc_extra = f$element(1, "=", arg_val)
$ endif
$!
$ is_debug = 0
$ if f$locate(",debug,", args_lower) .lt. args_lower_len
$ then
$     is_debug = 1
$ endif
$!
$ is_tests = 0
$ test_list = ""
$ if f$locate(",tests,", args_lower) .lt. args_lower_len
$ then
$     is_tests = 1
$ else
$     args_loc = f$locate(",tests=(", args_lower)
$     if args_loc .lt. args_lower_len
$     then
$         is_tests          = 1
$         arg               = f$extract(args_loc + 1, args_lower_len, args_lower)
$         arg_val           = f$element(0, ")", arg)
$         test_list_val     = f$element(1, "=", arg_val) - "(" - ")"
$         test_list_val     = f$edit(test_list_val, "COLLAPSE")
$         test_list_counter = 0
$
$ test_list_loop: 
$         next_test_val = f$element (test_list_counter, ",", test_list_val)
$         if next_test_val .nes. "" .and. next_test_val .nes. ","
$         then
$             test_list         = test_list + next_test_val + " "
$             test_list_counter = test_list_counter + 1
$             goto test_list_loop
$         endif
$     endif
$ endif
$!
$ run_tests = 0
$ if f$locate(",run_tests,", args_lower) .lt. args_lower_len
$ then
$     run_tests = 1
$ endif
$!
$ boost_root = ""
$ args_loc = f$locate(",boost_root=", args_lower)
$ if args_loc .lt. args_lower_len
$ then
$     arg = f$extract(args_loc + 1, args_lower_len, args_lower)
$     arg_val = f$element(0, ",", arg)
$     boost_root = f$element(1, "=", arg_val)
$ endif
$!
$ if is_tests .eqs. "1" .and. boost_root .eqs. ""
$ then
$     'vo_c' "%UNIC-I-NOTESTS, tests couldn't be built without BOOST_ROOT parameter, disabling."
$     is_tests = 0
$ endif
$!
$ if is_tests .eqs. "0" .and. boost_root .nes. ""
$ then
$     'vo_c' "%UNIC-I-BOOSTIGN, BOOST_ROOT parameter will be ignored without tests enabled."
$ endif
$!
$! Prepare build directory
$! -----------------------
$!
$! When building on a search list, need to do a create to make sure that
$! the output directory exists, since the clean procedure tries to delete
$! it.
$!
$ if f$search("''proc_dev_dir'''arch_name'.DIR") .eqs. ""
$ then
$     create/dir 'objdir'/prot=o:rwed
$ endif
$!
$ set default 'objdir'
$ if f$search("CXX_REPOSITORY.DIR") .nes. ""
$ then
$     set prot=w:d []CXX_REPOSITORY.DIR;*
$     delete/tree [.CXX_REPOSITORY...]*.*;*
$     delete []CXX_REPOSITORY.DIR;*
$ endif
$!
$ if f$locate(",nolib,", args_lower) .lt. args_lower_len
$ then
$     goto build_tests
$ endif
$!
$! Generate platform-specific config file
$! --------------------------------------
$!
$ if f$search("include/unic/config.h") .nes. "" then delete include/unic/config.h;*
$!
$! Get the version number
$! ----------------------
$!
$ i = 0
$ open/read/error=version_loop_end vhf [---]CMakeLists.txt
$ version_loop:
$     read/end=version_loop_end vhf line_in
$!
$     if line_in .eqs. "" then goto version_loop
$!
$     if f$locate("set (UNIC_VERSION_MAJOR ", line_in) .eq. 0
$     then
$         unic_vmajor = f$element(2, " ", line_in) - ")"
$         i = i + 1
$     endif
$!
$     if f$locate("set (UNIC_VERSION_MINOR ", line_in) .eq. 0
$     then
$         unic_vminor = f$element(2, " ", line_in) - ")"
$         i = i + 1
$     endif
$!
$     if f$locate("set (UNIC_VERSION_PATCH ", line_in) .eq. 0
$     then
$         unic_vpatch = f$element(2, " ", line_in) - ")"
$         i = i + 1
$     endif
$!
$     if f$locate("set (UNIC_VERSION_NUM ", line_in) .eq. 0
$     then
$         unic_vnum = f$element(2, " ", line_in) - ")"
$         i = i + 1
$     endif
$!
$     if i .lt 4 then goto version_loop
$ version_loop_end:
$ close vhf
$!
$! Write config file
$! -----------------
$!
$ open/write/error=config_write_end chf include/unic/config.h
$ write chf "#ifndef UNIC_HEADER_UNICCONFIG_H"
$ write chf "#define UNIC_HEADER_UNICCONFIG_H"
$ write chf ""
$ write chf "#define UNIC_VERSION_MAJOR ''unic_vmajor'"
$ write chf "#define UNIC_VERSION_MINOR ''unic_vminor'"
$ write chf "#define UNIC_VERSION_PATCH ''unic_vpatch'"
$ write chf "#define UNIC_VERSION_STR ""''unic_vmajor'.''unic_vminor'.''unic_vpatch'"""
$ write chf "#define UNIC_VERSION ''unic_vnum'"
$ write chf ""
$ write chf "#define UNIC_SIZEOF_SAFAMILY_T 1"
$ write chf ""
$ write chf "#include <pmacros.h>"
$ write chf ""
$ write chf "#include <float.h>"
$ write chf "#include <limits.h>"
$ write chf ""
$ write chf "U_BEGIN_DECLS"
$ write chf ""
$ write chf "#define U_MINFLOAT    FLT_MIN"
$ write chf "#define U_MAXFLOAT    FLT_MAX"
$ write chf "#define U_MINDOUBLE   DBL_MIN"
$ write chf "#define U_MAXDOUBLE   DBL_MAX"
$ write chf "#define U_MINSHORT    SHRT_MIN"
$ write chf "#define U_MAXSHORT    SHRT_MAX"
$ write chf "#define U_MAXUSHORT   USHRT_MAX"
$ write chf "#define U_MININT      INT_MIN"
$ write chf "#define U_MAXINT      INT_MAX"
$ write chf "#define U_MAXUINT     UINT_MAX"
$ write chf "#define U_MINLONG     LONG_MIN"
$ write chf "#define U_MAXLONG     LONG_MAX"
$ write chf "#define U_MAXULONG    ULONG_MAX"
$ write chf ""
$ write chf "#define UNIC_MMAP_HAS_MAP_ANONYMOUS"
$ write chf "#define UNIC_HAS_NANOSLEEP"
$ write chf "#define UNIC_HAS_GETADDRINFO"
$ write chf "#define UNIC_HAS_POSIX_SCHEDULING"
$ write chf "#define UNIC_HAS_POSIX_STACKSIZE"
$ write chf "#define UNIC_HAS_SOCKADDR_STORAGE"
$ write chf "#define UNIC_SOCKADDR_HAS_SA_LEN"
$ write chf "#define UNIC_SOCKADDR_IN6_HAS_SCOPEID"
$ write chf "#define UNIC_SOCKADDR_IN6_HAS_FLOWINFO"
$ write chf ""
$!
$ if build_64 .eqs. "1"
$ then
$     write chf "#define UNIC_SIZEOF_VOID_P 8"
$     write chf "#define UNIC_SIZEOF_SIZE_T 8"
$ else
$     write chf "#define UNIC_SIZEOF_VOID_P 4"
$     write chf "#define UNIC_SIZEOF_SIZE_T 4"
$ endif
$!
$ write chf "#define UNIC_SIZEOF_LONG 4"
$ write chf ""
$!
$ if big_endian .eqs. "1"
$ then
$     write chf "#define U_BYTE_ORDER U_BIG_ENDIAN"
$ else
$     write chf "#define U_BYTE_ORDER U_LITTLE_ENDIAN"
$ endif
$!
$ write chf ""
$ write chf "U_END_DECLS"
$ write chf ""
$ write chf "#endif /* UNIC_HEADER_UNICCONFIG_H */"
$ config_write_end:
$     close chf
$!
$! Prepare sources for compilation
$! -------------------------------
$!
$ cc_link_params = ""
$ cc_params = "/NAMES=(AS_IS,SHORTENED)"
$ cc_params = cc_params + "/DEFINE=(UNIC_COMPILATION,_REENTRANT,_POSIX_EXIT)"
$ cc_params = cc_params + "/INCLUDE_DIRECTORY=(''objdir',''base_src_dir')"
$ cc_params = cc_params + "/FLOAT=IEEE/IEEE_MODE=DENORM_RESULTS"
$!
$ if build_64 .eqs. "1"
$ then
$     cc_params = cc_params + "/POINTER_SIZE=64"
$ else
$     cc_params = cc_params + "/POINTER_SIZE=32"
$ endif
$!
$ if cc_extra .nes. ""
$ then
$     cc_params = cc_params + " " + cc_extra
$ endif
$!
$ if is_debug .eqs. "1"
$ then
$     cc_params = cc_params + "/DEBUG/NOOPTIMIZE/LIST/SHOW=ALL"
$     cc_link_params = "/DEBUG/TRACEBACK"
$ else
$     cc_link_params = "/NODEBUG/NOTRACEBACK"
$ endif
$!
$ unic_src = "patomic-decc.c condvar-posix.c hash-gost3411.c hash-md5.c"
$ unic_src = unic_src + " hash-sha1.c hash-sha2-256.c hash-sha2-512.c"
$ unic_src = unic_src + " hash-sha3.c hash.c dir-posix.c dir.c"
$ unic_src = unic_src + " err.c file.c htable.c inifile.c ipc.c dl-posix.c"
$ unic_src = unic_src + " list.c main.c mem.c mutex-posix.c process.c rwlock-posix.c"
$ unic_src = unic_src + " sema-posix.c shm-posix.c shmbuf.c socket.c"
$ unic_src = unic_src + " socketaddr.c spinlock-decc.c string.c sysclose-unix.c"
$ unic_src = unic_src + " profiler-posix.c profiler.c tree-avl.c tree-bst.c"
$ unic_src = unic_src + " tree-rb.c tree.c thread-posix.c thread.c"
$!
$! Inform about building
$! ---------------------
$!
$ if build_64 .eqs. "1"
$ then
$     'vo_c' "Building for ''arch_name' (64-bit)"
$ else
$     'vo_c' "Building for ''arch_name' (32-bit)"
$ endif
$!
$! Compile library modules
$! -----------------------
$!
$ 'vo_c' "Compiling object modules..."
$ src_counter = 0
$ unic_src = f$edit(unic_src, "COMPRESS")
$ unic_objs = ""
$!
$ src_loop:
$     next_src = f$element (src_counter, " ", unic_src)
$     if next_src .nes. "" .and. next_src .nes. " "
$     then
$         'vo_c' "[CC] ''next_src'"
$         cc [---.src]'next_src' 'cc_params'
$!
$         src_counter = src_counter + 1
$!
$         obj_file = f$extract (0, f$length (next_src) - 1, next_src) + "obj"
$         unic_objs = unic_objs + "''obj_file',"
$         purge 'obj_file'
$!
$         goto src_loop
$     endif
$!
$ unic_objs = f$extract (0, f$length (unic_objs) - 1, unic_objs)
$!
$! Create library
$! --------------
$!
$ 'vo_c' "Creating object library..."
$ library/CREATE/INSERT/REPLACE /LIST=UNIC.LIS UNIC.OLB 'unic_objs'
$ library/COMPRESS UNIC.OLB
$ purge UNIC.OLB
$ purge UNIC.LIS
$!
$ 'vo_c' "Creating shared library..."
$ link/SHAREABLE=UNIC.EXE /MAP=UNIC.MAP 'cc_link_params' 'unic_objs', [-]unic.opt/OPTION
$ purge UNIC.EXE
$ purge UNIC.MAP
$!
$! Testing area
$! ------------
$!
$ build_tests:
$ test_list_full = "atomic condvar hash dir"
$ test_list_full = test_list_full + " err file htable inifile dl list"
$ test_list_full = test_list_full + " macros main mem mutex process rwlock sema"
$ test_list_full = test_list_full + " shm shmbuf socket socketaddr spinlock string"
$ test_list_full = test_list_full + " profiler tree types thread"
$!
$ if is_tests .eqs. "0"
$ then
$     goto build_done
$ endif
$!
$! Write link options file
$! -----------------------
$!
$ if f$search("unic_link.opt") .nes. "" then delete unic_link.opt;*
$!
$ open/write/error=link_write_end lhf unic_link.opt
$ write lhf "''objdir'UNIC.EXE/SHARE"
$ write lhf ""
$ link_write_end:
$     close lhf
$!
$! Compile tests
$! -------------------------
$!
$ if test_list .nes. ""
$ then
$     unic_tests = f$edit(test_list, "TRIM")
$ else
$     unic_tests = test_list_full
$ endif
$!
$ 'vo_c' "Compiling test executables..."
$ test_counter = 0
$ unic_tests = f$edit(unic_tests, "COMPRESS")
$!
$ cxx_params = "/INCLUDE=(''objdir',''base_src_dir',""''boost_root'"")"
$ cxx_params = cxx_params + "/DEFINE=(__USE_STD_IOSTREAM,UNIC_TESTS_STATIC)/NAMES=(AS_IS, SHORTENED)"
$ cxx_params = cxx_params + "/FLOAT=IEEE/IEEE_MODE=DENORM_RESULTS"
$!
$ if build_64 .eqs. "1"
$ then
$     set noon
$     define/user/nolog sys$output NL:
$     define/user/nolog sys$error NL:
$     cxx/POINTER_SIZE=64=ARGV NL:
$!
$     if ($STATUS .and. %X0FFF0000) .eq. %X00030000
$     then
$!
$!        If we got here, it means DCL complained like this:
$!        %DCL-W-NOVALU, value not allowed - remove value specification
$!        \64=\
$!
$!        If the compiler was run, logicals defined in /USER would
$!        have been deassigned automatically.  However, when DCL
$!        complains, they aren't, so we do it here (it might be
$!        unnecessary, but just in case there will be another error
$!        message further on that we don't want to miss).
$!
$         deassign/user/nolog sys$error
$         deassign/user/nolog sys$output
$         cxx_params = cxx_params + "/POINTER_SIZE=64"
$     else
$         cxx_params = cxx_params + "/POINTER_SIZE=64=ARGV"
$     endif
$ else
$     cxx_params = cxx_params + "/POINTER_SIZE=32"
$ endif
$!
$ if is_debug .eqs. "1"
$ then
$     cxx_params = cxx_params + "/DEBUG/NOOPTIMIZE/LIST/SHOW=ALL"
$ endif
$!
$ test_loop:
$     next_test = f$element (test_counter, " ", unic_tests)
$     if next_test .nes. "" .and. next_test .nes. " "
$     then
$         next_test = next_test + "_test"
$         'vo_c' "[CXX    ] ''next_test'.cpp"
$         cxx [---.tests]'next_test'.cpp 'cxx_params'
$!
$         'vo_c' "[CXXLINK] ''next_test'.obj"
$          cxxlink 'next_test'.obj,'objdir'unic_link.opt/OPTION /THREADS_ENABLE
$!
$         if f$search("CXX_REPOSITORY.DIR") .nes. ""
$         then
$             set prot=w:d []CXX_REPOSITORY.DIR;*
$             delete/tree [.CXX_REPOSITORY...]*.*;*
$             delete []CXX_REPOSITORY.DIR;*
$         endif
$!
$         purge 'next_test'.obj
$         purge 'next_test'.exe
$!
$         test_counter = test_counter + 1
$         goto test_loop
$     endif
$!
$ build_done:
$     'vo_c' "Build done."
$!
$! Run unit tests
$! --------------
$!
$ if run_tests .eqs. "0"
$ then
$     if is_tests .eqs. "1"
$     then
$         'vo_c' "To run tests invoke: @build_vms.com NOLIB RUN_TESTS"
$     endif
$     goto common_exit
$ endif
$!
$ 'vo_c' "Running tests..."
$ test_counter = 0
$ tests_passed = 0
$!
$ run_loop:
$     next_test = f$element (test_counter, " ", test_list_full)
$     if next_test .nes. "" .and. next_test .nes. " "
$     then
$         if f$search("''next_test'_test.exe") .eqs. ""
$         then
$             'vo_c' "[SKIP] Test not found: ''next_test'"
$             goto run_loop_next
$         endif
$!
$         'vo_c' "[RUN ] ''next_test'"
$!
$         define/user/nolog sys$error NL:
$         define/user/nolog sys$output NL:
$         define/user/nolog unic 'objdir'UNIC.EXE
$         define/user/nolog test_imgdir 'objdir'
$!
$         xrun := $test_imgdir:'next_test'_test.exe
$         if next_test .eqs. "dl"
$         then
$             xrun 'objdir'UNIC.EXE
$         else
$             xrun
$         endif
$!
$         if $STATUS .eqs. "%X00000001"
$         then
$             'vo_c' "[PASS] Test passed: ''next_test'"
$             tests_passed = tests_passed + 1
$         else
$             'vo_c' "[FAIL] *** Test failed: ''next_test'"
$         endif
$!
$ run_loop_next:
$         test_counter = test_counter + 1
$         goto run_loop
$     endif
$!
$ 'vo_c' "Tests passed: ''tests_passed'/''test_counter'"
$!
$! In case of error during the last test
$ deassign/user/nolog sys$error
$ deassign/user/nolog sys$output
$ deassign/user/nolog unic
$ deassign/user/nolog test_imgdir
$!
$ common_exit:
$     set default 'orig_def'
$     exit
