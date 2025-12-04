#include "cvedix/nodes/src/cvedix_file_src_node.h"
#include "cvedix/nodes/infers/cvedix_yunet_face_detector_node.h"
#include "cvedix/nodes/track/cvedix_sort_track_node.h"
#include "cvedix/nodes/osd/cvedix_face_osd_node_v2.h"
#include "cvedix/nodes/des/cvedix_screen_des_node.h"
#include "cvedix/nodes/des/cvedix_rtmp_des_node.h"
#include "cvedix/nodes/mid/cvedix_split_node.h"
#include "cvedix/utils/analysis_board/cvedix_analysis_board.h"

#ifdef CVEDIX_WITH_MQTT
#include "cvedix/nodes/broker/cvedix_json_enhanced_console_broker_node.h"
#include "cvedix/nodes/broker/cvedix_json_mqtt_broker_node.h"
#include "cvedix/utils/mqtt_client/cvedix_mqtt_client.h"
#include "cvedix/nodes/broker/cereal_archive/cvedix_objects_cereal_archive.h"
#include "cpp_base64/base64.h"
#include <opencv2/imgcodecs.hpp>
#include <csignal>
#include <thread>
#include <atomic>
#include <chrono>
#include <ctime>
#include <sstream>
#include <iomanip>
#endif

/*
 * ## Simple RTMP + MQTT Sample ##
 * 
 * Sample đơn giản kết hợp RTMP streaming và MQTT event publishing
 * 
 * Tính năng:
 * - Đọc video từ file (có thể thay bằng RTSP)
 * - Phát hiện khuôn mặt với YuNet
 * - Theo dõi khuôn mặt với SORT tracker
 * - Gửi events qua MQTT khi có khuôn mặt mới
 * - Gửi video stream lên RTMP server
 * - Hiển thị trên màn hình
 * 
 * Yêu cầu:
 * - Build với: cmake -DCVEDIX_WITH_MQTT=ON ..
 * - MQTT broker (mosquitto)
 * - RTMP server (nginx-rtmp hoặc tương tự)
 * 
 * Cấu hình:
 * - Chỉnh sửa RTMP_URL và MQTT_BROKER trong code
 * 
 * Sử dụng:
 *   ./simple_rtmp_mqtt_sample [video_file] [rtmp_url] [mqtt_broker] [mqtt_port] [mqtt_topic]
 * 
 * Ví dụ:
 *   ./simple_rtmp_mqtt_sample ./test_video.mp4 rtmp://localhost:1935/live/test localhost 1883 events
 */

#ifdef CVEDIX_WITH_MQTT
// Global flag for signal handling
volatile sig_atomic_t stop_flag = 0;

void signal_handler(int signal) {
    stop_flag = 1;
}

// Helper function to crop image
cv::Mat crop_image(const cv::Mat& frame, int x, int y, int width, int height) {
    if (frame.empty()) return cv::Mat();
    int x1 = std::max(0, x);
    int y1 = std::max(0, y);
    int x2 = std::min(frame.cols, x + width);
    int y2 = std::min(frame.rows, y + height);
    if (x2 <= x1 || y2 <= y1) return cv::Mat();
    cv::Rect roi(x1, y1, x2 - x1, y2 - y1);
    return frame(roi).clone();
}

// Helper function to encode image to base64
std::string mat_to_base64(const cv::Mat& img) {
    if (img.empty()) return "";
    std::vector<uchar> buf;
    cv::imencode(".jpg", img, buf);
    std::string encoded = base64_encode(buf.data(), buf.size());
    return encoded;
}

// Helper function to get current timestamp
std::string get_current_timestamp() {
    auto now = std::chrono::duration_cast<std::chrono::milliseconds>(
        std::chrono::system_clock::now().time_since_epoch()).count();
    return std::to_string(now);
}
#endif

int main(int argc, char* argv[]) {
    CVEDIX_SET_LOG_INCLUDE_CODE_LOCATION(false);
    CVEDIX_SET_LOG_INCLUDE_THREAD_ID(false);
    CVEDIX_SET_LOG_LEVEL(cvedix_utils::cvedix_log_level::INFO);
    CVEDIX_LOGGER_INIT();

#ifdef CVEDIX_WITH_MQTT
    // Signal handling
    std::signal(SIGINT, signal_handler);
    std::signal(SIGTERM, signal_handler);
#endif

    // Configuration với giá trị mặc định
    std::string video_file = "./cvedix_data/test_video/face.mp4";
    std::string rtmp_url = "rtmp://localhost:1935/live/test";
    
#ifdef CVEDIX_WITH_MQTT
    std::string mqtt_broker = "localhost";
    int mqtt_port = 1883;
    std::string mqtt_topic = "events";
    std::string mqtt_username = "";
    std::string mqtt_password = "";
#endif

    // Parse command line arguments
    if (argc > 1) video_file = argv[1];
    if (argc > 2) rtmp_url = argv[2];
#ifdef CVEDIX_WITH_MQTT
    if (argc > 3) mqtt_broker = argv[3];
    if (argc > 4) mqtt_port = std::stoi(argv[4]);
    if (argc > 5) mqtt_topic = argv[5];
    if (argc > 6) mqtt_username = argv[6];
    if (argc > 7) mqtt_password = argv[7];
#endif

    std::cout << "=== Simple RTMP + MQTT Sample ===" << std::endl;
    std::cout << "Video file: " << video_file << std::endl;
    std::cout << "RTMP URL: " << rtmp_url << std::endl;
#ifdef CVEDIX_WITH_MQTT
    std::cout << "MQTT Broker: " << mqtt_broker << ":" << mqtt_port << std::endl;
    std::cout << "MQTT Topic: " << mqtt_topic << std::endl;
#endif
    std::cout << "=================================" << std::endl;

    try {
        // 1. Source node - Đọc video từ file
        auto file_src = std::make_shared<cvedix_nodes::cvedix_file_src_node>(
            "file_src_0", 0, video_file, 0.6);

        // 2. Face detector - Phát hiện khuôn mặt
        auto face_detector = std::make_shared<cvedix_nodes::cvedix_yunet_face_detector_node>(
            "yunet_face_detector_0", 
            "./cvedix_data/models/face/face_detection_yunet_2022mar.onnx");

        // 3. Tracker - Theo dõi khuôn mặt
        auto tracker = std::make_shared<cvedix_nodes::cvedix_sort_track_node>(
            "tracker_0",
            cvedix_nodes::cvedix_track_for::FACE);

#ifdef CVEDIX_WITH_MQTT
        // 4. MQTT Client - Kết nối đến MQTT broker
        auto mqtt_client = std::make_shared<cvedix_utils::cvedix_mqtt_client>(
            mqtt_broker, mqtt_port, "simple_rtmp_mqtt_sample");

        // Kết nối MQTT
        std::cout << "[MQTT] Connecting to broker..." << std::endl;
        if (!mqtt_client->connect(mqtt_username, mqtt_password)) {
            std::cerr << "[MQTT] Failed to connect: " << mqtt_client->get_last_error() << std::endl;
            std::cerr << "[MQTT] Continuing without MQTT..." << std::endl;
        } else {
            std::cout << "[MQTT] Connected successfully!" << std::endl;
        }

        // MQTT publish function - Gửi events khi có khuôn mặt mới
        auto mqtt_publish_func = [mqtt_client, mqtt_topic](const std::string& json_data) {
            if (mqtt_client && mqtt_client->is_connected()) {
                mqtt_client->publish(mqtt_topic, json_data, 1, false);
            }
        };

        // 5. MQTT Broker Node - Gửi events qua MQTT
        auto mqtt_broker_node = std::make_shared<cvedix_nodes::cvedix_json_enhanced_mqtt_broker_node>(
            "mqtt_broker_0",
            cvedix_nodes::cvedix_broke_for::FACE,
            100,   // broking_cache_warn_threshold
            500,   // broking_cache_ignore_threshold
            false, // encode_full_frame (chỉ encode crop images)
            mqtt_publish_func);
#endif

        // 6. OSD Node - Vẽ kết quả lên frame
        auto osd = std::make_shared<cvedix_nodes::cvedix_face_osd_node_v2>("osd_0");

        // 7. Split Node - Chia output thành nhiều nhánh
        auto split = std::make_shared<cvedix_nodes::cvedix_split_node>("split_0", false, false);

        // 8. Screen Destination - Hiển thị trên màn hình
        auto screen_des = std::make_shared<cvedix_nodes::cvedix_screen_des_node>("screen_des_0", 0);

        // 9. RTMP Destination - Gửi stream lên RTMP server
        auto rtmp_des = std::make_shared<cvedix_nodes::cvedix_rtmp_des_node>(
            "rtmp_des_0", 
            0, 
            rtmp_url,
            cvedix_objects::cvedix_size{1280, 720},  // Resolution
            2048);  // Bitrate

        // Xây dựng pipeline
        face_detector->attach_to({file_src});
        tracker->attach_to({face_detector});

#ifdef CVEDIX_WITH_MQTT
        // Nếu có MQTT: Tracker → MQTT Broker → OSD
        if (mqtt_client && mqtt_client->is_connected()) {
            mqtt_broker_node->attach_to({tracker});
            osd->attach_to({mqtt_broker_node});
        } else {
            // Nếu không có MQTT: Tracker → OSD
            osd->attach_to({tracker});
        }
#else
        // Không có MQTT: Tracker → OSD
        osd->attach_to({tracker});
#endif

        split->attach_to({osd});
        screen_des->attach_to({split});
        rtmp_des->attach_to({split});

        // Khởi động pipeline
        std::cout << "[Main] Starting pipeline..." << std::endl;
        file_src->start();

        // Analysis board (optional, for debugging)
        cvedix_utils::cvedix_analysis_board board({file_src});
        board.display(1, false);

        std::cout << "[Main] Pipeline running. Press Ctrl+C to stop..." << std::endl;

#ifdef CVEDIX_WITH_MQTT
        // Main loop với signal handling
        while (!stop_flag) {
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
        }
#else
        // Chờ user input
        std::string wait;
        std::getline(std::cin, wait);
#endif

        // Dừng pipeline
        std::cout << "[Main] Stopping pipeline..." << std::endl;
        file_src->detach_recursively();

#ifdef CVEDIX_WITH_MQTT
        // Disconnect MQTT
        if (mqtt_client) {
            mqtt_client->disconnect();
        }
#endif

        std::cout << "[Main] Pipeline stopped." << std::endl;

    } catch (const std::exception& e) {
        std::cerr << "[Error] " << e.what() << std::endl;
        return 1;
    }

    return 0;
}

#ifdef CVEDIX_WITH_MQTT
#else
// Nếu không build với MQTT, hiển thị thông báo
#include <iostream>
int main(int argc, char* argv[]) {
    std::cerr << "MQTT support not enabled. Build with -DCVEDIX_WITH_MQTT=ON" << std::endl;
    std::cerr << "Usage: ./simple_rtmp_mqtt_sample [video_file] [rtmp_url]" << std::endl;
    return 1;
}
#endif

