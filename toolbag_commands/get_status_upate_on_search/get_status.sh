#!/bin/zsh
# useful script for getting the progress 
# status of each search call on each mac mini. 
# written on 1202604116.200127

cd root/projects/ua/calls/18/
kill -s USR1 $(pgrep search_run)
sleep 1
cat *output*
screen -ls
hostinfo
top -l 1 | grep "CPU usage"
cd ~
exit




