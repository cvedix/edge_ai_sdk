# Hướng dẫn chạy Samples

## Vấn đề về Library Path

Một số samples cần `libtinyexpr.so` và các libraries khác từ SDK. Có 3 cách để giải quyết:

### Cách 1: Sử dụng script wrapper (Khuyến nghị)

```bash
./run_samples.sh <sample_name> [arguments...]

# Ví dụ:
./run_samples.sh mqtt_json_receiver_sample localhost 1883 events
./run_samples.sh simple_rtmp_mqtt_sample ./video.mp4 rtmp://localhost:1935/live/test
```

### Cách 2: Set LD_LIBRARY_PATH

```bash
export LD_LIBRARY_PATH=/opt/cvedix/lib:$LD_LIBRARY_PATH
cd build/bin
./mqtt_json_receiver_sample [arguments...]
```

### Cách 3: Thêm vào ldconfig (Permanent)

```bash
# Tạo file config cho ldconfig
sudo sh -c 'echo "/opt/cvedix/lib" > /etc/ld.so.conf.d/cvedix.conf'
sudo ldconfig

# Sau đó có thể chạy trực tiếp
cd build/bin
./mqtt_json_receiver_sample [arguments...]
```

## Danh sách Samples

Để xem danh sách samples có sẵn:

```bash
./run_samples.sh
# hoặc
ls build/bin/*_sample
```
