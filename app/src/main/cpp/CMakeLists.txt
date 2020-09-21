cmake_minimum_required(VERSION 3.4.1)

#add_library(native-lib SHARED IMPORTED)
add_library(
        native-lib
        SHARED
        native-lib.cpp
        WatermarkAlgorithm.cpp
        #              WatermarkAlgorithm.cpp
)

#include_directories(sdk/native/jni/include)
include_directories(${CMAKE_SOURCE_DIR}/../../opencv/include)
add_library( lib-opencv
        SHARED
        IMPORTED )
set_target_properties(
        lib-opencv
        PROPERTIES IMPORTED_LOCATION
        ${CMAKE_CURRENT_SOURCE_DIR}/../jniLibs/${ANDROID_ABI}/libopencv_java3.so
)

target_link_libraries(
        native-lib
        lib-opencv
        jnigraphics
)
#add_library(
#            app-glue
#            SHARED
#            app/src/opencv/include/opencv2/core/cvstd.cpp
#)

#find_library(
#              log-lib
#              log
#)

#include_directories(${CMAKE_SOURCE_DIR}/../../opencv/include)
#link_directories(${CMAKE_SOURCE_DIR}/../jniLibs/${ANDROID_ABI})
#link_directories(${CMAKE_SOURCE_DIR}/../jniLibs/)
#include_directories(src/main/cpp)

#target_link_libraries(
#                        native-lib
##                        ${log-lib}
##                        lib_opencv
#                        lib-opencv
#)