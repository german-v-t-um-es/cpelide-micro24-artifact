#!/bin/bash

#SBATCH -J compile_cpelide                      # Job name
#SBATCH -D /home/germanvt                       # Working directory on host
#SBATCH -o latest_outputs/compile_cpelide.log   # Standard output log
#SBATCH -e latest_outputs/compile_cpelide.err   # Standard error log
#SBATCH -c 8                                    # Request 8 CPU core

# Build gem5
enroot start --rw --mount $(pwd):$(pwd) cpelide bash -c "cd gem5_multigpu/; scons -j8 build/GCN3_X86/gem5.opt"

# Build m5ops used by benchmarks
enroot start --rw --mount $(pwd):$(pwd) cpelide bash -c "cd gem5_multigpu/util/m5; scons -j8 build/x86/out/m5"

# Compile square benchmark
enroot start --rw --mount $(pwd):$(pwd) cpelide bash -c "cd gem5_multigpu/multigpu_benchmarks/square; make square_m"

