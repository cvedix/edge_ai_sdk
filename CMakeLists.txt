cmake_minimum_required(VERSION 3.20)
project(rtsp_ba_crossline_app)

# Set C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Add compile options
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra")

# Include module cần thiết cho check_required_components
# (Cần thiết vì file config của SDK sử dụng macro này)
include(CMakePackageConfigHelpers)

# Định nghĩa macro check_required_components nếu chưa có
# (Một số phiên bản CMake có thể không có macro này sẵn)
if(NOT COMMAND check_required_components)
    macro(check_required_components _package)
        foreach(_comp ${${_package}_FIND_COMPONENTS})
            if(NOT ${_package}_${_comp}_FOUND)
                if(${_package}_FIND_REQUIRED_${_comp})
                    set(${_package}_FOUND FALSE)
                endif()
            endif()
        endforeach()
    endmacro()
endif()

# Tìm SDK CVEDIX
# Có thể chỉ định đường dẫn SDK nếu không cài đặt vào hệ thống
# Cách 1: Sử dụng biến môi trường CMAKE_PREFIX_PATH
# Cách 2: Uncomment và chỉnh sửa dòng dưới để chỉ định đường dẫn cụ thể
# set(CMAKE_PREFIX_PATH "/path/to/sdk/install" ${CMAKE_PREFIX_PATH})
# Cách 3: Sử dụng biến môi trường CVEDIX_SDK_ROOT (nếu đã cấu hình trong ~/.bashrc)
if(DEFINED ENV{CVEDIX_SDK_ROOT})
    set(CMAKE_PREFIX_PATH "$ENV{CVEDIX_SDK_ROOT}" ${CMAKE_PREFIX_PATH})
endif()

# Tìm package CVEDIX SDK
find_package(cvedix REQUIRED)

# Include thư mục hiện tại cho config.h
include_directories(${CMAKE_CURRENT_SOURCE_DIR})

# Source files
set(SOURCES
    src/main.cpp
)

# Create executable
add_executable(${PROJECT_NAME} ${SOURCES})

# Liên kết với SDK CVEDIX
target_link_libraries(${PROJECT_NAME} PRIVATE cvedix::cvedix_instance_sdk)

# Set RPATH để tìm SDK libraries khi chạy
# Điều này giúp tìm thấy libtinyexpr.so và các libraries khác
if(EXISTS /opt/cvedix/lib)
    set_target_properties(${PROJECT_NAME} PROPERTIES
        BUILD_RPATH "/opt/cvedix/lib"
        INSTALL_RPATH "/opt/cvedix/lib"
        BUILD_WITH_INSTALL_RPATH TRUE
    )
    # Also add library directory for linking
    target_link_directories(${PROJECT_NAME} PRIVATE /opt/cvedix/lib)
endif()

# Installation
install(TARGETS ${PROJECT_NAME}
    RUNTIME DESTINATION bin
)

# Build all samples
add_subdirectory(samples)

