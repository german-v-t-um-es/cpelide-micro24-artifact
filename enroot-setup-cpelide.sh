#!/bin/bash
set -e  # Exit if any command fails

# Step 0: Set paths
export CPELIDE_PATH=$(pwd)

# Clone benchmarks subrepo
git submodule update --init

# Step 1: Setup Enroot
cd ..
enroot import --output cpelide.sqsh docker://germanvt/cpelide:latest
enroot create --name cpelide cpelide.sqsh
cd "$CPELIDE_PATH"

# Step 2: Execute compile script
./enroot-compile-cpelide.sh


