#!/bin/zsh
# useful script executing commands on a mac mini. 
# written on 1202604046.204915

cd root/projects/ua/calls/21/

mv ../20/build new_build
mv build old_build
mv new_build build

ls -l

head -n 100 *build*


cd ~
exit

#cd 21
#ls -l
#head -n 40 c.c 
#cd ~
#echo "...."
#ls -l root/staging/21
#cd root/staging
#mkdir 21/
#mv c.c build 21/






#cd calls/18/
#ls -l
#top -l 1 | head -n 16
#hostinfo
#screen -ls


#git pull
#./build release
#screen -dmS search_call sh -c "./run; exec bash"
#cat *output*











































