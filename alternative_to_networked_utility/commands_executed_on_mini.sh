#!/bin/zsh
# useful script executing commands on a mac mini. 
# written on 1202604046.204915

cd root/projects/ua
git pull
cd calls/18/
./build release
screen -dmS search_call sh -c "./run; exec bash"
ls -l
sleep 1
cat *output*
cd ~
exit










































