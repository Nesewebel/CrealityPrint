function(__sub_build_info_line)
  set(__SUB_BUILD_INFO_DEFINE "${__SUB_BUILD_INFO_DEFINE}\n" PARENT_SCOPE)
endfunction()

function(__sub_build_info_text text)
  set(__SUB_BUILD_INFO_DEFINE "${__SUB_BUILD_INFO_DEFINE}\n${text}" PARENT_SCOPE)
endfunction()

function(__sub_build_info_commit commit_message)
  set(__SUB_BUILD_INFO_DEFINE "${__SUB_BUILD_INFO_DEFINE}// ${commit_message}" PARENT_SCOPE)
endfunction()

function(__sub_build_info_macro macro_name)
  set(__SUB_BUILD_INFO_DEFINE "${__SUB_BUILD_INFO_DEFINE}\n#define ${macro_name}" PARENT_SCOPE)
endfunction()

function(__sub_build_info_boolean macro_name boolean_value)
  set(__SUB_BUILD_INFO_DEFINE "${__SUB_BUILD_INFO_DEFINE}\n#define ${macro_name} ${boolean_value}" PARENT_SCOPE)
endfunction()

function(__sub_build_info_number macro_name number_value)
  set(__SUB_BUILD_INFO_DEFINE "${__SUB_BUILD_INFO_DEFINE}\n#define ${macro_name} ${number_value}" PARENT_SCOPE)
endfunction()

function(__sub_build_info_string macro_name string_value)
  set(__SUB_BUILD_INFO_DEFINE "${__SUB_BUILD_INFO_DEFINE}\n#define ${macro_name} \"${string_value}\"" PARENT_SCOPE)
endfunction()

macro(__sub_build_info_begin_group group_name)
  __sub_build_info_line()
  __sub_build_info_commit("${group_name} [begin]")
endmacro()

macro(__sub_build_info_end_group group_name)
  __sub_build_info_line()
  __sub_build_info_commit("${group_name} [end]")
  __sub_build_info_line()
endmacro()

macro(__configure_build_info_header)
  set(__SUB_BUILD_INFO_DEFINE "")
  foreach(dir ${ARGV})
    set(__SUB_BUILD_INFO_DIR ${CMAKE_SOURCE_DIR}/${dir})
    __sub_build_info_begin_group(${dir})
    if(EXISTS ${__SUB_BUILD_INFO_DIR}/module_info.cmake)
      include(${__SUB_BUILD_INFO_DIR}/module_info.cmake)
    endif()
    if(EXISTS ${__SUB_BUILD_INFO_DIR}/build_info.cmake)
      include(${__SUB_BUILD_INFO_DIR}/build_info.cmake)
    endif()
    __sub_build_info_end_group(${dir})
  endforeach()

  if(NOT APP_TYPE)
    set(APP_TYPE 0)
  endif()

  string(TIMESTAMP BUILD_TIME "%y-%m-%d %H:%M")
	set(BUILD_INFO_HEAD "BUILDINFO_H_  // ${PROJECT_NAME} ${BUILD_TIME}")
	set(DEBUG_RESOURCES_DIR "${BIN_OUTPUT_DIR}/Debug/resources/")
	set(RELEASE_RESOURCES_DIR "${BIN_OUTPUT_DIR}/Release/resources/")
	__get_main_git_hash(MAIN_GIT_HASH)

  if(NOT DEFINED PROJECT_VERSION_EXTRA)
  set(PROJECT_VERSION_EXTRA "Alpha")
  endif()
  set(SHIPPING_LEVEL 2)
  if(${PROJECT_VERSION_EXTRA} STREQUAL "Release")
	set(SHIPPING_LEVEL 2)
  elseif(${PROJECT_VERSION_EXTRA} STREQUAL "Beta")
	set(SHIPPING_LEVEL 1)
  elseif(${PROJECT_VERSION_EXTRA} STREQUAL "Alpha")
	set(SHIPPING_LEVEL 0)
  else()
	set(SHIPPING_LEVEL 0)
  endif()
  
  # if(EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/cmake/buildinfo.h.in)
  #   configure_file(${CMAKE_CURRENT_SOURCE_DIR}/cmake/buildinfo.h.in
  #                 ${CMAKE_CURRENT_BINARY_DIR}/src/libslic3r/buildinfo.h)
  # endif()
endmacro()

# old
macro(__build_info_slic3r_header)
  if(NOT APP_TYPE)
    set(APP_TYPE 0)
  endif()

  string(TIMESTAMP BUILD_TIME "%y-%m-%d %H:%M")
  set(BUILD_INFO_HEAD "BUILDINFO_H_  // ${PROCESS_NAME} ${BUILD_TIME}")
  # set(DEBUG_RESOURCES_DIR "${BIN_OUTPUT_DIR}/Debug/resources/")
  # set(RELEASE_RESOURCES_DIR "${BIN_OUTPUT_DIR}/Release/resources/")
  set(PROJECT_DLL_NAME_WIN32 "${PROCESS_NAME}_Slicer")
  __get_main_git_hash(MAIN_GIT_HASH)

  message("================__build_info_header=========")
  message("CMAKE_CURRENT_SOURCE_DIR1=${CMAKE_CURRENT_SOURCE_DIR}")
  message("CMAKE_CURRENT_BINARY_DIR=${CMAKE_CURRENT_BINARY_DIR}")
  
  if(EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/cmake/buildinfo.h.in)
    configure_file(${CMAKE_CURRENT_SOURCE_DIR}/cmake/buildinfo.h.in
                   ${CMAKE_CURRENT_BINARY_DIR}/src/libslic3r/buildinfo.h)
  endif()
endmacro()


macro(__build_rc_config)
  if(EXISTS ${CMAKE_SOURCE_DIR}/customized/${CUSTOM_TYPE}/msw/CrealityPrint.rc.in)
    configure_file(${CMAKE_SOURCE_DIR}/customized/${CUSTOM_TYPE}/msw/CrealityPrint.rc.in ${CMAKE_CURRENT_BINARY_DIR}/CrealityPrint.rc @ONLY)
  endif()
endmacro()
macro(__build_manifest_config)
  if(EXISTS ${CMAKE_SOURCE_DIR}/customized/${CUSTOM_TYPE}/msw/CrealityPrint.manifest.in)
    configure_file(${CMAKE_SOURCE_DIR}/customized/${CUSTOM_TYPE}/msw/CrealityPrint.manifest.in ${CMAKE_CURRENT_BINARY_DIR}/CrealityPrint.manifest @ONLY)
  endif()
endmacro()
