#!/bin/zsh
# useful script executing commands on a mac mini. 
# written on 1202604046.204915

cd root/projects/ua
git pull
cd calls/18/
./build release
./search_run &
exit










































