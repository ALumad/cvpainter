project(cvpainter)
cmake_minimum_required(VERSION 3.13)
set(CMAKE_CXX_STANDARD 17)
find_package(OpenCV  REQUIRED HINTS C:/opencv/build)
find_package(PkgConfig)
if ( NOT (PKGCONFIG_FOUND))
      message(FATAL_ERROR "Please Install PkgConfig: CMake will Exit")
endif()

pkg_check_modules(GST REQUIRED gstreamer-1.0)


set ( CMAKE_WINDOWS_EXPORT_ALL_SYMBOLS ON )


add_library( cvpainter SHARED ${CMAKE_CURRENT_SOURCE_DIR}/src/gstcvpainter.cpp )	
target_include_directories(cvpainter PUBLIC
    ${GST_INCLUDE_DIRS}
    ${OpenCV_INCLUDE_DIRS}
)

link_libraries(${GST_LIBRARIES})
target_link_libraries(cvpainter 
    C:/gstreamer/1.0/msvc_x86_64/lib/glib-2.0.lib
    C:/gstreamer/1.0/msvc_x86_64/lib/gmodule-2.0.lib
    C:/gstreamer/1.0/msvc_x86_64/lib/gobject-2.0.lib
    C:/gstreamer/1.0/msvc_x86_64/lib/gstreamer-1.0.lib
    ${OpenCV_LIBRARIES}
)

target_compile_options(cvpainter PRIVATE ${GST_FLAGS})