# Cách Export CVEDIX Library Path

Để các executable có thể tìm thấy `libtinyexpr.so` và các libraries khác từ CVEDIX SDK, bạn có thể sử dụng một trong các cách sau:

## Cách 1: Export trong shell hiện tại (Tạm thời)

```bash
export LD_LIBRARY_PATH=/opt/cvedix/lib:$LD_LIBRARY_PATH
./rtsp_ba_crossline_app
```

**Lưu ý:** Cách này chỉ có hiệu lực trong shell hiện tại. Khi đóng shell, biến sẽ mất.

## Cách 2: Thêm vào ~/.bashrc (Permanent cho user)

Thêm dòng sau vào file `~/.bashrc`:

```bash
# CVEDIX SDK Library Path
export LD_LIBRARY_PATH=/opt/cvedix/lib:$LD_LIBRARY_PATH
```

Sau đó reload:
```bash
source ~/.bashrc
```

Hoặc đăng nhập lại terminal.

## Cách 3: Thêm vào ldconfig (System-wide, Permanent)

Cách này yêu cầu quyền root nhưng sẽ áp dụng cho tất cả users:

```bash
# Tạo file config cho ldconfig
sudo sh -c 'echo "/opt/cvedix/lib" > /etc/ld.so.conf.d/cvedix.conf'

# Cập nhật library cache
sudo ldconfig

# Kiểm tra
ldconfig -p | grep cvedix
```

Sau đó bạn có thể chạy trực tiếp mà không cần set LD_LIBRARY_PATH:
```bash
./rtsp_ba_crossline_app
```

## Cách 4: Sử dụng script wrapper

Sử dụng script `run_samples.sh` đã được tạo:

```bash
# Cho samples
./run_samples.sh mqtt_json_receiver_sample localhost 1883 events

# Cho executable chính, bạn có thể tạo wrapper tương tự hoặc:
cd build
LD_LIBRARY_PATH=/opt/cvedix/lib:$LD_LIBRARY_PATH ./rtsp_ba_crossline_app
```

## Cách 5: Tạo wrapper script cho executable chính

Tạo file `run_app.sh`:

```bash
#!/bin/bash
export LD_LIBRARY_PATH=/opt/cvedix/lib:$LD_LIBRARY_PATH
cd build
exec ./rtsp_ba_crossline_app "$@"
```

Sau đó:
```bash
chmod +x run_app.sh
./run_app.sh
```

## Kiểm tra

Sau khi export, kiểm tra bằng:

```bash
# Kiểm tra LD_LIBRARY_PATH
echo $LD_LIBRARY_PATH

# Kiểm tra library được tìm thấy
ldd build/rtsp_ba_crossline_app | grep tinyexpr
# Nên thấy: libtinyexpr.so => /opt/cvedix/lib/libtinyexpr.so

# Chạy thử
LD_LIBRARY_PATH=/opt/cvedix/lib:$LD_LIBRARY_PATH ./build/rtsp_ba_crossline_app
```

## Khuyến nghị

- **Development:** Sử dụng Cách 2 (thêm vào ~/.bashrc)
- **Production:** Sử dụng Cách 3 (ldconfig) hoặc Cách 5 (wrapper script)
- **Testing nhanh:** Sử dụng Cách 1 (export trong shell)
