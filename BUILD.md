# Hướng dẫn Build Ứng dụng

## Yêu cầu

- CMake 3.10 trở lên
- C++17 compiler (g++ hoặc clang++)
- CVEDIX SDK đã được cài đặt (xem `INSTALL_SDK.md`)

## Các bước Build

### Bước 1: Cài đặt CVEDIX SDK

Xem chi tiết trong `INSTALL_SDK.md`. Đảm bảo SDK đã được cài đặt và có thể tìm thấy bởi CMake.

### Bước 2: Cấu hình đường dẫn SDK (nếu cần)

**Nếu SDK cài đặt vào hệ thống (`/usr/local`):**
```bash
# Không cần cấu hình gì thêm, CMake sẽ tự động tìm thấy
```

**Nếu SDK cài đặt vào thư mục tùy chỉnh:**
```bash
# Cách 1: Sử dụng biến môi trường
export CMAKE_PREFIX_PATH=/opt/cvedix:$CMAKE_PREFIX_PATH

# Cách 2: Chỉ định khi chạy cmake
cmake -DCMAKE_PREFIX_PATH=/opt/cvedix ..
```

**Nếu SDK ở thư mục local:**
```bash
# Chỉnh sửa CMakeLists.txt, uncomment và sửa dòng:
# set(CMAKE_PREFIX_PATH "/path/to/sdk/install" ${CMAKE_PREFIX_PATH})
```

### Bước 3: Build

```bash
# Tạo thư mục build
mkdir -p build
cd build

# Cấu hình với CMake
cmake ..

# Build
make -j$(nproc)

# Hoặc build với số luồng cụ thể
make -j4
```

### Bước 4: Kiểm tra

```bash
# Kiểm tra file executable đã được tạo
ls -lh rtsp_ba_crossline_app

# Chạy thử (nếu có RTSP stream)
./rtsp_ba_crossline_app
```

## Xử lý lỗi Build

### Lỗi: "Could not find a package configuration file provided by 'cvedix'"

**Nguyên nhân**: CMake không tìm thấy SDK.

**Giải pháp**:
1. Kiểm tra SDK đã được cài đặt:
   ```bash
   ls /usr/local/lib/cmake/cvedix/  # hoặc /opt/cvedix/lib/cmake/cvedix/
   ```

2. Chỉ định đường dẫn SDK:
   ```bash
   cmake -DCMAKE_PREFIX_PATH=/path/to/sdk/install ..
   ```

3. Kiểm tra biến môi trường:
   ```bash
   echo $CMAKE_PREFIX_PATH
   ```

### Lỗi: "Cannot find cvedix::cvedix_instance_sdk"

**Nguyên nhân**: Thư viện SDK không được tìm thấy.

**Giải pháp**:
1. Kiểm tra thư viện đã được cài đặt:
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

**Nguyên nhân**: Header files không được tìm thấy.

**Giải pháp**:
1. Kiểm tra header files:
   ```bash
   ls /usr/local/include/cvedix/  # hoặc /opt/cvedix/include/cvedix/
   ```

2. Đảm bảo CMAKE_PREFIX_PATH đúng:
   ```bash
   cmake -DCMAKE_PREFIX_PATH=/path/to/sdk/install ..
   ```

### Lỗi: "Unknown CMake command 'check_required_components'"

**Nguyên nhân**: File config của SDK sử dụng macro `check_required_components` nhưng CMake không tìm thấy macro này.

**Giải pháp**:
1. Đảm bảo CMakeLists.txt đã include module cần thiết (đã được thêm tự động):
   ```cmake
   include(CMakePackageConfigHelpers)
   ```

2. Nếu vẫn lỗi, cập nhật CMakeLists.txt với phiên bản mới nhất (đã có định nghĩa macro backup).

3. Kiểm tra phiên bản CMake:
   ```bash
   cmake --version
   ```
   Yêu cầu CMake >= 3.20 (đã được cấu hình trong CMakeLists.txt).

## Build với các tùy chọn

### Build Release mode

```bash
cmake -DCMAKE_BUILD_TYPE=Release ..
make -j$(nproc)
```

### Build Debug mode

```bash
cmake -DCMAKE_BUILD_TYPE=Debug ..
make -j$(nproc)
```

### Build với verbose output

```bash
make VERBOSE=1
```

### Clean build

```bash
cd build
rm -rf *
cmake ..
make
```

## Cài đặt ứng dụng (Optional)

```bash
cd build
sudo make install
# Ứng dụng sẽ được cài vào /usr/local/bin/rtsp_ba_crossline_app
```

## Kiểm tra SDK đã được cài đặt đúng

```bash
# Kiểm tra thư viện
ldconfig -p | grep cvedix

# Kiểm tra CMake config
cmake --find-package -DNAME=cvedix -DCOMPILER_ID=GNU -DLANGUAGE=CXX -DMODE=EXIST

# Kiểm tra pkg-config (nếu có)
pkg-config --modversion cvedix
```

