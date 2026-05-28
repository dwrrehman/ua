#!/bin/zsh
# useful script executing commands on a mac mini. 
# written on 1202604046.204915

cd root/projects/ua/calls/21/

./build release

nohup ./search_run >/dev/null 2>&1 </dev/null &
disown

echo "search call started."


exit










































