#!/bin/zsh
./r0d2_run | tee d2r0_output.txt
echo "------------------- next -----------------------"
./r1d2_run | tee d2r1_output.txt
