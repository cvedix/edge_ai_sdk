#!/bin/bash
# Wrapper script to run samples with proper LD_LIBRARY_PATH

# Set SDK library path
export LD_LIBRARY_PATH=/opt/cvedix/lib:${LD_LIBRARY_PATH}

# Get the script directory
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
BUILD_DIR="${SCRIPT_DIR}/build/bin"

# Check if build directory exists
if [ ! -d "${BUILD_DIR}" ]; then
    echo "Error: Build directory not found: ${BUILD_DIR}"
    echo "Please build the samples first: cd build && make"
    exit 1
fi

# If no arguments, show usage
if [ $# -eq 0 ]; then
    echo "Usage: $0 <sample_name> [arguments...]"
    echo ""
    echo "Available samples:"
    ls -1 "${BUILD_DIR}"/*_sample 2>/dev/null | xargs -n1 basename
    exit 1
fi

SAMPLE_NAME="$1"
shift

SAMPLE_PATH="${BUILD_DIR}/${SAMPLE_NAME}"

if [ ! -f "${SAMPLE_PATH}" ]; then
    echo "Error: Sample not found: ${SAMPLE_PATH}"
    exit 1
fi

# Run the sample with remaining arguments
exec "${SAMPLE_PATH}" "$@"
