/**
 * @file main_with_config.cpp
 * @brief Alternative main file using config.h for configuration
 * 
 * This is an alternative implementation that uses the config.h header
 * for centralized configuration management.
 */

#include "cvedix/nodes/src/cvedix_rtsp_src_node.h"
#include "cvedix/nodes/infers/cvedix_yolo_detector_node.h"
#include "cvedix/nodes/track/cvedix_sort_track_node.h"
#include "cvedix/nodes/ba/cvedix_ba_crossline_node.h"
#include "cvedix/nodes/osd/cvedix_ba_crossline_osd_node.h"
#include "cvedix/nodes/des/cvedix_screen_des_node.h"
#include "cvedix/nodes/des/cvedix_rtmp_des_node.h"
#include "cvedix/utils/analysis_board/cvedix_analysis_board.h"

#include "config.h"

#include <iostream>
#include <memory>
#include <string>
#include <map>

/**
 * @brief Main function using config.h for configuration
 */
int main(int argc, char* argv[]) {
    CVEDIX_SET_LOG_LEVEL(cvedix_utils::cvedix_log_level::INFO);
    CVEDIX_LOGGER_INIT();

    std::cout << "=========================================" << std::endl;
    std::cout << "RTSP BA Crossline Detection Application" << std::endl;
    std::cout << "=========================================" << std::endl;

    // Use configuration from config.h
    std::string rtsp_url = app_config::DEFAULT_RTSP_URL;
    std::string rtmp_url = app_config::DEFAULT_RTMP_URL;

    // Allow command line arguments to override defaults
    if (argc > 1) {
        rtsp_url = argv[1];
    }
    if (argc > 2) {
        rtmp_url = argv[2];
    }

    std::cout << "RTSP URL: " << rtsp_url << std::endl;
    std::cout << "RTMP URL: " << rtmp_url << std::endl;
    std::cout << "Line: (" << app_config::LINE_START_X << ", " << app_config::LINE_START_Y 
              << ") -> (" << app_config::LINE_END_X << ", " << app_config::LINE_END_Y << ")" << std::endl;

    try {
        // Create nodes using configuration
        std::cout << "\n[1/7] Creating RTSP source node..." << std::endl;
        auto rtsp_src_0 = std::make_shared<cvedix_nodes::cvedix_rtsp_src_node>(
            "rtsp_src_0", 
            app_config::RTSP_CHANNEL, 
            rtsp_url, 
            app_config::RTSP_FPS
        );

        std::cout << "[2/7] Creating YOLO detector node..." << std::endl;
        auto yolo_detector = std::make_shared<cvedix_nodes::cvedix_yolo_detector_node>(
            "yolo_detector",
            app_config::YOLO_WEIGHTS,
            app_config::YOLO_CONFIG,
            app_config::YOLO_CLASSES
        );

        std::cout << "[3/7] Creating SORT tracker node..." << std::endl;
        auto tracker = std::make_shared<cvedix_nodes::cvedix_sort_track_node>("sort_tracker");

        std::cout << "[4/7] Creating BA crossline node..." << std::endl;
        cvedix_objects::cvedix_point line_start(app_config::LINE_START_X, app_config::LINE_START_Y);
        cvedix_objects::cvedix_point line_end(app_config::LINE_END_X, app_config::LINE_END_Y);
        std::map<int, cvedix_objects::cvedix_line> lines = {
            {0, cvedix_objects::cvedix_line(line_start, line_end)}
        };
        auto ba_crossline = std::make_shared<cvedix_nodes::cvedix_ba_crossline_node>(
            "ba_crossline", 
            lines
        );

        std::cout << "[5/7] Creating OSD node..." << std::endl;
        auto osd = std::make_shared<cvedix_nodes::cvedix_ba_crossline_osd_node>("osd");

        std::cout << "[6/7] Creating screen destination node..." << std::endl;
        auto screen_des_0 = std::make_shared<cvedix_nodes::cvedix_screen_des_node>(
            "screen_des_0", 
            app_config::RTSP_CHANNEL
        );

        std::cout << "[7/7] Creating RTMP destination node..." << std::endl;
        auto rtmp_des_0 = std::make_shared<cvedix_nodes::cvedix_rtmp_des_node>(
            "rtmp_des_0", 
            app_config::RTMP_CHANNEL, 
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

        // For debug purpose
        if (app_config::ENABLE_ANALYSIS_BOARD) {
            cvedix_utils::cvedix_analysis_board board({rtsp_src_0});
            board.display(app_config::ANALYSIS_BOARD_DISPLAY_INTERVAL, false);
        }

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

