#include "cvedix/nodes/src/cvedix_rtsp_src_node.h"
#include "cvedix/nodes/infers/cvedix_yolo_detector_node.h"
#include "cvedix/nodes/track/cvedix_sort_track_node.h"
#include "cvedix/nodes/ba/cvedix_ba_crossline_node.h"
#include "cvedix/nodes/osd/cvedix_ba_crossline_osd_node.h"
#include "cvedix/nodes/des/cvedix_screen_des_node.h"
#include "cvedix/nodes/des/cvedix_rtmp_des_node.h"
#include "cvedix/utils/analysis_board/cvedix_analysis_board.h"

#include <iostream>
#include <memory>
#include <string>
#include <map>

/**
 * @file main.cpp
 * @brief RTSP Behavior Analysis Crossline Detection Application
 * 
 * This application implements a behavior analysis pipeline for crossline detection
 * using RTSP input stream. The pipeline includes:
 * - RTSP source node for video input
 * - YOLO detector for object detection
 * - SORT tracker for object tracking
 * - Behavior analysis crossline node for line crossing detection
 * - OSD node for visualization
 * - Screen and RTMP destination nodes for output
 */

/**
 * @brief Main function to run the RTSP BA crossline detection pipeline
 * @return Exit code (0 on success)
 */
int main(int argc, char* argv[]) {
    // Set log level and initialize logger
    CVEDIX_SET_LOG_LEVEL(cvedix_utils::cvedix_log_level::INFO);
    CVEDIX_LOGGER_INIT();

    std::cout << "=========================================" << std::endl;
    std::cout << "RTSP BA Crossline Detection Application" << std::endl;
    std::cout << "=========================================" << std::endl;

    // Configuration parameters
    // RTSP source configuration
    std::string rtsp_url = "rtsp://anhoidong.datacenter.cvedix.com:8554/live/camera_demo";
    int rtsp_channel = 0;
    double rtsp_fps = 0.6;

    // YOLO detector configuration
    std::string yolo_weights = "./cvedix_data/models/det_cls/yolov3-tiny-2022-0721_best.weights";
    std::string yolo_config = "./cvedix_data/models/det_cls/yolov3-tiny-2022-0721.cfg";
    std::string yolo_classes = "./cvedix_data/models/det_cls/yolov3_tiny_5classes.txt";

    // Crossline configuration (adjust these values based on your camera view)
    // Line coordinates must be within frame dimensions
    cvedix_objects::cvedix_point line_start(0, 250);
    cvedix_objects::cvedix_point line_end(700, 220);

    // RTMP destination configuration
    std::string rtmp_url = "rtmp://anhoidong.datacenter.cvedix.com:1935/live/camera_traffic_usa_ai";
    int rtmp_channel = 0;

    // Allow command line arguments to override defaults
    if (argc > 1) {
        rtsp_url = argv[1];
    }
    if (argc > 2) {
        rtmp_url = argv[2];
    }

    std::cout << "RTSP URL: " << rtsp_url << std::endl;
    std::cout << "RTMP URL: " << rtmp_url << std::endl;
    std::cout << "Line: (" << line_start.x << ", " << line_start.y 
              << ") -> (" << line_end.x << ", " << line_end.y << ")" << std::endl;

    try {
        // Create RTSP source node
        std::cout << "\n[1/7] Creating RTSP source node..." << std::endl;
        auto rtsp_src_0 = std::make_shared<cvedix_nodes::cvedix_rtsp_src_node>(
            "rtsp_src_0", 
            rtsp_channel, 
            rtsp_url, 
            rtsp_fps
        );

        // Create YOLO detector node
        std::cout << "[2/7] Creating YOLO detector node..." << std::endl;
        auto yolo_detector = std::make_shared<cvedix_nodes::cvedix_yolo_detector_node>(
            "yolo_detector",
            yolo_weights,
            yolo_config,
            yolo_classes
        );

        // Create SORT tracker node
        std::cout << "[3/7] Creating SORT tracker node..." << std::endl;
        auto tracker = std::make_shared<cvedix_nodes::cvedix_sort_track_node>("sort_tracker");

        // Create behavior analysis crossline node
        std::cout << "[4/7] Creating BA crossline node..." << std::endl;
        std::map<int, cvedix_objects::cvedix_line> lines = {
            {0, cvedix_objects::cvedix_line(line_start, line_end)}
        };
        auto ba_crossline = std::make_shared<cvedix_nodes::cvedix_ba_crossline_node>(
            "ba_crossline", 
            lines
        );

        // Create OSD (On-Screen Display) node
        std::cout << "[5/7] Creating OSD node..." << std::endl;
        auto osd = std::make_shared<cvedix_nodes::cvedix_ba_crossline_osd_node>("osd");

        // Create screen destination node
        std::cout << "[6/7] Creating screen destination node..." << std::endl;
        auto screen_des_0 = std::make_shared<cvedix_nodes::cvedix_screen_des_node>(
            "screen_des_0", 
            rtsp_channel
        );

        // Create RTMP destination node
        std::cout << "[7/7] Creating RTMP destination node..." << std::endl;
        auto rtmp_des_0 = std::make_shared<cvedix_nodes::cvedix_rtmp_des_node>(
            "rtmp_des_0", 
            rtmp_channel, 
            rtmp_url
        );

        // Construct pipeline
        std::cout << "\nConnecting pipeline nodes..." << std::endl;
        yolo_detector->attach_to({rtsp_src_0});
        tracker->attach_to({yolo_detector});
        ba_crossline->attach_to({tracker});
        osd->attach_to({ba_crossline});
        screen_des_0->attach_to({osd});
        rtmp_des_0->attach_to({osd});

        std::cout << "Pipeline connected successfully!" << std::endl;

        // Start the pipeline
        std::cout << "\nStarting RTSP source..." << std::endl;
        rtsp_src_0->start();

        std::cout << "\nApplication is running..." << std::endl;
        std::cout << "Press Enter to stop..." << std::endl;

        // For debug purpose - display analysis board
        cvedix_utils::cvedix_analysis_board board({rtsp_src_0});
        board.display(1, false);

        // Wait for user input to stop
        std::string wait;
        std::getline(std::cin, wait);

        std::cout << "\nStopping pipeline..." << std::endl;
        rtsp_src_0->detach_recursively();

        std::cout << "Application stopped successfully." << std::endl;

    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}

