#!/bin/bash

# Exit immediately if a command exits with a non-zero status.
set -e

# Define colors
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'

# Make sure we are inside the github workspace
cd $GITHUB_WORKSPACE

# Install PlatformIO CLI
echo -e "\n########################################################################";
echo -e "${YELLOW}INSTALLING PLATFORMIO CLI"
echo "########################################################################";
export PATH=$PATH:~/.platformio/penv/bin
curl -fsSL https://raw.githubusercontent.com/platformio/platformio-core-installer/master/get-platformio.py -o get-platformio.py
python3 get-platformio.py

# Install Teensy platform
echo -e "\n########################################################################";
echo -e "${YELLOW}INSTALLING TEENSY PLATFORM"
echo "########################################################################";
pio platform install "teensy"

# Install dependencies
echo -e "\n########################################################################";
echo -e "${YELLOW}INSTALLING DEPENDENCIES"
echo "########################################################################";
pio lib install FT81x_Arduino_Driver

echo -e "\n########################################################################";
echo -e "${YELLOW}BUILD"
echo "########################################################################";
echo "[env:teensy40]" > platformio.ini
echo "platform = teensy" >> platformio.ini
echo "framework = arduino" >> platformio.ini
echo "board = teensy40" >> platformio.ini
echo "" >> platformio.ini
echo "[env:teensy41]" >> platformio.ini
echo "platform = teensy" >> platformio.ini
echo "framework = arduino" >> platformio.ini
echo "board = teensy41" >> platformio.ini
pio run
if [ $? -ne 0 ]; then echo -e "${RED}\xe2\x9c\x96"; else echo -e "${GREEN}\xe2\x9c\x93"; fi
