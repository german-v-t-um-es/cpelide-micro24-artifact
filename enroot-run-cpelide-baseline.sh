#!/bin/bash

#SBATCH -J cpelide_square                           # Job name
#SBATCH -D /home/germanvt                           # Working directory on host
#SBATCH -o latest_outputs/cpelide_square_sim.log    # Standard output log
#SBATCH -e latest_outputs/cpelide_square_sim.err    # Standard error log
#SBATCH -c 8                                        # Request 1 CPU core

# Run the simulation using Enroot
enroot start --rw --mount /home/germanvt:/home/germanvt cpelide bash -c "
  cd /home/germanvt/MULTIGPU/cpelide-micro24-artifact/gem5_multigpu/; \
  build/GCN3_X86/gem5.opt \
    --debug-flags=GlobalScheduler,CPCoh \
    --outdir=../latest_stats/square \
    configs/example/apu_se.py \
      -n80 -u60 --cu-per-sa=60 \
      --num-gpu-complex=1 \
      --reg-alloc-policy=dynamic \
      --barriers-per-cu=16 \
      --num-tccs=8 \
      --bw-scalor=8 \
      --tcc-size=4096kB \
      --tcc-assoc=32 \
      --num-dirs=64 \
      --mem-size=16GB \
      --mem-type=HBM_1000_4H_1x64 \
      --vreg-file-size=16384 \
      --sreg-file-size=800 \
      --num-hw-queues=256 \
      --num-gpus=4 \
      --gs-policy=GSP_RRCS \
      --benchmark-root=multigpu_benchmarks/square \
      -c 'square_m' \
      --options='524288 1 2 2048 256' \
      --coal-tokens=160 \
      --gpu-clock=1801MHz \
      --ruby-clock=1000MHz \
      --TCC_latency=121 \
      --vrf_lm_bus_latency=6 \
      --mem-req-latency=69 \
      --mem-resp-latency=69 \
      --TCP_latency=16 \
      --gs-num-sched-gpu=2 \
      --max-coalesces-per-cycle=10 \
      --sqc-size=16kB \
      --default-acq-rel
"


