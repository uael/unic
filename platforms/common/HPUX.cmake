include(CheckFunctionExists)

function(unic_hpux_detect_libraryloader_model result)
  message(STATUS "Checking whether dlopen() presents")

  check_function_exists(dlopen UNIC_HPUX_HAS_DLOPEN)

  if (UNIC_HPUX_HAS_DLOPEN)
    message(STATUS "Checking whether dlopen() presents - yes")
    set(${result} posix PARENT_SCOPE)
  else ()
    message(STATUS "Checking whether dlopen() presents - no")
    set(${result} shl PARENT_SCOPE)
  endif ()
endfunction(unic_hpux_detect_libraryloader_model)
