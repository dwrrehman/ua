#!/bin/zsh
# useful script for getting the progress 
# status of each search call on each mac mini. 
# written on 1202604116.200127

cd root/projects/ua/calls/17/
kill -s HUP 11354
sleep 1
cat *output*
cd ~
exit










































