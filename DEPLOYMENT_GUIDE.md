# Hướng dẫn Triển khai CVEDIX SDK vào Dự án Thực tế

## Mục lục

1. [Tổng quan](#tổng-quan)
2. [Yêu cầu hệ thống](#yêu-cầu-hệ-thống)
3. [Cài đặt SDK](#cài-đặt-sdk)
4. [Tích hợp SDK vào dự án](#tích-hợp-sdk-vào-dự-án)
5. [Các phương pháp tích hợp](#các-phương-pháp-tích-hợp)
6. [Ví dụ triển khai thực tế](#ví-dụ-triển-khai-thực-tế)
7. [Best Practices](#best-practices)
8. [Troubleshooting](#troubleshooting)
9. [Kiểm tra và Debug](#kiểm-tra-và-debug)

---

## Tổng quan

CVEDIX Instance Pipeline SDK là một framework C++ mạnh mẽ để xây dựng các ứng dụng phân tích video và AI. Tài liệu này hướng dẫn chi tiết cách triển khai SDK vào các dự án thực tế, từ cài đặt ban đầu đến tích hợp hoàn chỉnh.

### Kiến trúc SDK

SDK được xây dựng dựa trên mô hình pipeline với các thành phần chính:

- **Source Nodes**: Nhận dữ liệu từ các nguồn khác nhau (file, RTSP, RTMP, camera, ...)
- **Inference Nodes**: Thực hiện suy luận AI (YOLO, TensorRT, RKNN, PaddlePaddle, ...)
- **Processing Nodes**: Xử lý dữ liệu (tracking, OSD, transform, ...)
- **Destination Nodes**: Xuất dữ liệu (screen, RTMP, file, MQTT, Kafka, ...)

---

## Yêu cầu hệ thống

### Phụ thuộc bắt buộc

- **CMake**: >= 3.20
- **C++ Compiler**: GCC >= 7.5 hoặc Clang >= 5.0 (hỗ trợ C++17)
- **OpenCV**: >= 4.6
- **GStreamer**: >= 1.14.5

### Phụ thuộc tùy chọn (tùy theo tính năng sử dụng)

- **CUDA**: >= 10.0 (cho GPU acceleration)
- **TensorRT**: >= 7.0 (cho inference trên NVIDIA GPU)
- **PaddlePaddle**: >= 2.0 (cho Paddle inference)
- **Kafka**: (cho message broker)
- **MQTT**: libmosquitto (cho MQTT messaging)
- **OpenSSL**: (cho LLM features)
- **FFmpeg**: >= 4.0 (cho FFmpeg integration)
- **RKNN**: (cho Rockchip NPU)
- **RGA**: (cho Rockchip Graphics Accelerator)

### Kiểm tra dependencies

```bash
# Kiểm tra CMake
cmake --version

# Kiểm tra compiler
g++ --version

# Kiểm tra OpenCV
pkg-config --modversion opencv4

# Kiểm tra GStreamer
gst-launch-1.0 --version
```

---

## Cài đặt SDK

### Bước 1: Lấy mã nguồn SDK

```bash
# Clone repository
git clone <repository-url>
cd instance_pipeline
```

### Bước 2: Build và cài đặt SDK

#### Phương án 1: Cài đặt vào hệ thống (Khuyến nghị cho production)

```bash
# Build và cài đặt vào /usr/local
sudo ./build_sdk.sh --prefix=/usr/local

# Cập nhật library cache
sudo ldconfig

# Kiểm tra cài đặt
ldconfig -p | grep cvedix
```

**Ưu điểm:**
- SDK có thể được tìm thấy tự động bởi CMake
- Không cần cấu hình thêm biến môi trường
- Phù hợp cho môi trường production

**Nhược điểm:**
- Cần quyền root
- Khó quản lý nhiều phiên bản SDK

#### Phương án 2: Cài đặt vào thư mục tùy chỉnh (Khuyến nghị cho development)

```bash
# Cài đặt vào thư mục tùy chỉnh
./build_sdk.sh --prefix=/opt/cvedix

# Cấu hình biến môi trường
export CVEDIX_SDK_ROOT=/opt/cvedix
export PATH=$CVEDIX_SDK_ROOT/bin:$PATH
export LD_LIBRARY_PATH=$CVEDIX_SDK_ROOT/lib:$LD_LIBRARY_PATH
export PKG_CONFIG_PATH=$CVEDIX_SDK_ROOT/lib/pkgconfig:$PKG_CONFIG_PATH
export CMAKE_PREFIX_PATH=$CVEDIX_SDK_ROOT:$CMAKE_PREFIX_PATH
```

**Ưu điểm:**
- Không cần quyền root
- Dễ quản lý nhiều phiên bản SDK
- Phù hợp cho development và testing

**Nhược điểm:**
- Cần cấu hình biến môi trường
- Cần chỉ định đường dẫn trong CMakeLists.txt

#### Phương án 3: Cài đặt local (cho testing)

```bash
# Cài đặt vào thư mục local
./build_sdk.sh --prefix=./output

# SDK sẽ được đặt tại ./output/
```

### Bước 3: Cấu hình biến môi trường (Nếu cài đặt tùy chỉnh)

Thêm vào `~/.bashrc` hoặc `~/.profile`:

```bash
# CVEDIX SDK Configuration
export CVEDIX_SDK_ROOT=/opt/cvedix
export PATH=$CVEDIX_SDK_ROOT/bin:$PATH
export LD_LIBRARY_PATH=$CVEDIX_SDK_ROOT/lib:$LD_LIBRARY_PATH
export PKG_CONFIG_PATH=$CVEDIX_SDK_ROOT/lib/pkgconfig:$PKG_CONFIG_PATH
export CMAKE_PREFIX_PATH=$CVEDIX_SDK_ROOT:$CMAKE_PREFIX_PATH
```

Áp dụng thay đổi:

```bash
source ~/.bashrc
```

### Bước 4: Kiểm tra cài đặt

```bash
# Kiểm tra thư viện
ldconfig -p | grep cvedix

# Kiểm tra CMake config
cmake --find-package -DNAME=cvedix -DCOMPILER_ID=GNU -DLANGUAGE=CXX -DMODE=EXIST

# Kiểm tra pkg-config
pkg-config --modversion cvedix
pkg-config --cflags cvedix
pkg-config --libs cvedix

# Kiểm tra cấu trúc SDK
ls -la /opt/cvedix/include/cvedix/
ls -la /opt/cvedix/lib/
```

---

## Tích hợp SDK vào dự án

### Cấu trúc dự án mẫu

```
my_project/
├── CMakeLists.txt          # Build configuration
├── README.md               # Project documentation
├── src/
│   ├── main.cpp           # Main application
│   └── pipeline.cpp       # Pipeline implementation
├── include/
│   └── config.h           # Configuration header
├── models/                 # AI models
│   └── yolov3/
│       ├── weights
│       ├── cfg
│       └── classes.txt
└── data/                   # Data files
    └── videos/
```

---

## Các phương pháp tích hợp

### Phương pháp 1: Sử dụng CMake find_package (Khuyến nghị)

Đây là phương pháp được khuyến nghị nhất vì:
- Tự động quản lý dependencies
- Hỗ trợ version checking
- Dễ dàng maintain và update

#### CMakeLists.txt

```cmake
cmake_minimum_required(VERSION 3.20)
project(my_ai_app)

# Set C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Compile options
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra")

# Include CMakePackageConfigHelpers (required for SDK config)
include(CMakePackageConfigHelpers)

# Define check_required_components macro if not available
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

# Configure SDK path
# Option 1: Use environment variable (recommended)
if(DEFINED ENV{CVEDIX_SDK_ROOT})
    set(CMAKE_PREFIX_PATH "$ENV{CVEDIX_SDK_ROOT}" ${CMAKE_PREFIX_PATH})
endif()

# Option 2: Uncomment and set path directly
# set(CMAKE_PREFIX_PATH "/opt/cvedix" ${CMAKE_PREFIX_PATH})

# Option 3: Use command line argument
# cmake -DCMAKE_PREFIX_PATH=/opt/cvedix ..

# Find CVEDIX SDK
find_package(cvedix REQUIRED)

# Include current directory for config.h
include_directories(${CMAKE_CURRENT_SOURCE_DIR})

# Source files
set(SOURCES
    src/main.cpp
    src/pipeline.cpp
)

# Create executable
add_executable(${PROJECT_NAME} ${SOURCES})

# Link with SDK
target_link_libraries(${PROJECT_NAME} PRIVATE cvedix::cvedix_instance_sdk)

# Installation (optional)
install(TARGETS ${PROJECT_NAME}
    RUNTIME DESTINATION bin
)
```

#### Build project

```bash
mkdir build
cd build

# Nếu SDK cài đặt hệ thống
cmake ..

# Nếu SDK cài đặt tùy chỉnh
cmake -DCMAKE_PREFIX_PATH=/opt/cvedix ..

# Build
make -j$(nproc)

# Run
./my_ai_app
```

### Phương pháp 2: Sử dụng pkg-config

Phù hợp cho các dự án nhỏ hoặc khi không sử dụng CMake.

#### Build script

```bash
#!/bin/bash

# Set SDK path
export PKG_CONFIG_PATH=/opt/cvedix/lib/pkgconfig:$PKG_CONFIG_PATH

# Compile
g++ -std=c++17 \
    $(pkg-config --cflags cvedix) \
    src/main.cpp \
    -o my_app \
    $(pkg-config --libs cvedix)

# Run
./my_app
```

### Phương pháp 3: Manual linking (Không khuyến nghị)

Chỉ sử dụng khi không thể dùng CMake hoặc pkg-config.

#### CMakeLists.txt

```cmake
cmake_minimum_required(VERSION 3.10)
project(my_ai_app)

set(CMAKE_CXX_STANDARD 17)

# Set SDK path
set(CVEDIX_SDK_ROOT "/opt/cvedix")

# Include directories
include_directories(${CVEDIX_SDK_ROOT}/include)

# Link directories
link_directories(${CVEDIX_SDK_ROOT}/lib)

# Create executable
add_executable(my_app src/main.cpp)

# Link libraries
target_link_libraries(my_app PRIVATE cvedix_instance_sdk)
```

---

## Ví dụ triển khai thực tế

### Ví dụ 1: Ứng dụng phát hiện đối tượng từ file video

#### src/main.cpp

```cpp
#include "cvedix/nodes/src/cvedix_file_src_node.h"
#include "cvedix/nodes/infers/cvedix_yolo_detector_node.h"
#include "cvedix/nodes/osd/cvedix_osd_node.h"
#include "cvedix/nodes/des/cvedix_screen_des_node.h"
#include "cvedix/utils/analysis_board/cvedix_analysis_board.h"
#include <iostream>

int main(int argc, char* argv[]) {
    // Initialize logger
    CVEDIX_SET_LOG_LEVEL(cvedix_utils::cvedix_log_level::INFO);
    CVEDIX_LOGGER_INIT();
    
    // Get video path from command line or use default
    std::string video_path = (argc > 1) ? argv[1] : "./data/test_video.mp4";
    
    try {
        // Create source node - read video from file
        auto file_src = std::make_shared<cvedix_nodes::cvedix_file_src_node>(
            "file_src_0", 
            0, 
            video_path, 
            0.6  // FPS
        );
        
        // Create YOLO detector node
        auto yolo_detector = std::make_shared<cvedix_nodes::cvedix_yolo_detector_node>(
            "yolo_detector",
            "./models/yolov3.weights",
            "./models/yolov3.cfg",
            "./models/classes.txt",
            0.5,  // confidence threshold
            0.4   // NMS threshold
        );
        
        // Create OSD node to draw detection results
        auto osd = std::make_shared<cvedix_nodes::cvedix_osd_node>("osd_0");
        
        // Create screen destination node
        auto screen_des = std::make_shared<cvedix_nodes::cvedix_screen_des_node>(
            "screen_des_0", 
            0
        );
        
        // Build pipeline: file_src -> yolo_detector -> osd -> screen_des
        yolo_detector->attach_to({file_src});
        osd->attach_to({yolo_detector});
        screen_des->attach_to({osd});
        
        // Start pipeline
        file_src->start();
        
        // Display analysis board (optional, for debugging)
        cvedix_utils::cvedix_analysis_board board({file_src});
        board.display(1, false);
        
        // Wait for user input to stop
        std::cout << "Press Enter to stop..." << std::endl;
        std::string wait;
        std::getline(std::cin, wait);
        
        // Stop pipeline
        file_src->detach_recursively();
        
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
    
    return 0;
}
```

### Ví dụ 2: Ứng dụng RTSP stream với TensorRT

#### src/main.cpp

```cpp
#include "cvedix/nodes/src/cvedix_rtsp_src_node.h"
#include "cvedix/nodes/infers/cvedix_trt_yolo_detector_node.h"
#include "cvedix/nodes/track/cvedix_sort_track_node.h"
#include "cvedix/nodes/osd/cvedix_osd_node.h"
#include "cvedix/nodes/des/cvedix_screen_des_node.h"
#include "cvedix/nodes/des/cvedix_rtmp_des_node.h"
#include "cvedix/nodes/mid/cvedix_split_node.h"
#include <iostream>

int main(int argc, char* argv[]) {
    CVEDIX_SET_LOG_LEVEL(cvedix_utils::cvedix_log_level::INFO);
    CVEDIX_LOGGER_INIT();
    
    // Get RTSP URL from command line
    std::string rtsp_url = (argc > 1) ? argv[1] : "rtsp://example.com:8554/live/stream";
    std::string rtmp_url = (argc > 2) ? argv[2] : "rtmp://example.com:1935/live/stream";
    
    try {
        // RTSP source
        auto rtsp_src = std::make_shared<cvedix_nodes::cvedix_rtsp_src_node>(
            "rtsp_src_0",
            0,
            rtsp_url,
            0.6
        );
        
        // TensorRT YOLO detector
        auto trt_detector = std::make_shared<cvedix_nodes::cvedix_trt_yolo_detector_node>(
            "trt_detector",
            "./models/yolov8.engine",  // TensorRT engine file
            0.5,
            0.4
        );
        
        // SORT tracker
        auto tracker = std::make_shared<cvedix_nodes::cvedix_sort_track_node>(
            "tracker",
            cvedix_nodes::cvedix_track_for::NORMAL
        );
        
        // OSD
        auto osd = std::make_shared<cvedix_nodes::cvedix_osd_node>("osd_0");
        
        // Split node to duplicate output
        auto split = std::make_shared<cvedix_nodes::cvedix_split_node>("split_0", false, false);
        
        // Screen destination
        auto screen_des = std::make_shared<cvedix_nodes::cvedix_screen_des_node>("screen_des_0", 0);
        
        // RTMP destination
        auto rtmp_des = std::make_shared<cvedix_nodes::cvedix_rtmp_des_node>(
            "rtmp_des_0",
            0,
            rtmp_url
        );
        
        // Build pipeline
        trt_detector->attach_to({rtsp_src});
        tracker->attach_to({trt_detector});
        osd->attach_to({tracker});
        split->attach_to({osd});
        screen_des->attach_to({split});
        rtmp_des->attach_to({split});
        
        // Start
        rtsp_src->start();
        
        std::cout << "Pipeline running. Press Enter to stop..." << std::endl;
        std::string wait;
        std::getline(std::cin, wait);
        
        rtsp_src->detach_recursively();
        
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
    
    return 0;
}
```

### Ví dụ 3: Ứng dụng với cấu hình từ file

#### include/config.h

```cpp
#ifndef CONFIG_H
#define CONFIG_H

#include <string>

namespace config {
    // RTSP configuration
    const std::string RTSP_URL = "rtsp://192.168.1.100:8554/live/stream";
    const double RTSP_FPS = 0.6;
    
    // Model paths
    const std::string MODEL_WEIGHTS = "./models/yolov3.weights";
    const std::string MODEL_CFG = "./models/yolov3.cfg";
    const std::string MODEL_CLASSES = "./models/classes.txt";
    
    // Detection thresholds
    const double CONFIDENCE_THRESHOLD = 0.5;
    const double NMS_THRESHOLD = 0.4;
    
    // Output configuration
    const std::string RTMP_URL = "rtmp://example.com:1935/live/stream";
}

#endif // CONFIG_H
```

#### src/main_with_config.cpp

```cpp
#include "config.h"
#include "cvedix/nodes/src/cvedix_rtsp_src_node.h"
#include "cvedix/nodes/infers/cvedix_yolo_detector_node.h"
#include "cvedix/nodes/osd/cvedix_osd_node.h"
#include "cvedix/nodes/des/cvedix_screen_des_node.h"
#include "cvedix/nodes/des/cvedix_rtmp_des_node.h"
#include <iostream>

int main() {
    CVEDIX_SET_LOG_LEVEL(cvedix_utils::cvedix_log_level::INFO);
    CVEDIX_LOGGER_INIT();
    
    try {
        // Use configuration from config.h
        auto rtsp_src = std::make_shared<cvedix_nodes::cvedix_rtsp_src_node>(
            "rtsp_src_0",
            0,
            config::RTSP_URL,
            config::RTSP_FPS
        );
        
        auto yolo_detector = std::make_shared<cvedix_nodes::cvedix_yolo_detector_node>(
            "yolo_detector",
            config::MODEL_WEIGHTS,
            config::MODEL_CFG,
            config::MODEL_CLASSES,
            config::CONFIDENCE_THRESHOLD,
            config::NMS_THRESHOLD
        );
        
        auto osd = std::make_shared<cvedix_nodes::cvedix_osd_node>("osd_0");
        auto screen_des = std::make_shared<cvedix_nodes::cvedix_screen_des_node>("screen_des_0", 0);
        auto rtmp_des = std::make_shared<cvedix_nodes::cvedix_rtmp_des_node>(
            "rtmp_des_0",
            0,
            config::RTMP_URL
        );
        
        yolo_detector->attach_to({rtsp_src});
        osd->attach_to({yolo_detector});
        screen_des->attach_to({osd});
        rtmp_des->attach_to({osd});
        
        rtsp_src->start();
        
        std::cout << "Press Enter to stop..." << std::endl;
        std::string wait;
        std::getline(std::cin, wait);
        
        rtsp_src->detach_recursively();
        
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
    
    return 0;
}
```

---

## Best Practices

### 1. Quản lý đường dẫn SDK

**Khuyến nghị:**
- Sử dụng biến môi trường `CVEDIX_SDK_ROOT` cho development
- Cài đặt vào hệ thống (`/usr/local`) cho production
- Document rõ ràng trong README của dự án

**Tránh:**
- Hardcode đường dẫn SDK trong code
- Sử dụng đường dẫn tương đối không rõ ràng

### 2. Error Handling

```cpp
try {
    // Pipeline setup
    file_src->start();
    
    // ... processing ...
    
} catch (const std::exception& e) {
    CVEDIX_LOG_ERROR("Pipeline error: {}", e.what());
    // Cleanup
    if (file_src) {
        file_src->detach_recursively();
    }
    return 1;
}
```

### 3. Resource Management

```cpp
// Always detach pipeline before exit
void cleanup() {
    if (file_src) {
        file_src->detach_recursively();
    }
}

// Use RAII or signal handlers
std::signal(SIGINT, [](int) { cleanup(); exit(0); });
```

### 4. Logging

```cpp
// Set appropriate log level
CVEDIX_SET_LOG_LEVEL(cvedix_utils::cvedix_log_level::INFO);
CVEDIX_LOGGER_INIT();

// Use logging macros
CVEDIX_LOG_INFO("Pipeline started");
CVEDIX_LOG_ERROR("Error occurred: {}", error_msg);
```

### 5. Configuration Management

- Sử dụng file config riêng (`config.h` hoặc JSON)
- Cho phép override từ command line arguments
- Validate configuration trước khi khởi tạo pipeline

### 6. Build Configuration

```cmake
# Separate Debug and Release builds
if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    add_definitions(-DDEBUG)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g -O0")
else()
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O3 -DNDEBUG")
endif()
```

---

## Troubleshooting

### Lỗi: "Could not find a package configuration file provided by 'cvedix'"

**Nguyên nhân:** CMake không tìm thấy SDK.

**Giải pháp:**

1. Kiểm tra SDK đã được cài đặt:
   ```bash
   ls /usr/local/lib/cmake/cvedix/
   # hoặc
   ls /opt/cvedix/lib/cmake/cvedix/
   ```

2. Chỉ định đường dẫn SDK:
   ```bash
   cmake -DCMAKE_PREFIX_PATH=/opt/cvedix ..
   ```

3. Kiểm tra biến môi trường:
   ```bash
   echo $CMAKE_PREFIX_PATH
   echo $CVEDIX_SDK_ROOT
   ```

### Lỗi: "Cannot find cvedix::cvedix_instance_sdk"

**Nguyên nhân:** Thư viện SDK không được tìm thấy.

**Giải pháp:**

1. Kiểm tra thư viện:
   ```bash
   ldconfig -p | grep cvedix
   ```

2. Cập nhật library cache:
   ```bash
   sudo ldconfig
   ```

3. Kiểm tra LD_LIBRARY_PATH:
   ```bash
   export LD_LIBRARY_PATH=/opt/cvedix/lib:$LD_LIBRARY_PATH
   ```

### Lỗi: "Cannot find header file cvedix/..."

**Nguyên nhân:** Header files không được tìm thấy.

**Giải pháp:**

1. Kiểm tra header files:
   ```bash
   ls /usr/local/include/cvedix/
   ```

2. Đảm bảo CMAKE_PREFIX_PATH đúng:
   ```bash
   cmake -DCMAKE_PREFIX_PATH=/path/to/sdk/install ..
   ```

### Lỗi runtime: "cannot open shared object file"

**Nguyên nhân:** Runtime không tìm thấy shared library.

**Giải pháp:**

1. Kiểm tra LD_LIBRARY_PATH:
   ```bash
   export LD_LIBRARY_PATH=/opt/cvedix/lib:$LD_LIBRARY_PATH
   ```

2. Hoặc copy library vào system path:
   ```bash
   sudo cp /opt/cvedix/lib/libcvedix_instance_sdk.so* /usr/local/lib/
   sudo ldconfig
   ```

3. Hoặc sử dụng rpath trong CMake:
   ```cmake
   set(CMAKE_BUILD_RPATH /opt/cvedix/lib)
   set(CMAKE_INSTALL_RPATH /opt/cvedix/lib)
   ```

### Lỗi: "Unknown CMake command 'check_required_components'"

**Giải pháp:**

Đảm bảo CMakeLists.txt đã include module cần thiết:

```cmake
include(CMakePackageConfigHelpers)
```

Và định nghĩa macro backup nếu cần (xem ví dụ CMakeLists.txt ở trên).

---

## Kiểm tra và Debug

### 1. Kiểm tra SDK installation

```bash
# Kiểm tra thư viện
ldconfig -p | grep cvedix

# Kiểm tra CMake config
cmake --find-package -DNAME=cvedix -DCOMPILER_ID=GNU -DLANGUAGE=CXX -DMODE=EXIST

# Kiểm tra pkg-config
pkg-config --modversion cvedix
pkg-config --cflags cvedix
pkg-config --libs cvedix
```

### 2. Sử dụng Analysis Board

```cpp
// Enable analysis board for debugging
cvedix_utils::cvedix_analysis_board board({file_src});
board.display(1, false);  // Display every 1 second
```

### 3. Enable verbose logging

```cpp
CVEDIX_SET_LOG_LEVEL(cvedix_utils::cvedix_log_level::DEBUG);
CVEDIX_LOGGER_INIT();
```

### 4. Build với debug symbols

```bash
cmake -DCMAKE_BUILD_TYPE=Debug ..
make
gdb ./my_app
```

### 5. Kiểm tra dependencies

```bash
# Kiểm tra dependencies của executable
ldd ./my_app | grep cvedix

# Kiểm tra missing dependencies
ldd ./my_app | grep "not found"
```

---

## Tài liệu tham khảo

- [README.md](./README.md) - Tài liệu chính của dự án
- [README_SDK.md](./README_SDK.md) - Tài liệu SDK
- [BUILD.md](./BUILD.md) - Hướng dẫn build chi tiết
- [INSTALL_SDK.md](./INSTALL_SDK.md) - Hướng dẫn cài đặt SDK
- [samples/README.md](./samples/README.md) - Các ví dụ mẫu

---

## Hỗ trợ

Nếu gặp vấn đề khi triển khai:

1. Kiểm tra tài liệu và examples trong thư mục `samples/`
2. Xem các issues trên repository
3. Tạo issue mới với thông tin chi tiết:
   - Phiên bản SDK
   - Hệ điều hành và compiler
   - Logs và error messages
   - Steps to reproduce

---

**Chúc bạn triển khai thành công!**

