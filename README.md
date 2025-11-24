# RTSP Behavior Analysis Crossline Detection Application

Ứng dụng phát hiện hành vi vượt đường (crossline detection) sử dụng SDK cvedix-ai-runtime với đầu vào RTSP và đầu ra RTMP.

## Mô tả

Ứng dụng này triển khai một pipeline phân tích hành vi để phát hiện đối tượng vượt qua đường kẻ định trước. Pipeline bao gồm:

1. **RTSP Source Node**: Nhận video stream từ RTSP
2. **YOLO Detector Node**: Phát hiện đối tượng sử dụng mô hình YOLO
3. **SORT Tracker Node**: Theo dõi đối tượng qua các frame
4. **BA Crossline Node**: Phân tích hành vi vượt đường
5. **OSD Node**: Hiển thị thông tin trên video
6. **Screen Destination Node**: Hiển thị video trên màn hình
7. **RTMP Destination Node**: Phát video stream qua RTMP

## Yêu cầu

- CMake 3.10 trở lên
- C++17 compiler
- cvedix-ai-runtime SDK đã được cài đặt
- Mô hình YOLO tại đường dẫn: `./cvedix_data/models/det_cls/`
  - `yolov3-tiny-2022-0721_best.weights`
  - `yolov3-tiny-2022-0721.cfg`
  - `yolov3_tiny_5classes.txt`

## Cấu trúc thư mục

```
edge_ai_sdk/
├── CMakeLists.txt          # Build configuration
├── README.md               # Documentation
├── BUILD.md                # Chi tiết hướng dẫn build
├── INSTALL_SDK.md          # Hướng dẫn cài đặt SDK
├── .gitignore              # Git ignore file
├── include/
│   └── config.h           # Configuration header
└── src/
    ├── main.cpp           # Main application
    └── main_with_config.cpp # Alternative main using config.h
```

## Cài đặt và Build

### 1. Cài đặt CVEDIX SDK

Trước tiên, bạn cần cài đặt CVEDIX SDK. Xem chi tiết trong file `INSTALL_SDK.md`.

**Tùy chọn 1: Cài đặt vào hệ thống (khuyến nghị)**
```bash
sudo ./build_sdk.sh --prefix=/usr/local
sudo ldconfig
```

**Tùy chọn 2: Cài đặt vào thư mục tùy chỉnh**
```bash
./build_sdk.sh --prefix=/opt/cvedix
export CMAKE_PREFIX_PATH=/opt/cvedix:$CMAKE_PREFIX_PATH
```

**Tùy chọn 3: Sử dụng SDK từ thư mục local**
```bash
# Chỉ định đường dẫn SDK khi chạy cmake
cmake -DCMAKE_PREFIX_PATH=/path/to/sdk/install ..
```

### 2. Cấu hình biến môi trường (Tùy chọn)

Nếu SDK được cài đặt vào thư mục tùy chỉnh, thêm vào `~/.bashrc`:

```bash
export CVEDIX_SDK_ROOT=/opt/cvedix
export CMAKE_PREFIX_PATH=$CVEDIX_SDK_ROOT:$CMAKE_PREFIX_PATH
export LD_LIBRARY_PATH=$CVEDIX_SDK_ROOT/lib:$LD_LIBRARY_PATH
```

Sau đó:
```bash
source ~/.bashrc
```

### 3. Build ứng dụng

```bash
mkdir build
cd build
cmake ..
make
```

**Lưu ý**: Nếu SDK không được tìm thấy, bạn có thể chỉ định đường dẫn trực tiếp:

```bash
cmake -DCMAKE_PREFIX_PATH=/path/to/sdk/install ..
make
```

**Xem thêm**: Chi tiết về build và xử lý lỗi trong file `BUILD.md`.

### 3. Chạy ứng dụng

```bash
# Sử dụng URL mặc định
./rtsp_ba_crossline_app

# Hoặc chỉ định RTSP và RTMP URL
./rtsp_ba_crossline_app <rtsp_url> <rtmp_url>
```

Ví dụ:
```bash
./rtsp_ba_crossline_app \
    "rtsp://example.com:8554/live/camera" \
    "rtmp://example.com:1935/live/stream"
```

## Cấu hình

### Điều chỉnh đường kẻ phát hiện

Chỉnh sửa tọa độ đường kẻ trong `src/main.cpp` hoặc `include/config.h`:

```cpp
cvedix_objects::cvedix_point line_start(0, 250);  // Điểm bắt đầu
cvedix_objects::cvedix_point line_end(700, 220);   // Điểm kết thúc
```

**Lưu ý**: Tọa độ phải nằm trong phạm vi kích thước frame của video.

### Điều chỉnh FPS

Thay đổi giá trị `rtsp_fps` trong code:

```cpp
double rtsp_fps = 0.6;  // FPS của RTSP stream
```

### Đường dẫn mô hình YOLO

Đảm bảo các file mô hình YOLO nằm đúng đường dẫn:
- `./cvedix_data/models/det_cls/yolov3-tiny-2022-0721_best.weights`
- `./cvedix_data/models/det_cls/yolov3-tiny-2022-0721.cfg`
- `./cvedix_data/models/det_cls/yolov3_tiny_5classes.txt`

## Sử dụng

1. Khởi động ứng dụng
2. Ứng dụng sẽ:
   - Kết nối đến RTSP stream
   - Phát hiện và theo dõi đối tượng
   - Phân tích hành vi vượt đường
   - Hiển thị kết quả trên màn hình
   - Phát stream qua RTMP
3. Nhấn Enter để dừng ứng dụng

## Debug

Ứng dụng sử dụng `cvedix_analysis_board` để hiển thị thông tin debug. Bảng phân tích sẽ hiển thị thông tin về các node trong pipeline.

## Xử lý lỗi

Ứng dụng sử dụng try-catch để xử lý các lỗi runtime. Các lỗi phổ biến:

- **Lỗi kết nối RTSP**: Kiểm tra URL và kết nối mạng
- **Lỗi tải mô hình**: Kiểm tra đường dẫn và file mô hình YOLO
- **Lỗi RTMP**: Kiểm tra URL và quyền truy cập RTMP server

## Ghi chú

- Đảm bảo camera RTSP đang hoạt động trước khi chạy ứng dụng
- RTMP server phải sẵn sàng nhận stream
- Tọa độ đường kẻ cần được điều chỉnh phù hợp với góc nhìn camera

## License

[Thêm thông tin license nếu cần]

