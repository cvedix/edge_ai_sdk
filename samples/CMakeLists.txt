# CMakeLists.txt for CVEDIX SDK Samples
# This file can ONLY be called by parent CMakeLists.txt
if(CMAKE_SOURCE_DIR STREQUAL CMAKE_CURRENT_SOURCE_DIR)
    message(FATAL_ERROR "This CMakeLists.txt can ONLY be called by parent CMakeLists.txt.")
endif()

message(STATUS "Building CVEDIX SDK samples...")

# Set output directory for all executables
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

# Set RPATH to find SDK libraries at runtime
# This allows executables to find libraries in /opt/cvedix/lib
# Note: For transitive dependencies like libtinyexpr.so, you may need to set LD_LIBRARY_PATH
# or use the run_samples.sh wrapper script
if(EXISTS /opt/cvedix/lib)
    set(CMAKE_BUILD_RPATH "/opt/cvedix/lib")
    set(CMAKE_INSTALL_RPATH "/opt/cvedix/lib")
    set(CMAKE_BUILD_WITH_INSTALL_RPATH TRUE)
    set(CMAKE_INSTALL_RPATH_USE_LINK_PATH TRUE)
    # Also set for all targets
    set(CMAKE_BUILD_RPATH_USE_ORIGIN TRUE)
    
    # Add library directory to linker search path for all targets
    link_directories(/opt/cvedix/lib)
endif()

# Find GStreamer (required for RTSP and some other features)
find_package(PkgConfig REQUIRED)
pkg_check_modules(GSTREAMER REQUIRED gstreamer-1.0)
pkg_check_modules(GSTREAMER_APP REQUIRED gstreamer-app-1.0)
pkg_check_modules(GSTREAMER_VIDEO REQUIRED gstreamer-video-1.0)

# Helper function to link GStreamer to a target
function(link_gstreamer target_name)
    target_include_directories(${target_name} PRIVATE ${GSTREAMER_INCLUDE_DIRS})
    target_link_libraries(${target_name} ${GSTREAMER_LIBRARIES} ${GSTREAMER_APP_LIBRARIES} ${GSTREAMER_VIDEO_LIBRARIES})
    target_compile_options(${target_name} PRIVATE ${GSTREAMER_CFLAGS_OTHER})
endfunction()

# Helper function to link third_party (cereal) to a target
# Header files in SDK look for ../../../third_party/cereal from /opt/cvedix/include
# So we need to ensure /opt/cvedix/third_party exists or add include paths
function(link_third_party target_name)
    # Check if /opt/cvedix/third_party exists
    if(EXISTS /opt/cvedix/third_party)
        # If it exists, add /opt/cvedix as include directory so relative paths work
        target_include_directories(${target_name} PRIVATE /opt/cvedix)
    else()
        # If not, try to use workspace third_party
        # We need to add include directory that makes the relative path work
        # Header looks for ../../../third_party/cereal from /opt/cvedix/include/cvedix/nodes/broker/cereal_archive/
        # So we add /opt/cvedix/include/cvedix/nodes/broker/cereal_archive as base
        # And then add workspace third_party with proper structure
        if(EXISTS ${CMAKE_SOURCE_DIR}/third_party)
            # Add workspace third_party, but we need to make it look like it's in /opt/cvedix
            # We'll add both /opt/cvedix and workspace third_party
            target_include_directories(${target_name} PRIVATE /opt/cvedix)
            # Create a workaround: add the workspace third_party with a path that matches the expected structure
            # The header expects: ../../../third_party/cereal from /opt/cvedix/include/cvedix/nodes/broker/cereal_archive/
            # So we add the parent directory of workspace third_party
            get_filename_component(THIRD_PARTY_PARENT ${CMAKE_SOURCE_DIR}/third_party DIRECTORY)
            target_include_directories(${target_name} PRIVATE ${THIRD_PARTY_PARENT})
        endif()
        message(WARNING "third_party not found in /opt/cvedix. Please create symlink: sudo ln -sf ${CMAKE_SOURCE_DIR}/third_party /opt/cvedix/third_party")
    endif()
endfunction()

# ============================================================================
# Basic Samples - No special dependencies required
# ============================================================================
message(STATUS "Building basic samples...")

# Pipeline structure samples
add_executable(1-1-1_sample "1-1-1_sample.cpp")
target_link_libraries(1-1-1_sample cvedix::cvedix_instance_sdk)

add_executable(1-1-N_sample "1-1-N_sample.cpp")
target_link_libraries(1-1-N_sample cvedix::cvedix_instance_sdk)

add_executable(1-N-N_sample "1-N-N_sample.cpp")
target_link_libraries(1-N-N_sample cvedix::cvedix_instance_sdk)

add_executable(N-1-N_sample "N-1-N_sample.cpp")
target_link_libraries(N-1-N_sample cvedix::cvedix_instance_sdk)

add_executable(N-N_sample "N-N_sample.cpp")
target_link_libraries(N-N_sample cvedix::cvedix_instance_sdk)

# Image processing samples
add_executable(image_src_sample "image_src_sample.cpp")
target_link_libraries(image_src_sample cvedix::cvedix_instance_sdk)
link_gstreamer(image_src_sample)

add_executable(image_des_sample "image_des_sample.cpp")
target_link_libraries(image_des_sample cvedix::cvedix_instance_sdk)
link_gstreamer(image_des_sample)

# Detection and tracking samples
add_executable(face_tracking_sample "face_tracking_sample.cpp")
target_link_libraries(face_tracking_sample cvedix::cvedix_instance_sdk)

add_executable(multi_detectors_sample "multi_detectors_sample.cpp")
target_link_libraries(multi_detectors_sample cvedix::cvedix_instance_sdk)
link_gstreamer(multi_detectors_sample)

add_executable(multi_detectors_and_classifiers_sample "multi_detectors_and_classifiers_sample.cpp")
target_link_libraries(multi_detectors_and_classifiers_sample cvedix::cvedix_instance_sdk)

# Segmentation samples
add_executable(mask_rcnn_sample "mask_rcnn_sample.cpp")
target_link_libraries(mask_rcnn_sample cvedix::cvedix_instance_sdk)

add_executable(enet_seg_sample "enet_seg_sample.cpp")
target_link_libraries(enet_seg_sample cvedix::cvedix_instance_sdk)

add_executable(openpose_sample "openpose_sample.cpp")
target_link_libraries(openpose_sample cvedix::cvedix_instance_sdk)

# Behavior analysis samples
add_executable(ba_crossline_sample "ba_crossline_sample.cpp")
target_link_libraries(ba_crossline_sample cvedix::cvedix_instance_sdk)

# Face processing samples
add_executable(face_swap_sample "face_swap_sample.cpp")
target_link_libraries(face_swap_sample cvedix::cvedix_instance_sdk)

add_executable(face_yunet_int8_sample "face_yunet_int8_sample.cpp")
target_link_libraries(face_yunet_int8_sample cvedix::cvedix_instance_sdk)

# Utility samples
add_executable(cvedix_logger_sample "cvedix_logger_sample.cpp")
target_link_libraries(cvedix_logger_sample cvedix::cvedix_instance_sdk)

add_executable(record_sample "record_sample.cpp")
target_link_libraries(record_sample cvedix::cvedix_instance_sdk)

add_executable(skip_sample "skip_sample.cpp")
target_link_libraries(skip_sample cvedix::cvedix_instance_sdk)
link_gstreamer(skip_sample)

add_executable(interaction_with_pipe_sample "interaction_with_pipe_sample.cpp")
target_link_libraries(interaction_with_pipe_sample cvedix::cvedix_instance_sdk)

add_executable(message_broker_sample "message_broker_sample.cpp")
target_link_libraries(message_broker_sample cvedix::cvedix_instance_sdk)
link_third_party(message_broker_sample)

# Detection samples
add_executable(obstacle_detect_sample "obstacle_detect_sample.cpp")
target_link_libraries(obstacle_detect_sample cvedix::cvedix_instance_sdk)
link_gstreamer(obstacle_detect_sample)

add_executable(firesmoke_detect_sample "firesmoke_detect_sample.cpp")
target_link_libraries(firesmoke_detect_sample cvedix::cvedix_instance_sdk)
link_gstreamer(firesmoke_detect_sample)

add_executable(lane_detect_sample "lane_detect_sample.cpp")
target_link_libraries(lane_detect_sample cvedix::cvedix_instance_sdk)

# Video processing samples
add_executable(video_restoration_sample "video_restoration_sample.cpp")
target_link_libraries(video_restoration_sample cvedix::cvedix_instance_sdk)

add_executable(frame_fusion_sample "frame_fusion_sample.cpp")
target_link_libraries(frame_fusion_sample cvedix::cvedix_instance_sdk)

# Application source/des samples
add_executable(app_des_sample "app_des_sample.cpp")
target_link_libraries(app_des_sample cvedix::cvedix_instance_sdk)

add_executable(app_src_des_sample "app_src_des_sample.cpp")
target_link_libraries(app_src_des_sample cvedix::cvedix_instance_sdk)

# Test sample
add_executable(cvedix_test "cvedix_test.cpp")
target_link_libraries(cvedix_test cvedix::cvedix_instance_sdk)
link_gstreamer(cvedix_test)

# ============================================================================
# RTSP Samples - Require RTSP server support
# ============================================================================
if(CVEDIX_WITH_RTSP_SERVER)
    message(STATUS "Building RTSP samples...")
    
    add_executable(rtsp_src_sample "rtsp_src_sample.cpp")
    target_link_libraries(rtsp_src_sample cvedix::cvedix_instance_sdk)
    link_gstreamer(rtsp_src_sample)
    
    add_executable(rtsp_des_sample "rtsp_des_sample.cpp")
    target_link_libraries(rtsp_des_sample cvedix::cvedix_instance_sdk)
    link_gstreamer(rtsp_des_sample)
    
    add_executable(rtsp_ba_crossline_sample "rtsp_ba_crossline_sample.cpp")
    target_link_libraries(rtsp_ba_crossline_sample cvedix::cvedix_instance_sdk)
    link_gstreamer(rtsp_ba_crossline_sample)
    
    # Face tracking RTSP sample - requires RTSP and optionally MQTT
    add_executable(face_tracking_rtsp_sample "face_tracking_rtsp_sample.cpp")
    target_include_directories(face_tracking_rtsp_sample PRIVATE ${CMAKE_SOURCE_DIR}/third_party)
    target_link_libraries(face_tracking_rtsp_sample cvedix::cvedix_instance_sdk)
    link_gstreamer(face_tracking_rtsp_sample)
    
    # Link with mosquitto if MQTT is enabled
    if(CVEDIX_WITH_MQTT)
        if(MOSQUITTO_FOUND)
            target_link_libraries(face_tracking_rtsp_sample ${MOSQUITTO_LIBRARIES})
        else()
            find_library(MOSQUITTO_LIB NAMES mosquitto)
            if(MOSQUITTO_LIB)
                target_link_libraries(face_tracking_rtsp_sample ${MOSQUITTO_LIB})
            endif()
        endif()
    endif()
else()
    message(STATUS "Skipping RTSP samples - RTSP server not available")
endif()

# ============================================================================
# Kafka Samples - Require Kafka support
# ============================================================================
if(CVEDIX_WITH_KAFKA)
    message(STATUS "Building Kafka samples...")
    
    add_executable(message_broker_kafka_sample "message_broker_kafka_sample.cpp")
    target_link_libraries(message_broker_kafka_sample cvedix::cvedix_instance_sdk)
    link_third_party(message_broker_kafka_sample)
else()
    message(STATUS "Skipping Kafka samples - Kafka not available")
endif()

# ============================================================================
# MQTT Samples - Require MQTT support
# ============================================================================
if(CVEDIX_WITH_MQTT)
    message(STATUS "Building MQTT samples...")
    
    # Note: mqtt_json_receiver_sample requires cvedix_mqtt_json_receiver implementation
    # which is not available in the current SDK build. Commented out to avoid linking errors.
    # Uncomment the following lines if your SDK has MQTT JSON receiver support:
    #
    # Check if mosquitto is available for mqtt_json_receiver_sample
    find_path(MOSQUITTO_INCLUDE_DIR2 mosquitto.h
        PATHS /usr/include /usr/local/include /opt/local/include
    )
    find_library(MOSQUITTO_LIB2 NAMES mosquitto
        PATHS /usr/lib /usr/local/lib /opt/local/lib
    )
    
    if(MOSQUITTO_INCLUDE_DIR2 AND MOSQUITTO_LIB2)
        add_executable(mqtt_json_receiver_sample 
            "mqtt_json_receiver_sample.cpp"
            "mqtt_json_receiver_impl.cpp"
        )
        target_include_directories(mqtt_json_receiver_sample PRIVATE 
            ${CMAKE_SOURCE_DIR}/third_party
            ${MOSQUITTO_INCLUDE_DIR2}
        )
    target_link_libraries(mqtt_json_receiver_sample 
        cvedix::cvedix_instance_sdk
        ${MOSQUITTO_LIB2}
    )
    target_compile_definitions(mqtt_json_receiver_sample PRIVATE CVEDIX_WITH_MQTT)
    # Set RPATH for runtime library loading
    if(EXISTS /opt/cvedix/lib)
        set_target_properties(mqtt_json_receiver_sample PROPERTIES
            BUILD_RPATH "/opt/cvedix/lib"
            INSTALL_RPATH "/opt/cvedix/lib"
            BUILD_WITH_INSTALL_RPATH TRUE
        )
        # Also link the directory so linker can find it
        target_link_directories(mqtt_json_receiver_sample PRIVATE /opt/cvedix/lib)
    endif()
    else()
        message(WARNING "mosquitto not found - skipping mqtt_json_receiver_sample")
    endif()
    
    # Simple RTMP + MQTT sample - Basic sample combining RTMP and MQTT
    # Check if mosquitto is available
    find_path(MOSQUITTO_INCLUDE_DIR mosquitto.h
        PATHS
        /usr/include
        /usr/local/include
        /opt/local/include
    )
    
    find_library(MOSQUITTO_LIB NAMES mosquitto
        PATHS
        /usr/lib
        /usr/local/lib
        /opt/local/lib
    )
    
    if(MOSQUITTO_INCLUDE_DIR AND MOSQUITTO_LIB)
        message(STATUS "Found mosquitto: ${MOSQUITTO_INCLUDE_DIR}, ${MOSQUITTO_LIB}")
        add_executable(simple_rtmp_mqtt_sample "simple_rtmp_mqtt_sample.cpp")
        target_include_directories(simple_rtmp_mqtt_sample PRIVATE 
            ${CMAKE_SOURCE_DIR}/third_party
            ${MOSQUITTO_INCLUDE_DIR}
        )
        target_link_libraries(simple_rtmp_mqtt_sample 
            cvedix::cvedix_instance_sdk
            ${MOSQUITTO_LIB}
        )
        # Define CVEDIX_WITH_MQTT macro for compilation
        target_compile_definitions(simple_rtmp_mqtt_sample PRIVATE CVEDIX_WITH_MQTT)
        # Set RPATH for runtime library loading
        if(EXISTS /opt/cvedix/lib)
            set_target_properties(simple_rtmp_mqtt_sample PROPERTIES
                BUILD_RPATH "/opt/cvedix/lib"
                INSTALL_RPATH "/opt/cvedix/lib"
            )
        endif()
    else()
        message(WARNING "mosquitto not found - skipping simple_rtmp_mqtt_sample")
        message(WARNING "Install libmosquitto-dev to build this sample: sudo apt-get install libmosquitto-dev")
    endif()
else()
    message(STATUS "Skipping MQTT samples - MQTT not available")
endif()

# ============================================================================
# LLM Samples - Require LLM support
# ============================================================================
if(CVEDIX_WITH_LLM)
    message(STATUS "Building LLM samples...")
    
    add_executable(mllm_analyse_sample "mllm_analyse_sample.cpp")
    target_link_libraries(mllm_analyse_sample cvedix::cvedix_instance_sdk)
    
    add_executable(mllm_analyse_sample_openai "mllm_analyse_sample_openai.cpp")
    target_link_libraries(mllm_analyse_sample_openai cvedix::cvedix_instance_sdk)
else()
    message(STATUS "Skipping LLM samples - LLM not available")
endif()

# ============================================================================
# FFmpeg Samples - Require FFmpeg support
# ============================================================================
if(CVEDIX_WITH_FFMPEG)
    message(STATUS "Building FFmpeg samples...")
    
    add_executable(ffmpeg_src_des_sample "ffmpeg_src_des_sample.cpp")
    target_link_libraries(ffmpeg_src_des_sample cvedix::cvedix_instance_sdk)
    
    add_executable(ffmpeg_transcode_sample "ffmpeg_transcode_sample.cpp")
    target_link_libraries(ffmpeg_transcode_sample cvedix::cvedix_instance_sdk)
else()
    message(STATUS "Skipping FFmpeg samples - FFmpeg not available")
endif()

# ============================================================================
# RKNN Samples - Require RKNN support
# ============================================================================
if(CVEDIX_WITH_RKNN)
    message(STATUS "Building RKNN samples...")
    
    add_executable(rknn_yolov11_detector_sample "rknn_yolov11_detector_sample.cpp")
    target_link_libraries(rknn_yolov11_detector_sample cvedix::cvedix_instance_sdk)
    
    add_executable(rknn_face_detector_sample "rknn_face_detector_sample.cpp")
    target_link_libraries(rknn_face_detector_sample cvedix::cvedix_instance_sdk)
    
    add_executable(rknn_face_detection_sample "rknn_face_detection_sample.cpp")
    target_link_libraries(rknn_face_detection_sample cvedix::cvedix_instance_sdk)
    link_gstreamer(rknn_face_detection_sample)
    
    add_executable(rknn_face_detection_file_sample "rknn_face_detection_file_sample.cpp")
    target_link_libraries(rknn_face_detection_file_sample cvedix::cvedix_instance_sdk)
    link_gstreamer(rknn_face_detection_file_sample)
    
    add_executable(rknn_rtsp_tracking_sample "rknn_rtsp_tracking_sample.cpp")
    target_link_libraries(rknn_rtsp_tracking_sample cvedix::cvedix_instance_sdk)
    link_gstreamer(rknn_rtsp_tracking_sample)
    
    # RKNN RTSP tracking with MQTT - requires RKNN, RTSP, and MQTT
    if(CVEDIX_WITH_RTSP_SERVER AND CVEDIX_WITH_MQTT)
        add_executable(rknn_rtsp_tracking_mqtt_sample "rknn_rtsp_tracking_mqtt_sample.cpp")
        target_include_directories(rknn_rtsp_tracking_mqtt_sample PRIVATE ${CMAKE_SOURCE_DIR}/third_party)
        target_link_libraries(rknn_rtsp_tracking_mqtt_sample cvedix::cvedix_instance_sdk)
        link_gstreamer(rknn_rtsp_tracking_mqtt_sample)
        
        # Link with mosquitto
        if(MOSQUITTO_FOUND)
            target_link_libraries(rknn_rtsp_tracking_mqtt_sample ${MOSQUITTO_LIBRARIES})
        else()
            find_library(MOSQUITTO_LIB NAMES mosquitto)
            if(MOSQUITTO_LIB)
                target_link_libraries(rknn_rtsp_tracking_mqtt_sample ${MOSQUITTO_LIB})
            endif()
        endif()
    endif()
    
    # RKNN face tracking sample - requires RKNN and MQTT
    if(CVEDIX_WITH_MQTT)
        add_executable(rknn_face_tracking_sample "rknn_face_tracking_sample.cpp")
        target_include_directories(rknn_face_tracking_sample PRIVATE ${CMAKE_SOURCE_DIR}/third_party)
        target_link_libraries(rknn_face_tracking_sample cvedix::cvedix_instance_sdk)
        link_gstreamer(rknn_face_tracking_sample)
        
        # Link with mosquitto
        if(MOSQUITTO_FOUND)
            target_link_libraries(rknn_face_tracking_sample ${MOSQUITTO_LIBRARIES})
        else()
            find_library(MOSQUITTO_LIB NAMES mosquitto)
            if(MOSQUITTO_LIB)
                target_link_libraries(rknn_face_tracking_sample ${MOSQUITTO_LIB})
            endif()
        endif()
    endif()
else()
    message(STATUS "Skipping RKNN samples - RKNN not available")
endif()

# ============================================================================
# PaddlePaddle Samples - Require PaddlePaddle support
# ============================================================================
if(CVEDIX_WITH_PADDLE)
    message(STATUS "Building PaddlePaddle samples...")
    
    add_executable(app_src_sample "app_src_sample.cpp")
    target_link_libraries(app_src_sample cvedix::cvedix_instance_sdk)
    
    add_executable(paddle_infer_sample "paddle_infer_sample.cpp")
    target_link_libraries(paddle_infer_sample cvedix::cvedix_instance_sdk)
else()
    message(STATUS "Skipping PaddlePaddle samples - PaddlePaddle not available")
endif()

# ============================================================================
# CUDA Samples - Require CUDA support
# ============================================================================
if(CVEDIX_WITH_CUDA)
    message(STATUS "Building CUDA samples...")
    
    # nvdec & nvenc plugins required from DeepStream
    add_executable(nv_hard_codec_sample "nv_hard_codec_sample.cpp")
    target_link_libraries(nv_hard_codec_sample cvedix::cvedix_instance_sdk)
    link_gstreamer(nv_hard_codec_sample)
else()
    message(STATUS "Skipping CUDA samples - CUDA not available")
endif()

# ============================================================================
# TensorRT Samples - Require TensorRT support
# ============================================================================
if(CVEDIX_WITH_TRT)
    message(STATUS "Building TensorRT samples...")
    
    # Behavior analysis samples
    add_executable(ba_jam_sample "ba_jam_sample.cpp")
    target_link_libraries(ba_jam_sample cvedix::cvedix_instance_sdk)
    
    add_executable(ba_stop_sample "ba_stop_sample.cpp")
    target_link_libraries(ba_stop_sample cvedix::cvedix_instance_sdk)
    
    # Detection and recognition samples
    add_executable(body_scan_and_plate_detect_sample "body_scan_and_plate_detect_sample.cpp")
    target_link_libraries(body_scan_and_plate_detect_sample cvedix::cvedix_instance_sdk)
    
    add_executable(plate_recognize_sample "plate_recognize_sample.cpp")
    target_link_libraries(plate_recognize_sample cvedix::cvedix_instance_sdk)
    
    add_executable(vehicle_body_scan_sample "vehicle_body_scan_sample.cpp")
    target_link_libraries(vehicle_body_scan_sample cvedix::cvedix_instance_sdk)
    
    add_executable(vehicle_tracking_sample "vehicle_tracking_sample.cpp")
    target_link_libraries(vehicle_tracking_sample cvedix::cvedix_instance_sdk)
    
    add_executable(vehicle_cluster_based_on_classify_encoding_sample "vehicle_cluster_based_on_classify_encoding_sample.cpp")
    target_link_libraries(vehicle_cluster_based_on_classify_encoding_sample cvedix::cvedix_instance_sdk)
    
    # Pipeline samples
    add_executable(dynamic_pipeline_sample "dynamic_pipeline_sample.cpp")
    target_link_libraries(dynamic_pipeline_sample cvedix::cvedix_instance_sdk)
    link_gstreamer(dynamic_pipeline_sample)
    link_third_party(dynamic_pipeline_sample)
    
    add_executable(dynamic_pipeline_sample2 "dynamic_pipeline_sample2.cpp")
    target_link_libraries(dynamic_pipeline_sample2 cvedix::cvedix_instance_sdk)
    link_gstreamer(dynamic_pipeline_sample2)
    link_third_party(dynamic_pipeline_sample2)
    
    add_executable(src_des_sample "src_des_sample.cpp")
    target_link_libraries(src_des_sample cvedix::cvedix_instance_sdk)
    link_gstreamer(src_des_sample)
    
    # Inference samples
    add_executable(trt_infer_sample "trt_infer_sample.cpp")
    target_link_libraries(trt_infer_sample cvedix::cvedix_instance_sdk)
    
    add_executable(trt_yolov8_sample "trt_yolov8_sample.cpp")
    target_link_libraries(trt_yolov8_sample cvedix::cvedix_instance_sdk)
    
    add_executable(trt_yolov8_sample2 "trt_yolov8_sample2.cpp")
    target_link_libraries(trt_yolov8_sample2 cvedix::cvedix_instance_sdk)
    
    add_executable(multi_trt_infer_nodes_sample "multi_trt_infer_nodes_sample.cpp")
    target_link_libraries(multi_trt_infer_nodes_sample cvedix::cvedix_instance_sdk)
    
    # Pipeline structure samples
    add_executable(1-N-1_sample "1-N-1_sample.cpp")
    target_link_libraries(1-N-1_sample cvedix::cvedix_instance_sdk)
    
    add_executable(1-N-1_sample2 "1-N-1_sample2.cpp")
    target_link_libraries(1-N-1_sample2 cvedix::cvedix_instance_sdk)
    
    add_executable(1-N-1_sample3 "1-N-1_sample3.cpp")
    target_link_libraries(1-N-1_sample3 cvedix::cvedix_instance_sdk)
    
    # Message broker sample
    add_executable(message_broker_sample2 "message_broker_sample2.cpp")
    target_link_libraries(message_broker_sample2 cvedix::cvedix_instance_sdk)
    link_third_party(message_broker_sample2)
    
    # RTMP source sample
    add_executable(rtmp_src_sample "rtmp_src_sample.cpp")
    target_link_libraries(rtmp_src_sample cvedix::cvedix_instance_sdk)
else()
    message(STATUS "Skipping TensorRT samples - TensorRT not available")
endif()

# ============================================================================
# Complex Samples - Require CVEDIX_BUILD_COMPLEX_SAMPLES flag
# ============================================================================
if(CVEDIX_BUILD_COMPLEX_SAMPLES)
    message(STATUS "Building complex samples...")
    
    # Face recognition pipeline
    set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/face_recognize)
    add_executable(face_recognize_pipeline "face_recognize/face_recognize_pipeline.cpp")
    target_link_libraries(face_recognize_pipeline cvedix::cvedix_instance_sdk)
    
    if(CVEDIX_WITH_TRT)
        # Similarity search pipelines
        set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/similiarity_search)
        add_executable(face_encoding_pipeline "similiarity_search/face_encoding_pipeline.cpp")
        target_link_libraries(face_encoding_pipeline cvedix::cvedix_instance_sdk)
        
        add_executable(vehicle_encoding_pipeline "similiarity_search/vehicle_encoding_pipeline.cpp")
        target_link_libraries(vehicle_encoding_pipeline cvedix::cvedix_instance_sdk)
        
        # Vehicle behaviour analysis pipeline
        set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/vehicle_behaviour_analysis)
        add_executable(vehicle_ba_pipeline "vehicle_behaviour_analysis/vehicle_ba_pipeline.cpp")
        target_link_libraries(vehicle_ba_pipeline cvedix::cvedix_instance_sdk)
        
        # Vehicle property and similarity search pipeline
        set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/vehicle_property_and_similiarity_search)
        add_executable(vehicle_encoding_classify_pipeline "vehicle_property_and_similiarity_search/vehicle_encoding_classify_pipeline.cpp")
        target_link_libraries(vehicle_encoding_classify_pipeline cvedix::cvedix_instance_sdk)
        
        # License plate recognition pipeline
        set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/lpr_camera)
        add_executable(plate_recognize_pipeline "lpr_camera/plate_recognize_pipeline.cpp")
        target_link_libraries(plate_recognize_pipeline cvedix::cvedix_instance_sdk)
    endif()
    
    if(CVEDIX_WITH_PADDLE)
        # Math review pipeline
        set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/math_review)
        add_executable(math_expression_check_pipeline "math_review/math_expression_check_pipeline.cpp")
        target_link_libraries(math_expression_check_pipeline cvedix::cvedix_instance_sdk)
    endif()
    
    # Reset output directory
    set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
else()
    message(STATUS "Skipping complex samples - CVEDIX_BUILD_COMPLEX_SAMPLES not enabled")
endif()

# ============================================================================
# Installation Rules
# ============================================================================
message(STATUS "Configuring installation rules...")

# Install basic samples
install(TARGETS 
    1-1-1_sample 1-1-N_sample 1-N-N_sample N-1-N_sample N-N_sample
    ba_crossline_sample enet_seg_sample face_tracking_sample
    image_des_sample image_src_sample interaction_with_pipe_sample
    mask_rcnn_sample message_broker_sample multi_detectors_and_classifiers_sample
    openpose_sample record_sample
    cvedix_logger_sample skip_sample obstacle_detect_sample
    multi_detectors_sample firesmoke_detect_sample face_swap_sample
    face_yunet_int8_sample video_restoration_sample app_des_sample
    app_src_des_sample lane_detect_sample frame_fusion_sample cvedix_test
    DESTINATION bin
    OPTIONAL
)

# Install RTSP samples if available
if(CVEDIX_WITH_RTSP_SERVER)
    install(TARGETS 
        rtsp_des_sample rtsp_src_sample rtsp_ba_crossline_sample
        face_tracking_rtsp_sample
        DESTINATION bin
        OPTIONAL
    )
endif()

# Install Kafka samples if available
if(CVEDIX_WITH_KAFKA)
    install(TARGETS 
        message_broker_kafka_sample
        DESTINATION bin
        OPTIONAL
    )
endif()

# Install MQTT samples if available
if(CVEDIX_WITH_MQTT)
    # Only install samples if they were built (mosquitto available)
    if(TARGET simple_rtmp_mqtt_sample)
        install(TARGETS 
            simple_rtmp_mqtt_sample
            DESTINATION bin
            OPTIONAL
        )
    endif()
    if(TARGET mqtt_json_receiver_sample)
        install(TARGETS 
            mqtt_json_receiver_sample
            DESTINATION bin
            OPTIONAL
        )
    endif()
endif()

# Install LLM samples if available
if(CVEDIX_WITH_LLM)
    install(TARGETS 
        mllm_analyse_sample mllm_analyse_sample_openai
        DESTINATION bin
        OPTIONAL
    )
endif()

# Install FFmpeg samples if available
if(CVEDIX_WITH_FFMPEG)
    install(TARGETS 
        ffmpeg_src_des_sample ffmpeg_transcode_sample
        DESTINATION bin
        OPTIONAL
    )
endif()

# Install RKNN samples if available
if(CVEDIX_WITH_RKNN)
    install(TARGETS 
        rknn_yolov11_detector_sample
        rknn_face_detector_sample
        rknn_face_detection_sample
        rknn_face_detection_file_sample
        rknn_rtsp_tracking_sample
        DESTINATION bin
        OPTIONAL
    )
    
    # Install RKNN MQTT samples if available
    if(CVEDIX_WITH_MQTT)
        install(TARGETS 
            rknn_face_tracking_sample
            DESTINATION bin
            OPTIONAL
        )
        
        if(CVEDIX_WITH_RTSP_SERVER)
            install(TARGETS 
                rknn_rtsp_tracking_mqtt_sample
                DESTINATION bin
                OPTIONAL
            )
        endif()
    endif()
endif()

# Install PaddlePaddle samples if available
if(CVEDIX_WITH_PADDLE)
    install(TARGETS 
        app_src_sample paddle_infer_sample
        DESTINATION bin
        OPTIONAL
    )
endif()

# Install CUDA samples if available
if(CVEDIX_WITH_CUDA)
    install(TARGETS 
        nv_hard_codec_sample
        DESTINATION bin
        OPTIONAL
    )
endif()

# Install TensorRT samples if available
if(CVEDIX_WITH_TRT)
    install(TARGETS 
        ba_jam_sample ba_stop_sample body_scan_and_plate_detect_sample
        dynamic_pipeline_sample dynamic_pipeline_sample2
        message_broker_sample2 multi_trt_infer_nodes_sample
        plate_recognize_sample src_des_sample trt_infer_sample
        vehicle_body_scan_sample vehicle_cluster_based_on_classify_encoding_sample
        vehicle_tracking_sample 1-N-1_sample 1-N-1_sample2 1-N-1_sample3
        trt_yolov8_sample trt_yolov8_sample2 rtmp_src_sample
        DESTINATION bin
        OPTIONAL
    )
endif()

message(STATUS "Sample build configuration completed.")
