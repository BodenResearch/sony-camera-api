cmake_minimum_required(VERSION 3.10)
if(APPLE)
set(CMAKE_OSX_DEPLOYMENT_TARGET "12.1" CACHE STRING "Minimum OS X deployment version")
endif(APPLE)
project(RemoteCli LANGUAGES CXX)

if(WIN32)
    set_property ( DIRECTORY  PROPERTY
        VS_STARTUP_PROJECT RemoteCli
    )
endif()

### Append project cmake script dir ###
list(APPEND CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/cmake)

### Enumerate project files ###
include(enum_cli_hdr)
include(enum_cli_src)
include(enum_crsdk_hdr)

### Define output target ###
set(remotecli "${PROJECT_NAME}")
add_executable(${remotecli}
    ${cli_hdrs}
    ${cli_srcs}
    ${crsdk_hdrs}
)

if(APPLE)
    set_target_properties(${remotecli} PROPERTIES
        CXX_STANDARD 17
        CXX_STANDARD_REQUIRED YES
        CXX_EXTENSIONS NO
        BUILD_RPATH "@executable_path"
    )
endif(APPLE)

if(NOT APPLE)
    set_target_properties(${remotecli} PROPERTIES
        CXX_STANDARD 17
        CXX_STANDARD_REQUIRED YES
        CXX_EXTENSIONS NO
        BUILD_RPATH "$ORIGIN"
        INSTALL_RPATH "$ORIGIN"
    )
endif(NOT APPLE)

## Specify char is signed-char to fix mismatch with Raspbian
if(UNIX AND NOT APPLE)
    target_compile_options(${remotecli}
        PUBLIC
            -fstack-protector-all
        PRIVATE
            -fsigned-char
    )
endif(UNIX AND NOT APPLE)

target_include_directories(${remotecli}
    PRIVATE
        ${crsdk_hdr_dir} # defined in enum script
        ${CMAKE_CURRENT_SOURCE_DIR}/external/opencv/include
)

### Configure external library directories ###
set(ldir ${CMAKE_CURRENT_SOURCE_DIR}/external)
set(cr_ldir ${ldir}/crsdk)

### Link CRSDK library
find_library(camera_remote Cr_Core HINTS ${cr_ldir})
target_link_libraries(${remotecli}
    PRIVATE
        ${camera_remote}
)

### Windows specific configuration ###
if(WIN32)
    ## Build with unicode on Windows
    target_compile_definitions(${remotecli} PRIVATE UNICODE _UNICODE)
endif(WIN32)

if(MSVC)
    target_link_libraries(${remotecli} PRIVATE 
        $<$<CONFIG:Release>:${CMAKE_CURRENT_SOURCE_DIR}/external/opencv/Windows/x86_64/Release/lib/opencv_core480.lib>
        $<$<CONFIG:Release>:${CMAKE_CURRENT_SOURCE_DIR}/external/opencv/Windows/x86_64/Release/lib/opencv_highgui480.lib>
        $<$<CONFIG:Release>:${CMAKE_CURRENT_SOURCE_DIR}/external/opencv/Windows/x86_64/Release/lib/opencv_imgcodecs480.lib>
        $<$<CONFIG:Release>:${CMAKE_CURRENT_SOURCE_DIR}/external/opencv/Windows/x86_64/Release/lib/opencv_imgproc480.lib>
        $<$<CONFIG:Debug>:${CMAKE_CURRENT_SOURCE_DIR}/external/opencv/Windows/x86_64/Debug/lib/opencv_core480d.lib>
        $<$<CONFIG:Debug>:${CMAKE_CURRENT_SOURCE_DIR}/external/opencv/Windows/x86_64/Debug/lib/opencv_highgui480d.lib>
        $<$<CONFIG:Debug>:${CMAKE_CURRENT_SOURCE_DIR}/external/opencv/Windows/x86_64/Debug/lib/opencv_imgcodecs480d.lib>
        $<$<CONFIG:Debug>:${CMAKE_CURRENT_SOURCE_DIR}/external/opencv/Windows/x86_64/Debug/lib/opencv_imgproc480d.lib>
    )
    target_compile_options(${remotecli}
        PUBLIC
            -guard:cf
    )
endif(MSVC)

### Linux specific configuration ###
if(UNIX AND NOT APPLE)
    if("${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU")
        if(CMAKE_CXX_COMPILER_VERSION VERSION_LESS 8)
            # Must use std::experimental namespace if older than GCC8
            message("[${PROJECT_NAME}] GCC version less than 8. Using std::experimental namespace.")
            target_compile_definitions(${remotecli} PRIVATE USE_EXPERIMENTAL_FS)
        endif()

        if(CMAKE_CXX_COMPILER_VERSION VERSION_LESS 9)
            # Must explicitly link separate std::filesystem if older than GCC9
            message("[${PROJECT_NAME}] GCC version less than 9. Explicitly linking separate std::filesystem library.")
            target_link_libraries(${remotecli} PRIVATE stdc++fs)
        endif()
    endif()

    target_link_libraries(${remotecli} PRIVATE 
        ${CMAKE_CURRENT_SOURCE_DIR}/external/opencv/Linux/libopencv_core.so.408
        ${CMAKE_CURRENT_SOURCE_DIR}/external/opencv/Linux/libopencv_highgui.so.408
        ${CMAKE_CURRENT_SOURCE_DIR}/external/opencv/Linux/libopencv_imgcodecs.so.408
        ${CMAKE_CURRENT_SOURCE_DIR}/external/opencv/Linux/libopencv_imgproc.so.408
    )

endif(UNIX AND NOT APPLE)

### Mac specific configuration ###
if(APPLE)
    target_link_libraries(${remotecli} PRIVATE 
        ${CMAKE_CURRENT_SOURCE_DIR}/external/opencv/Darwin/Release/macos/bin/libopencv_core.408.dylib
        ${CMAKE_CURRENT_SOURCE_DIR}/external/opencv/Darwin/Release/macos/bin/libopencv_highgui.408.dylib
        ${CMAKE_CURRENT_SOURCE_DIR}/external/opencv/Darwin/Release/macos/bin/libopencv_imgcodecs.408.dylib
        ${CMAKE_CURRENT_SOURCE_DIR}/external/opencv/Darwin/Release/macos/bin/libopencv_imgproc.408.dylib
    )
endif(APPLE)

## Copy required library binaries
if(WIN32)
    add_custom_command(TARGET ${remotecli} PRE_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_directory ${cr_ldir} $<TARGET_FILE_DIR:${remotecli}>
        COMMAND $<$<CONFIG:Release>:${CMAKE_COMMAND}> $<$<CONFIG:Release>:-E> $<$<CONFIG:Release>:copy_if_different> $<$<CONFIG:Release>:"${CMAKE_CURRENT_SOURCE_DIR}/external/opencv/Windows/x86_64/Release/bin/opencv_core480.dll"> $<$<CONFIG:Release>:$<TARGET_FILE_DIR:${remotecli}>>
        COMMAND $<$<CONFIG:Release>:${CMAKE_COMMAND}> $<$<CONFIG:Release>:-E> $<$<CONFIG:Release>:copy_if_different> $<$<CONFIG:Release>:"${CMAKE_CURRENT_SOURCE_DIR}/external/opencv/Windows/x86_64/Release/bin/opencv_highgui480.dll"> $<$<CONFIG:Release>:$<TARGET_FILE_DIR:${remotecli}>>
        COMMAND $<$<CONFIG:Release>:${CMAKE_COMMAND}> $<$<CONFIG:Release>:-E> $<$<CONFIG:Release>:copy_if_different> $<$<CONFIG:Release>:"${CMAKE_CURRENT_SOURCE_DIR}/external/opencv/Windows/x86_64/Release/bin/opencv_imgcodecs480.dll"> $<$<CONFIG:Release>:$<TARGET_FILE_DIR:${remotecli}>>
        COMMAND $<$<CONFIG:Release>:${CMAKE_COMMAND}> $<$<CONFIG:Release>:-E> $<$<CONFIG:Release>:copy_if_different> $<$<CONFIG:Release>:"${CMAKE_CURRENT_SOURCE_DIR}/external/opencv/Windows/x86_64/Release/bin/opencv_imgproc480.dll"> $<$<CONFIG:Release>:$<TARGET_FILE_DIR:${remotecli}>>
        COMMAND $<$<CONFIG:Debug>:${CMAKE_COMMAND}> $<$<CONFIG:Debug>:-E> $<$<CONFIG:Debug>:copy_if_different> $<$<CONFIG:Debug>:"${CMAKE_CURRENT_SOURCE_DIR}/external/opencv/Windows/x86_64/Debug/bin/opencv_core480d.dll"> $<$<CONFIG:Debug>:$<TARGET_FILE_DIR:${remotecli}>>
        COMMAND $<$<CONFIG:Debug>:${CMAKE_COMMAND}> $<$<CONFIG:Debug>:-E> $<$<CONFIG:Debug>:copy_if_different> $<$<CONFIG:Debug>:"${CMAKE_CURRENT_SOURCE_DIR}/external/opencv/Windows/x86_64/Debug/bin/opencv_highgui480d.dll"> $<$<CONFIG:Debug>:$<TARGET_FILE_DIR:${remotecli}>>
        COMMAND $<$<CONFIG:Debug>:${CMAKE_COMMAND}> $<$<CONFIG:Debug>:-E> $<$<CONFIG:Debug>:copy_if_different> $<$<CONFIG:Debug>:"${CMAKE_CURRENT_SOURCE_DIR}/external/opencv/Windows/x86_64/Debug/bin/opencv_imgcodecs480d.dll"> $<$<CONFIG:Debug>:$<TARGET_FILE_DIR:${remotecli}>>
        COMMAND $<$<CONFIG:Debug>:${CMAKE_COMMAND}> $<$<CONFIG:Debug>:-E> $<$<CONFIG:Debug>:copy_if_different> $<$<CONFIG:Debug>:"${CMAKE_CURRENT_SOURCE_DIR}/external/opencv/Windows/x86_64/Debug/bin/opencv_imgproc480d.dll"> $<$<CONFIG:Debug>:$<TARGET_FILE_DIR:${remotecli}>>
    )
endif(WIN32)

if(UNIX AND NOT APPLE)
    add_custom_command(TARGET ${remotecli} PRE_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_directory ${cr_ldir} $<TARGET_FILE_DIR:${remotecli}>
    )
    
    add_custom_command(TARGET ${remotecli} PRE_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different "${CMAKE_CURRENT_SOURCE_DIR}/external/opencv/Linux/libopencv_core.so.408" $<TARGET_FILE_DIR:${remotecli}>
        COMMAND ${CMAKE_COMMAND} -E copy_if_different "${CMAKE_CURRENT_SOURCE_DIR}/external/opencv/Linux/libopencv_highgui.so.408" $<TARGET_FILE_DIR:${remotecli}>
        COMMAND ${CMAKE_COMMAND} -E copy_if_different "${CMAKE_CURRENT_SOURCE_DIR}/external/opencv/Linux/libopencv_imgcodecs.so.408" $<TARGET_FILE_DIR:${remotecli}>
        COMMAND ${CMAKE_COMMAND} -E copy_if_different "${CMAKE_CURRENT_SOURCE_DIR}/external/opencv/Linux/libopencv_imgproc.so.408" $<TARGET_FILE_DIR:${remotecli}>
    )
endif(UNIX AND NOT APPLE)

if(APPLE)
    set(ct_fw "Contents/Frameworks")
    set(adaptor_dir_name "CrAdapter")
    add_custom_command(TARGET ${remotecli} PRE_BUILD
        COMMAND ${CMAKE_COMMAND} -E make_directory $<TARGET_FILE_DIR:${remotecli}>/${ct_fw}/${adaptor_dir_name}
        COMMAND ${CMAKE_COMMAND} -E copy_directory ${cr_ldir} $<TARGET_FILE_DIR:${remotecli}>
        COMMAND ${CMAKE_COMMAND} -E copy_directory $<TARGET_FILE_DIR:${remotecli}>/${adaptor_dir_name} $<TARGET_FILE_DIR:${remotecli}>/${ct_fw}/${adaptor_dir_name}
        COMMAND ${CMAKE_COMMAND} -E rm -rf $<TARGET_FILE_DIR:${remotecli}>/${adaptor_dir_name}
        COMMAND ${CMAKE_COMMAND} -E copy_if_different "${CMAKE_CURRENT_SOURCE_DIR}/external/opencv/Darwin/Release/macos/bin/libopencv_core.408.dylib" $<TARGET_FILE_DIR:${remotecli}>
        COMMAND ${CMAKE_COMMAND} -E copy_if_different "${CMAKE_CURRENT_SOURCE_DIR}/external/opencv/Darwin/Release/macos/bin/libopencv_highgui.408.dylib" $<TARGET_FILE_DIR:${remotecli}>
        COMMAND ${CMAKE_COMMAND} -E copy_if_different "${CMAKE_CURRENT_SOURCE_DIR}/external/opencv/Darwin/Release/macos/bin/libopencv_imgcodecs.408.dylib" $<TARGET_FILE_DIR:${remotecli}>
        COMMAND ${CMAKE_COMMAND} -E copy_if_different "${CMAKE_CURRENT_SOURCE_DIR}/external/opencv/Darwin/Release/macos/bin/libopencv_imgproc.408.dylib" $<TARGET_FILE_DIR:${remotecli}>
    )
endif(APPLE)


## Install application
## '.' means, install to the root directory of CMAKE_INSTALL_PREFIX
install(TARGETS ${remotecli} DESTINATION .)
install(DIRECTORY ${cr_ldir}/ DESTINATION .)
