#!/bin/bash
# Wrapper script to run rtsp_ba_crossline_app with proper LD_LIBRARY_PATH

# Set SDK library path
export LD_LIBRARY_PATH=/opt/cvedix/lib:${LD_LIBRARY_PATH}

# Suppress GStreamer CRITICAL warnings (set to 1 for WARNING and above, 0 for ERROR only)
if [ -z "$GST_DEBUG" ]; then
    export GST_DEBUG=1
fi

# Get the script directory
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
BUILD_DIR="${SCRIPT_DIR}/build"

# Check if executable exists
if [ ! -f "${BUILD_DIR}/rtsp_ba_crossline_app" ]; then
    echo "Error: Executable not found: ${BUILD_DIR}/rtsp_ba_crossline_app"
    echo "Please build the application first: cd build && make"
    exit 1
fi

# Run the application with all arguments
cd "${BUILD_DIR}"
exec ./rtsp_ba_crossline_app "$@"
