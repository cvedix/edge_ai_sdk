/**
 * @file config.h
 * @brief Configuration header for RTSP BA Crossline Detection Application
 */

#ifndef CONFIG_H
#define CONFIG_H

#include <string>

namespace app_config {

    // RTSP Source Configuration
    constexpr int RTSP_CHANNEL = 0;
    constexpr double RTSP_FPS = 0.6;
    const std::string DEFAULT_RTSP_URL = "rtsp://anhoidong.datacenter.cvedix.com:8554/live/camera_demo";

    // YOLO Detector Configuration
    const std::string YOLO_WEIGHTS = "./cvedix_data/models/det_cls/yolov3-tiny-2022-0721_best.weights";
    const std::string YOLO_CONFIG = "./cvedix_data/models/det_cls/yolov3-tiny-2022-0721.cfg";
    const std::string YOLO_CLASSES = "./cvedix_data/models/det_cls/yolov3_tiny_5classes.txt";

    // Crossline Configuration
    // Adjust these coordinates based on your camera view and frame resolution
    // Line coordinates must be within frame dimensions
    constexpr int LINE_START_X = 0;
    constexpr int LINE_START_Y = 250;
    constexpr int LINE_END_X = 700;
    constexpr int LINE_END_Y = 220;

    // RTMP Destination Configuration
    constexpr int RTMP_CHANNEL = 0;
    const std::string DEFAULT_RTMP_URL = "rtmp://anhoidong.datacenter.cvedix.com:1935/live/camera_traffic_usa_ai";

    // Application Settings
    constexpr bool ENABLE_ANALYSIS_BOARD = true;
    constexpr int ANALYSIS_BOARD_DISPLAY_INTERVAL = 1;

} // namespace app_config

#endif // CONFIG_H

