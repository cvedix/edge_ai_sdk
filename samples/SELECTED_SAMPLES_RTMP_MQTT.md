# Các Sample Được Chọn: RTMP và MQTT

Tài liệu này mô tả các sample được chọn để triển khai các tính năng:
- **Gửi RTMP lên server**
- **Gửi xử lý event thông qua MQTT**
- **Chạy cơ bản, dễ hiểu**

---

## 1. Sample Cơ Bản: RTMP Output (1-1-N_sample.cpp)

### Mô tả
Sample đơn giản nhất để gửi video stream lên RTMP server. Pipeline: File Source → Face Detection → OSD → Screen + RTMP Output.

### Đặc điểm
- ✅ Gửi RTMP lên server
- ✅ Đơn giản, dễ hiểu
- ❌ Chưa có MQTT event

### Yêu cầu
- OpenCV
- Face detection model (YuNet)
- RTMP server

### Cách chạy

```bash
# Build
cd build
cmake ..
make 1-1-N_sample

# Chạy (cần chỉnh sửa RTMP URL trong code)
./1-1-N_sample
```

### Code chính

```cpp
// File source
auto file_src_0 = std::make_shared<cvedix_nodes::cvedix_file_src_node>(
    "file_src_0", 0, "./cvedix_data/test_video/face.mp4", 0.8);

// Face detector
auto yunet_face_detector_0 = std::make_shared<cvedix_nodes::cvedix_yunet_face_detector_node>(
    "yunet_face_detector_0", "./cvedix_data/models/face/face_detection_yunet_2022mar.onnx");

// OSD
auto osd_0 = std::make_shared<cvedix_nodes::cvedix_face_osd_node_v2>("osd_0");

// RTMP output
auto rtmp_des_0 = std::make_shared<cvedix_nodes::cvedix_rtmp_des_node>(
    "rtmp_des_0", 0, "rtmp://your-server.com:1935/live/stream");

// Pipeline
yunet_face_detector_0->attach_to({file_src_0});
osd_0->attach_to({yunet_face_detector_0});
screen_des_0->attach_to({osd_0});
rtmp_des_0->attach_to({osd_0});  // Auto split
```

### Cấu hình RTMP URL

Chỉnh sửa dòng trong code:
```cpp
auto rtmp_des_0 = std::make_shared<cvedix_nodes::cvedix_rtmp_des_node>(
    "rtmp_des_0", 0, "rtmp://YOUR_SERVER:1935/live/YOUR_STREAM_KEY");
```

---

## 2. Sample Đầy Đủ: RTMP + MQTT (face_tracking_rtsp_sample.cpp)

### Mô tả
Sample đầy đủ với RTSP input, face tracking, gửi RTMP stream và MQTT events. Pipeline: RTSP Source → Face Detection → Tracking → MQTT Broker → OSD → Screen + RTMP.

### Đặc điểm
- ✅ Gửi RTMP lên server
- ✅ Gửi events qua MQTT
- ✅ Face tracking với SORT
- ✅ Crop images và gửi qua MQTT
- ⚠️ Phức tạp hơn, cần RTSP input

### Yêu cầu
- RTSP camera/stream
- MQTT broker (mosquitto)
- Build với `-DCVEDIX_WITH_MQTT=ON`
- Face detection model

### Cách chạy

```bash
# Build với MQTT support
cd build
cmake -DCVEDIX_WITH_MQTT=ON ..
make face_tracking_rtsp_sample

# Chạy
./face_tracking_rtsp_sample
```

### Cấu hình MQTT và RTMP

Chỉnh sửa trong code:

```cpp
// MQTT configuration
std::string mqtt_broker = "anhoidong.datacenter.cvedix.com";
int mqtt_port = 1883;
std::string mqtt_topic = "events";
std::string mqtt_username = "";
std::string mqtt_password = "";

// RTMP configuration
auto rtmp_des_0 = std::make_shared<cvedix_nodes::cvedix_rtmp_des_node>(
    "rtmp_des_0", 0, "rtmp://your-server.com:1935/live/stream",
    cvedix_objects::cvedix_size{1280, 720}, 2048);
```

### Code chính

```cpp
// RTSP source
auto rtsp_src_0 = std::make_shared<cvedix_nodes::cvedix_rtsp_src_node>(
    "rtsp_src_0", 0, "rtsp://camera-ip:8554/live/stream", 0.6);

// Face detector
auto yunet_face_detector_0 = std::make_shared<cvedix_nodes::cvedix_yunet_face_detector_node>(
    "yunet_face_detector_0", "./cvedix_data/models/face/face_detection_yunet_2022mar.onnx");

// Tracker
auto track_0 = std::make_shared<cvedix_nodes::cvedix_sort_track_node>("track_0");

#ifdef CVEDIX_WITH_MQTT
// MQTT broker - gửi events với crop images
auto enhanced_mqtt_broker_0 = std::make_shared<cvedix_json_enhanced_mqtt_broker_node>(
    "enhanced_mqtt_broker_0",
    cvedix_nodes::cvedix_broke_for::FACE,
    100, 500, false, mqtt_publish_func);
#endif

// RTMP output
auto rtmp_des_0 = std::make_shared<cvedix_nodes::cvedix_rtmp_des_node>(
    "rtmp_des_0", 0, "rtmp://server.com:1935/live/stream");

// Pipeline
yunet_face_detector_0->attach_to({rtsp_src_0});
track_0->attach_to({yunet_face_detector_0});
#ifdef CVEDIX_WITH_MQTT
enhanced_mqtt_broker_0->attach_to({track_0});
osd_0->attach_to({enhanced_mqtt_broker_0});
#else
osd_0->attach_to({track_0});
#endif
rtmp_des_0->attach_to({osd_0});
```

---

## 3. Sample Đơn Giản Mới: RTMP + MQTT Basic (simple_rtmp_mqtt_sample.cpp)

### Mô tả
Sample đơn giản nhất kết hợp cả RTMP streaming và MQTT event publishing. Được thiết kế để dễ hiểu và dễ tùy chỉnh.

### Đặc điểm
- ✅ Gửi RTMP lên server
- ✅ Gửi events qua MQTT khi có khuôn mặt mới
- ✅ Face tracking với SORT
- ✅ Đơn giản, dễ hiểu và tùy chỉnh
- ✅ Hỗ trợ command line arguments
- ✅ Graceful shutdown với signal handling

### Yêu cầu
- Build với `-DCVEDIX_WITH_MQTT=ON`
- MQTT broker (mosquitto)
- RTMP server
- Face detection model (YuNet)

### Cách chạy

```bash
# 1. Build với MQTT support
cd build
cmake -DCVEDIX_WITH_MQTT=ON ..
make simple_rtmp_mqtt_sample

# 2. Chạy với arguments
./simple_rtmp_mqtt_sample [video_file] [rtmp_url] [mqtt_broker] [mqtt_port] [mqtt_topic]

# Ví dụ:
./simple_rtmp_mqtt_sample \
    ./cvedix_data/test_video/face.mp4 \
    rtmp://localhost:1935/live/test \
    localhost \
    1883 \
    events
```

### Code chính

```cpp
// File source
auto file_src = std::make_shared<cvedix_nodes::cvedix_file_src_node>(
    "file_src_0", 0, video_file, 0.6);

// Face detector
auto face_detector = std::make_shared<cvedix_nodes::cvedix_yunet_face_detector_node>(
    "yunet_face_detector_0", 
    "./cvedix_data/models/face/face_detection_yunet_2022mar.onnx");

// Tracker
auto tracker = std::make_shared<cvedix_nodes::cvedix_sort_track_node>(
    "tracker_0", cvedix_nodes::cvedix_track_for::FACE);

#ifdef CVEDIX_WITH_MQTT
// MQTT client
auto mqtt_client = std::make_shared<cvedix_utils::cvedix_mqtt_client>(
    mqtt_broker, mqtt_port, "simple_rtmp_mqtt_sample");
mqtt_client->connect(mqtt_username, mqtt_password);

// MQTT broker node
auto mqtt_broker_node = std::make_shared<cvedix_nodes::cvedix_json_enhanced_mqtt_broker_node>(
    "mqtt_broker_0",
    cvedix_nodes::cvedix_broke_for::FACE,
    100, 500, false, mqtt_publish_func);
#endif

// OSD
auto osd = std::make_shared<cvedix_nodes::cvedix_face_osd_node_v2>("osd_0");

// RTMP output
auto rtmp_des = std::make_shared<cvedix_nodes::cvedix_rtmp_des_node>(
    "rtmp_des_0", 0, rtmp_url,
    cvedix_objects::cvedix_size{1280, 720}, 2048);

// Pipeline
face_detector->attach_to({file_src});
tracker->attach_to({face_detector});
#ifdef CVEDIX_WITH_MQTT
mqtt_broker_node->attach_to({tracker});
osd->attach_to({mqtt_broker_node});
#else
osd->attach_to({tracker});
#endif
rtmp_des->attach_to({osd});
```

### Cấu hình

Chỉnh sửa trong code hoặc dùng command line arguments:

```cpp
// Mặc định
std::string video_file = "./cvedix_data/test_video/face.mp4";
std::string rtmp_url = "rtmp://localhost:1935/live/test";
std::string mqtt_broker = "localhost";
int mqtt_port = 1883;
std::string mqtt_topic = "events";
```

### Ưu điểm của sample này

1. **Đơn giản**: Code dễ đọc và hiểu
2. **Linh hoạt**: Hỗ trợ cả có và không có MQTT
3. **Dễ tùy chỉnh**: Có thể thay đổi input (file → RTSP) dễ dàng
4. **Error handling**: Xử lý lỗi kết nối MQTT gracefully
5. **Signal handling**: Hỗ trợ Ctrl+C để dừng pipeline

---

## So Sánh Các Sample

| Sample | RTMP | MQTT | Độ phức tạp | Input | Use Case |
|--------|------|------|-------------|-------|----------|
| **1-1-N_sample** | ✅ | ❌ | ⭐⭐ | File | Test RTMP cơ bản |
| **simple_rtmp_mqtt_sample** | ✅ | ✅ | ⭐⭐⭐ | File | **Khuyến nghị: Development & Testing** |
| **face_tracking_rtsp_sample** | ✅ | ✅ | ⭐⭐⭐⭐ | RTSP | Production với events |

---

## Hướng Dẫn Chạy Nhanh

### Sample 1: RTMP Cơ Bản (1-1-N_sample)

```bash
# 1. Build
cd build
cmake ..
make 1-1-N_sample

# 2. Chỉnh sửa RTMP URL trong code
# 3. Chạy
./1-1-N_sample
```

### Sample 2: RTMP + MQTT Đơn Giản (simple_rtmp_mqtt_sample) ⭐ Khuyến nghị

```bash
# 1. Cài đặt MQTT library
sudo apt-get install libmosquitto-dev

# 2. Build với MQTT
cd build
cmake -DCVEDIX_WITH_MQTT=ON ..
make simple_rtmp_mqtt_sample

# 3. Chạy với arguments
./simple_rtmp_mqtt_sample \
    ./cvedix_data/test_video/face.mp4 \
    rtmp://localhost:1935/live/test \
    localhost \
    1883 \
    events

# Hoặc chạy với giá trị mặc định
./simple_rtmp_mqtt_sample
```

### Sample 3: RTMP + MQTT Production (face_tracking_rtsp_sample)

```bash
# 1. Cài đặt MQTT library
sudo apt-get install libmosquitto-dev

# 2. Build với MQTT
cd build
cmake -DCVEDIX_WITH_MQTT=ON ..
make face_tracking_rtsp_sample

# 3. Cấu hình MQTT broker và RTMP URL trong code
# 4. Chạy
./face_tracking_rtsp_sample
```

---

## Cấu Hình RTMP Server

### Sử dụng Nginx-RTMP

```bash
# Cài đặt nginx với RTMP module
sudo apt-get install nginx libnginx-mod-rtmp

# Cấu hình /etc/nginx/nginx.conf
rtmp {
    server {
        listen 1935;
        application live {
            live on;
            allow publish all;
            allow play all;
        }
    }
}

# Khởi động
sudo systemctl restart nginx
```

### Test RTMP Stream

```bash
# Publish test stream
ffmpeg -re -i test_video.mp4 -c copy -f flv rtmp://localhost:1935/live/test

# Play stream
ffplay rtmp://localhost:1935/live/test
```

---

## Cấu Hình MQTT Broker

### Sử dụng Mosquitto

```bash
# Cài đặt
sudo apt-get install mosquitto mosquitto-clients

# Khởi động
sudo systemctl start mosquitto

# Test publish/subscribe
mosquitto_pub -h localhost -t test -m "Hello"
mosquitto_sub -h localhost -t test
```

### Cấu hình Authentication (Optional)

```bash
# Tạo password file
mosquitto_passwd -c /etc/mosquitto/passwd username

# Cấu hình /etc/mosquitto/mosquitto.conf
allow_anonymous false
password_file /etc/mosquitto/passwd
```

---

## Troubleshooting

### RTMP Connection Failed

1. Kiểm tra RTMP server đang chạy:
   ```bash
   netstat -tlnp | grep 1935
   ```

2. Kiểm tra firewall:
   ```bash
   sudo ufw allow 1935/tcp
   ```

3. Test với ffmpeg:
   ```bash
   ffmpeg -re -i test.mp4 -c copy -f flv rtmp://server:1935/live/test
   ```

### MQTT Connection Failed

1. Kiểm tra MQTT broker:
   ```bash
   sudo systemctl status mosquitto
   ```

2. Test connection:
   ```bash
   mosquitto_sub -h localhost -t test -v
   ```

3. Kiểm tra port:
   ```bash
   netstat -tlnp | grep 1883
   ```

### Build Errors

1. MQTT not found:
   ```bash
   sudo apt-get install libmosquitto-dev
   cmake -DCVEDIX_WITH_MQTT=ON ..
   ```

2. Library not found:
   ```bash
   export LD_LIBRARY_PATH=/opt/cvedix/lib:$LD_LIBRARY_PATH
   ```

---

## Next Steps

1. **Bắt đầu với Sample 1** (1-1-N_sample) để test RTMP
2. **Chuyển sang Sample 2** (face_tracking_rtsp_sample) khi cần MQTT events
3. **Tùy chỉnh** theo nhu cầu của bạn

---

**Lưu ý:** Đảm bảo đã cài đặt SDK và các dependencies trước khi chạy các sample này.

