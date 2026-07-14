#!/bin/zsh
# useful script executing commands on a mac mini. 
# written on 1202604046.204915

echo "--------------------------------------------"
hostinfo | grep "Load average"
pmset -g | grep -v -E '(autorestart|displaysleep|Button)'
pmset -g assertions | grep "   UserIsActive"
top -l 3 | grep "CPU usage"
top -l 3 | grep "search_run" | cut -c 1-50
exit














#top -l 10 | head -n 18
#hostinfo
#screen -ls
#cd root/projects/ua/
#cd calls/18/
#ls -l
#cd ~
#exit

#git pull
#git pull
#./build release
#screen -dmS search_call sh -c "./run; exec bash"
#cat *output*






































