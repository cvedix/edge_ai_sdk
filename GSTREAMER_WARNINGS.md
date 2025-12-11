# Xử lý GStreamer CRITICAL Warnings

## Vấn đề

Khi chạy ứng dụng, bạn có thể thấy các warnings từ GStreamer:

```
(rtsp_ba_crossline_app:393466): GStreamer-CRITICAL **: gst_caps_get_structure: assertion 'GST_IS_CAPS (caps)' failed
(rtsp_ba_crossline_app:393466): GStreamer-CRITICAL **: gst_structure_get_int: assertion 'structure != NULL' failed
```

## Nguyên nhân

Các warnings này xảy ra khi:
- GStreamer pipeline đang được khởi tạo
- Code cố gắng truy cập caps trước khi pipeline sẵn sàng
- Đây là warnings từ SDK khi khởi tạo RTSP source node

**Lưu ý:** Những warnings này không ảnh hưởng đến functionality. Ứng dụng vẫn chạy bình thường và RTSP connection vẫn hoạt động.

## Giải pháp

### Cách 1: Suppress warnings bằng environment variable (Khuyến nghị)

Trước khi chạy ứng dụng:

```bash
export GST_DEBUG=1  # Chỉ hiển thị WARNING và ERROR, suppress CRITICAL
./rtsp_ba_crossline_app
```

Hoặc trong code (đã được thêm vào main.cpp):

```cpp
if (!getenv("GST_DEBUG")) {
    setenv("GST_DEBUG", "1", 0);
}
```

### Cách 2: Suppress tất cả GStreamer output

```bash
export GST_DEBUG=0  # Chỉ hiển thị ERROR
# hoặc
export GST_DEBUG=-1  # Không hiển thị gì cả
./rtsp_ba_crossline_app
```

### Cách 3: Redirect stderr (Nếu chỉ muốn ẩn warnings)

```bash
./rtsp_ba_crossline_app 2>/dev/null
# hoặc redirect vào file
./rtsp_ba_crossline_app 2>gstreamer_warnings.log
```

### Cách 4: Thêm vào script wrapper

Cập nhật `run_app.sh`:

```bash
#!/bin/bash
export LD_LIBRARY_PATH=/opt/cvedix/lib:${LD_LIBRARY_PATH}
export GST_DEBUG=1  # Suppress CRITICAL warnings
cd build
exec ./rtsp_ba_crossline_app "$@"
```

## GStreamer Debug Levels

- `-1`: No output
- `0`: ERROR only
- `1`: WARNING and above (khuyến nghị để suppress CRITICAL)
- `2`: FIXME and above
- `3`: INFO and above
- `4`: DEBUG and above
- `5`: LOG and above
- `6`: TRACE and above

## Kiểm tra

Sau khi set GST_DEBUG=1, warnings CRITICAL sẽ không còn xuất hiện:

```bash
export GST_DEBUG=1
./rtsp_ba_crossline_app
# Sẽ không còn thấy CRITICAL warnings
```

## Lưu ý

- Warnings này không ảnh hưởng đến functionality
- RTSP connection vẫn hoạt động bình thường
- Có thể bỏ qua nếu ứng dụng chạy ổn định
- Nếu muốn debug GStreamer issues, set `GST_DEBUG=4` để xem chi tiết
