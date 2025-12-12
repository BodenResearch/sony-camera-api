#!/bin/bash

echo "Sony Camera REST API - Build Script"
echo "===================================="

# Update system and install build tools
echo "Installing build dependencies..."
sudo apt update
sudo apt install -y build-essential cmake git

# Initialize and update git submodules
echo "Initializing git submodules..."
git submodule update --init --recursive

# Build the project
echo "Building project..."
mkdir -p build
cd build || exit 1

echo "Running cmake..."
cmake ..

echo "Compiling..."
make -j$(nproc)

if [ $? -eq 0 ]; then
    echo ""
    echo "Build successful!"
    echo "Executable: ./CameraRestAPI"
    echo ""
    echo "To run the server:"
    echo "  ./CameraRestAPI [port] [--fake-camera - enables debugging without a camera]"
    echo ""
    echo "Default port is 8080 if not specified."
    echo ""
    
    # Check if user wants to run immediately
    read -p "Run the server now? (y/n) " -n 1 -r
    echo
    if [[ $REPLY =~ ^[Yy]$ ]]; then
        ./CameraRestAPI
    fi
else
    echo ""
    echo "Build failed!"
    exit 1
fi