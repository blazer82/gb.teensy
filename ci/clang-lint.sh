#!/bin/bash

# Exit immediately if a command exits with a non-zero status.
set -e

# Enable the globstar shell option
shopt -s globstar

# Define colors
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'

# Make sure we are inside the github workspace
cd $GITHUB_WORKSPACE

# Install clang-format
sudo apt-get update
sudo apt-get -y install clang-format-10

# Check clang-format output
for f in **/*.{h,c,hpp,cpp,ino} ; do
    if [ -f "$f" ]; then
        echo -e "\n########################################################################";
        echo -e "${YELLOW}CHECK FORMATTING OF ${f}"
        echo "########################################################################";
        diff $f <(clang-format -assume-filename=main.cpp $f) 1>&2
        if [ $? -ne 0 ]; then echo -e "${RED}\xe2\x9c\x96"; else echo -e "${GREEN}\xe2\x9c\x93"; fi
    fi
done
