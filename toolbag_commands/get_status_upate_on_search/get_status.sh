#!/bin/zsh
# useful script for getting the progress 
# status of each search call on each mac mini. 
# written on 1202604116.200127

cd root/projects/ua/calls/21/
kill -s USR1 $(pgrep search_run)
sleep 1

#cat *output* | tail -n 20 | grep "remaining"

cat *output* | tail -n 45

cd ~
exit




