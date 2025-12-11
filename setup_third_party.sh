#!/bin/bash
# Script to setup third_party directory for CVEDIX SDK
# This creates a symlink from /opt/cvedix/third_party to workspace third_party

set -e

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
THIRD_PARTY_DIR="${SCRIPT_DIR}/third_party"
TARGET_DIR="/opt/cvedix/third_party"

echo "Setting up third_party for CVEDIX SDK..."
echo "Source: ${THIRD_PARTY_DIR}"
echo "Target: ${TARGET_DIR}"

# Check if source directory exists
if [ ! -d "${THIRD_PARTY_DIR}" ]; then
    echo "Error: ${THIRD_PARTY_DIR} does not exist!"
    echo "Please ensure third_party directory exists in workspace."
    exit 1
fi

# Check if cereal exists
if [ ! -d "${THIRD_PARTY_DIR}/cereal" ]; then
    echo "Error: ${THIRD_PARTY_DIR}/cereal does not exist!"
    echo "Please ensure cereal library is in third_party directory."
    exit 1
fi

# Create symlink (requires sudo)
if [ -L "${TARGET_DIR}" ] || [ -d "${TARGET_DIR}" ]; then
    echo "Warning: ${TARGET_DIR} already exists."
    read -p "Do you want to remove it and create a new symlink? (y/n) " -n 1 -r
    echo
    if [[ $REPLY =~ ^[Yy]$ ]]; then
        sudo rm -rf "${TARGET_DIR}"
    else
        echo "Aborted."
        exit 1
    fi
fi

echo "Creating symlink (requires sudo password)..."
sudo ln -sf "${THIRD_PARTY_DIR}" "${TARGET_DIR}"

if [ $? -eq 0 ]; then
    echo "Success! Symlink created: ${TARGET_DIR} -> ${THIRD_PARTY_DIR}"
    echo "You can now build the samples."
else
    echo "Error: Failed to create symlink."
    exit 1
fi


