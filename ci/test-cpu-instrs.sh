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

echo -e "\n########################################################################";
echo -e "${YELLOW}BUILD"
echo "########################################################################";
pio run -e native
if [ $? -ne 0 ]; then echo -e "${RED}\xe2\x9c\x96"; else echo -e "${GREEN}\xe2\x9c\x93"; fi

echo -e "\n########################################################################";
echo -e "${YELLOW}RUN TEST"
echo "########################################################################";
.pio/build/native/program 0 70000000 | tee test.out
if grep -q "Passed all tests" test.out; then 
    echo -e "${GREEN}\xe2\x9c\x93";
else
    echo -e "${RED}\xe2\x9c\x96"; 
    exit 1;
fi
