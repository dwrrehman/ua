#!/bin/zsh
echo "------------------- 2sp r0 -----------------------"
./r0d2_run | tee d2r0_output.txt
echo "------------------- 2sp r1 -----------------------"
./r1d2_run | tee d2r1_output.txt
