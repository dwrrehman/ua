#!/bin/zsh
# useful script executing commands on a mac mini. 
# written on 1202604046.204915

cd root/projects/ua

git pull

cd calls/17/

./build release

screen -d -m session_name "./run"

ls -l

sleep 1

cat *output*

cd ~

exit










































